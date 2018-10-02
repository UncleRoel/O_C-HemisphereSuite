// Copyright (c) 2018, Roel Das
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// Max and min clocks in bpm
#define HEM_BEN_CLK_HIGH 1000
#define HEM_BEN_CLK_LOW 1
// Max and min division
#define HEM_BEN_DIV_LOW 1
#define HEM_BEN_DIV_HIGH 32
// Max mod ranges
#define HEM_BEN_DIVCV_LOW 0
#define HEM_BEN_DIVCV_HIGH 31
#define HEM_BEN_CLKCV_LOW 0
#define HEM_BEN_CLKCV_HIGH 1000
// number of display refresh cycles to display clock-output
#define HEM_BEN_DRAW_CLK_CYCLES 100 

#define SEL_BEN_CLK 0
#define SEL_BEN_CLK_CV 1
#define SEL_BEN_DIV 2
#define SEL_BEN_DIV_CV 3


class BigBen : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "Big Ben";
    }

	/* Run when the Applet is selected */
    void Start() {
        next_clock_countdown = 0;
        bpm = 100;
        current_bpm = bpm;
        clk_cv_range = 0;
        divider = 4;
        current_div = divider;
        div_counter = 0;
        div_cv_range = 0;
        selected = 0;
        last_clock = 0;
        last_clock = 0;
        
    }
  /* Run during interrupt service routine, 16667 times per second, every 60us*/
    void Controller() {
        // Handle reset trigger
        if (Clock(0)) {
          next_clock_countdown = 0;
          div_counter = 0;
        }
        if (Clock(1)) div_counter = 0;

        // Take CVs modulation into account
        current_bpm = bpm + Proportion(In(0),HEMISPHERE_MAX_CV,clk_cv_range);
        current_bpm = constrain(current_bpm, HEM_BEN_CLK_LOW, HEM_BEN_CLK_HIGH);
        current_div = divider + Proportion(In(1),HEMISPHERE_MAX_CV,div_cv_range);
        current_div = constrain(current_div, HEM_BEN_DIV_LOW, HEM_BEN_DIV_HIGH);

        if (next_clock_countdown == 0) {
          div_counter++; // Accent before clock (to use as RESET!)
          if (div_counter >= divider) {
            ClockOut(1);
            div_counter = 0;
            last_div = OC::CORE::ticks;
          }
          next_clock_countdown = 1000000 /current_bpm; // 1.000.000 ticks/min - 60us/tick - bpm
          ClockOut(0);
          last_clock = OC::CORE::ticks;
        }
        next_clock_countdown-=1;
    }
	/* Draw the screen */
    void View() {
        gfxHeader(applet_name());
        DrawSelector();
    }

	/* Draw the screensaver */
    void ScreensaverView() {
        DrawSelector();
    }

	/* Called when the encoder button for this hemisphere is pressed */
    void OnButtonPress() {
      selected +=1;
      if (selected >= 4) selected = 0;
    }

	/* Called when the encoder for this hemisphere is rotated
	 * direction 1 is clockwise
	 * direction -1 is counterclockwise
	 */
    void OnEncoderMove(int direction) {
      if (selected == SEL_BEN_CLK) {
        bpm += direction;
        bpm = constrain(bpm, HEM_BEN_CLK_LOW, HEM_BEN_CLK_HIGH);
      }
      if (selected == SEL_BEN_CLK_CV) {
        clk_cv_range += direction;
        clk_cv_range = constrain(clk_cv_range, HEM_BEN_CLKCV_LOW, HEM_BEN_CLKCV_HIGH);
      } 
      if (selected == SEL_BEN_DIV) {
        divider += direction;
        divider = constrain(divider, HEM_BEN_DIV_LOW, HEM_BEN_DIV_HIGH);
      }
      if (selected == SEL_BEN_DIV_CV) {
        div_cv_range += direction;
        div_cv_range = constrain(div_cv_range, HEM_BEN_DIVCV_LOW, HEM_BEN_DIVCV_HIGH);
      }
      
    }
        
    /* Each applet may save up to 32 bits of data. When data is requested from
     * the manager, OnDataRequest() packs it up (see HemisphereApplet::Pack()) and
     * returns it.
     */
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,10}, bpm); // 10 bits for bpm, range = 1024
        Pack(data, PackLocation {10,10}, clk_cv_range);
        Pack(data, PackLocation {20,6}, divider-1); // 6bit range
        Pack(data, PackLocation {26,6}, div_cv_range);
        return data;
    }

    /* When the applet is restored (from power-down state, etc.), the manager may
     * send data to the applet via OnDataReceive(). The applet should take the data
     * and unpack it (see HemisphereApplet::Unpack()) into zero or more of the applet's
     * properties.
     */
    void OnDataReceive(uint32_t data) {
        // example: unpack value at bit 0 with size of 8 bits to property_name
        // property_name = Unpack(data, PackLocation {0,8}); 
        bpm = Unpack(data, PackLocation {0,10}); 
        clk_cv_range = Unpack(data, PackLocation {10,10});
        divider = Unpack(data, PackLocation {20,6})+1; 
        div_cv_range = Unpack(data, PackLocation {26,6});
    }

protected:
    /* Set help text. Each help section can have up to 18 characters. Be concise! */
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "Reset 1=clk 2=div";
        help[HEMISPHERE_HELP_CVS]      = "1=BPM 2=DIV";
        help[HEMISPHERE_HELP_OUTS]     = "A=CLK B=DIV";
        help[HEMISPHERE_HELP_ENCODER]  = "Master clock + div";
        //                               "------------------" <-- Size Guide
    }
    
private:
    uint16_t bpm; // Amount of clocks per minute
    uint32_t next_clock_countdown; // Counting down to next clock (for clock multiply)
    uint8_t divider;
    uint8_t div_counter;
    int selected;
    int clk_cv_range;
    int div_cv_range;
    int current_bpm; // BPM with CV added
    int current_div;
    uint32_t last_clock;
    uint32_t last_div;

    void DrawSelector() {
        int y = 16 + (selected * 12);
        gfxCursor(0, y + 9, 63);
        gfxPrint(2, 16, current_bpm);
        gfxPrint(" BPM");
        if (OC::CORE::ticks - last_clock < 1667) gfxBitmap(54, 16, 8, CLOCK_ICON);
        gfxPrint(2, 28, "mod: ");
        gfxPrint(clk_cv_range);
        gfxPrint(2, 40, "1/");
        gfxPrint(current_div);
        gfxPrint(" DIV");
        if (OC::CORE::ticks - last_div < 1667) gfxBitmap(54, 40, 8, CLOCK_ICON);
        gfxPrint(2, 52, "mod: ");
        gfxPrint(div_cv_range);   
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to ClassName,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
BigBen BigBen_instance[2];

void BigBen_Start(bool hemisphere) {
    BigBen_instance[hemisphere].BaseStart(hemisphere);
}

void BigBen_Controller(bool hemisphere, bool forwarding) {
    BigBen_instance[hemisphere].BaseController(forwarding);
}

void BigBen_View(bool hemisphere) {
    BigBen_instance[hemisphere].BaseView();
}

void BigBen_Screensaver(bool hemisphere) {
    BigBen_instance[hemisphere].BaseScreensaverView();
}

void BigBen_OnButtonPress(bool hemisphere) {
    BigBen_instance[hemisphere].OnButtonPress();
}

void BigBen_OnEncoderMove(bool hemisphere, int direction) {
    BigBen_instance[hemisphere].OnEncoderMove(direction);
}

void BigBen_ToggleHelpScreen(bool hemisphere) {
    BigBen_instance[hemisphere].HelpScreen();
}

uint32_t BigBen_OnDataRequest(bool hemisphere) {
    return BigBen_instance[hemisphere].OnDataRequest();
}

void BigBen_OnDataReceive(bool hemisphere, uint32_t data) {
    BigBen_instance[hemisphere].OnDataReceive(data);
}
