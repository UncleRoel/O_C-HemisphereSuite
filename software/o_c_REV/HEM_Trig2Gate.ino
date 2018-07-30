// Trigger to gate convertor
// applet for Hemisphere by Roel Das

// Added inverted signal


// Max and min gate lengths in ms
#define HEM_T2G_LENGTH_HIGH 9990
// Max mod ranges
#define HEM_T2G_CV_RANGE 2000
// number of display refresh cycles to display clock-output
#define HEM_T2G_DRAW_CLK_CYCLES 100 
// Samplecounter (scope speed)
#define SAMPLE_COUNT 500
// Cursor selectors
#define SEL_T2G_LENGTH_1 0
#define SEL_T2G_LENGTH_2 1

class Trig2Gate : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "Trig2Gate";
    }

  /* Run when the Applet is selected */
    void Start() {
        ForEachChannel(ch)
        {
          //initialise channels
          Ch_GateTicks[ch]=-1; // Ticks the gate has been high. -1 means gate is low.
          last_trigger[ch]=0;
          Ch_GateLengthMenu[ch] = 1000;         
        }
        cursor = 0;
        
    }
  /* Run during interrupt service routine, 16667 times per second, every 60us*/
    void Controller() {
        // Handle trigger ins
        ForEachChannel(ch)
        {
          // Inverted?
          if (Ch_GateLengthMenu[ch] <0) Ch_GateInv[ch]=1;
          else Ch_GateInv[ch]=0;
          
          if (Clock(ch)) {
            // handle clock: start counting ticks + set output 
            Ch_GateTicks[ch]=0;
            GateOut(ch, !Ch_GateInv[ch]);            
            gate[ch]=!Ch_GateInv[ch];
            last_trigger[ch] = OC::CORE::ticks;
          }

          // Take CVs modulation into account
          //  ticks = length(ms) *1000 us /60 ticks/us
          if (!Ch_GateInv[ch]) Ch_CurrentGateLength[ch] = Ch_GateLengthMenu[ch] + Proportion(In(ch),HEMISPHERE_MAX_CV,HEM_T2G_CV_RANGE);
          else Ch_CurrentGateLength[ch] = -Ch_GateLengthMenu[ch] + Proportion(In(ch),HEMISPHERE_MAX_CV,HEM_T2G_CV_RANGE);
          Ch_CurrentGateLength[ch] = constrain(Ch_CurrentGateLength[ch], 1, HEM_T2G_LENGTH_HIGH);
          Ch_CurrentGateLength[ch] = Ch_CurrentGateLength[ch] * 1000 /60; // Convert to ticks!!
          
          // Check if gate time has passed
          if (Ch_GateTicks[ch]>=Ch_CurrentGateLength[ch]) {
            // Reset gate
            Ch_GateTicks[ch]=-1;
            GateOut(ch, Ch_GateInv[ch]);
            gate[ch]=Ch_GateInv[ch];
          }
          
          // While gate is high, increase ticks! and output high gate! (No gate means -1!!)
          if (Ch_GateTicks[ch]>=0) {
            Ch_GateTicks[ch]++;    
          }
        }
        // Scope_code
        if (--sample_countdown <1)
        {
          sample_countdown = SAMPLE_COUNT;
          if (++sample_num > 63) sample_num = 0;
          scope[0][sample_num] = gate[0];
          scope[1][sample_num] = gate[1];
        }
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
      cursor = 1 - cursor;
      //if (cursor >= 2) cursor = 0;
    }

  /* Called when the encoder for this hemisphere is rotated
   * direction 1 is clockwise
   * direction -1 is counterclockwise
   */
    void OnEncoderMove(int direction) {
      if (cursor == SEL_T2G_LENGTH_1) {
        int lastValue = Ch_GateLengthMenu[0];
        Ch_GateLengthMenu[0] += direction * 10;
        Ch_GateLengthMenu[0] = constrain(Ch_GateLengthMenu[0], -HEM_T2G_LENGTH_HIGH, HEM_T2G_LENGTH_HIGH);
        if (Ch_GateLengthMenu[0] == 0) {
          Ch_GateLengthMenu[0] -= lastValue; // substract last value to go one step further and skip 0!
          Ch_GateInv[0] =!Ch_GateInv[0];
          GateOut(0, Ch_GateInv[0]);            
          gate[0]= Ch_GateInv[0];
        }
      }
      if (cursor == SEL_T2G_LENGTH_2) {
        int lastValue = Ch_GateLengthMenu[1];
        Ch_GateLengthMenu[1] += direction * 10;
        Ch_GateLengthMenu[1] = constrain(Ch_GateLengthMenu[1], -HEM_T2G_LENGTH_HIGH, HEM_T2G_LENGTH_HIGH);
        if (Ch_GateLengthMenu[1] == 0) {
          Ch_GateLengthMenu[1] -= lastValue; // substract last value to go one step further and skip 0!
          Ch_GateInv[1] =!Ch_GateInv[1];
          GateOut(1, Ch_GateInv[1]);            
          gate[1]= Ch_GateInv[1];
        }
      } 
      
    }
        
    /* Each applet may save up to 32 bits of data. When data is requested from
     * the manager, OnDataRequest() packs it up (see HemisphereApplet::Pack()) and
     * returns it.
     */
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,16}, Ch_GateLengthMenu[0]); // 10 bits for bpm, range = 1024
        Pack(data, PackLocation {16,16}, Ch_GateLengthMenu[1]);
        return data;
    }

    /* When the applet is restored (from power-down state, etc.), the manager may
     * send data to the applet via OnDataReceive(). The applet should take the data
     * and unpack it (see HemisphereApplet::Unpack()) into zero or more of the applet's
     * properties.
     */
    void OnDataReceive(uint32_t data) {
        Ch_GateLengthMenu[0] = Unpack(data, PackLocation {0,16}); 
        Ch_GateLengthMenu[1] = Unpack(data, PackLocation {16,16});
    }

protected:
    /* Set help text. Each help section can have up to 18 characters. Be concise! */
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "Trig Ch1,2";
        help[HEMISPHERE_HELP_CVS]      = "Length mod Ch1,2";
        help[HEMISPHERE_HELP_OUTS]     = "Gate Ch1,2";
        help[HEMISPHERE_HELP_ENCODER]  = "Set length";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int32_t Ch_GateTicks[2]; // Gate is high as long as there are ticks remaining.
    int16_t Ch_GateLengthMenu[2]; // Length of gate as set by user, in ms (Negative = inverted out!)
    int Ch_CurrentGateLength[2]; // Length of gate after CV mod
    boolean Ch_GateInv[2];
    uint32_t last_trigger[2];
    int cursor; // Cursor 
    boolean scope[2][64];
    boolean gate[2];
    int scope_sample_nr;
    int sample_countdown;
    int sample_num;
    const uint8_t NOT_bitmap[12] = { 0x08, 0x08, 0x08, 0x7f, 0x7f, 0x3e, 0x1c, 0x1c, 0x08, 0x0a, 0x0c}; // NOT
    
    void DrawSelector() {
        ForEachChannel(ch)
        {
          int y = 15 + (ch * 25);
          if (ch == cursor) gfxCursor(0, y + 8, 63);
    
          gfxPrint(1, y, Ch_GateLengthMenu[ch]);
          gfxPrint("ms");
          if (Ch_GateInv[ch]) gfxBitmap(40, y, 12, NOT_bitmap);
          if (OC::CORE::ticks - last_trigger[ch] < 1667) gfxBitmap(54, y, 8, clock_icon);

          // Scope code
          for (int s = 0; s <64; s++)
          {
            int x = s + sample_num;
            int s_draw = 63-s;
            if (x>63) x-=64;
            if (scope[ch][x]!= scope[ch][(x+1)%63]) {
              // Draw line!
              gfxPixel(s_draw, y+13);
              gfxPixel(s_draw, y+14);
              gfxPixel(s_draw, y+15);
              gfxPixel(s_draw, y+16);
              gfxPixel(s_draw, y+17);
              gfxPixel(s_draw, y+18);
            }
            gfxPixel(s_draw, y+18-(scope[ch][x]*5));    
          }
        }
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
Trig2Gate Trig2Gate_instance[2];

void Trig2Gate_Start(int hemisphere) {
    Trig2Gate_instance[hemisphere].BaseStart(hemisphere);
}

void Trig2Gate_Controller(int hemisphere, bool forwarding) {
    Trig2Gate_instance[hemisphere].BaseController(forwarding);
}

void Trig2Gate_View(int hemisphere) {
    Trig2Gate_instance[hemisphere].BaseView();
}

void Trig2Gate_Screensaver(int hemisphere) {
    Trig2Gate_instance[hemisphere].BaseScreensaverView();
}

void Trig2Gate_OnButtonPress(int hemisphere) {
    Trig2Gate_instance[hemisphere].OnButtonPress();
}

void Trig2Gate_OnEncoderMove(int hemisphere, int direction) {
    Trig2Gate_instance[hemisphere].OnEncoderMove(direction);
}

void Trig2Gate_ToggleHelpScreen(int hemisphere) {
    Trig2Gate_instance[hemisphere].HelpScreen();
}

uint32_t Trig2Gate_OnDataRequest(int hemisphere) {
    return Trig2Gate_instance[hemisphere].OnDataRequest();
}

void Trig2Gate_OnDataReceive(int hemisphere, uint32_t data) {
    Trig2Gate_instance[hemisphere].OnDataReceive(data);
}
