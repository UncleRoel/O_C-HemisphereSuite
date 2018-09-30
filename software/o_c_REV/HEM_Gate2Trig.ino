// Trigger to inverted gate convertor + not gate
// applet for Hemisphere by Roel Das
//
// Specific application tool, combining parts of 2 apps. Might not be usefull for others... 


// Max and min gate lengths in ms
#define HEM_GATE2TRIG_LENGTH_HIGH 9990
// Max mod ranges
#define HEM_GATE2TRIG_CV_RANGE 2000
// number of display refresh cycles to display clock-output
#define HEM_GATE2TRIG_DRAW_CLK_CYCLES 100 
// Samplecounter (scope speed)
#define HEM_GATE2TRIG_SAMPLE_COUNT 500
#define HEM_GATE2TRIG_DEBOUNCETIME 1000 // in ticks, 1000 ticks = 60ms
// Cursor selectors
//#define SEL_T2G_LENGTH_1 0
//#define SEL_T2G_LENGTH_2 1

class Gate2Trig : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "Gate2Trig";
    }

  /* Run when the Applet is selected */
    void Start() {
        ForEachChannel(ch)
        {
          //initialise channels
          Ch_GateTicks[ch]=-1; // Ticks the gate has been high. -1 means gate is low.
          last_trigger[ch]=0;
          gate_debounce[ch]=0; 
          Ch_GateLengthMenu[ch] = 1000;  
        }
        cursor = 0; 
    }
  /* Run during interrupt service routine, 16667 times per second, every 60us*/
    void Controller() {
        // Handle trigger ins
        ForEachChannel(ch)
        {
          // inverted?
          if (Ch_GateLengthMenu[ch] <0) Ch_GateInv[ch]=1;
          else Ch_GateInv[ch]=0;

          // Take CVs modulation into account
          //  ticks = length(ms) *1000 us /60 ticks/us
          if (!Ch_GateInv[ch]) Ch_CurrentGateLength[ch] = Ch_GateLengthMenu[ch] + Proportion(In(ch),HEMISPHERE_MAX_CV,HEM_GATE2TRIG_CV_RANGE);
          else Ch_CurrentGateLength[ch] = -Ch_GateLengthMenu[ch] + Proportion(In(ch),HEMISPHERE_MAX_CV,HEM_GATE2TRIG_CV_RANGE);
          Ch_CurrentGateLength[ch] = constrain(Ch_CurrentGateLength[ch], 1, HEM_GATE2TRIG_LENGTH_HIGH);
          Ch_CurrentGateLength[ch] = Ch_CurrentGateLength[ch] * 100 /6; // Convert to ticks!!

          // We start the debouncing only when gate is high and output has been set! 
          if (Gate(ch) && (gate[ch]==!Ch_GateInv[ch])) gate_debounce[ch] = HEM_GATE2TRIG_DEBOUNCETIME;
          // Countdown while gate is low
          if (!Gate(ch) && (gate_debounce[ch] > 0)) gate_debounce[ch]--;    

          if (Clock(ch) && (gate_debounce[ch]==0)) {
              // handle clock: start counting ticks + set output 
              Ch_GateTicks[ch]=0;
              GateOut(ch, !Ch_GateInv[ch]);            
              gate[ch]=!Ch_GateInv[ch];
              last_trigger[ch] = OC::CORE::ticks; // Start counting, output is high for a time after this clock.
          }
          

          // Check if gate time has passed
          if (Ch_GateTicks[ch]>=Ch_CurrentGateLength[ch]) {
            // Reset gate
            Ch_GateTicks[ch]=-1;
            GateOut(ch, Ch_GateInv[ch]);
            gate[ch]= Ch_GateInv[ch];
          }
          
          // While gate is high, increase ticks! and output high gate! (No gate means -1!!)
          if (Ch_GateTicks[ch]>=0) {
            Ch_GateTicks[ch]++;    
          }


        }
        // Scope_code
        if (--sample_countdown <1)
        {
          sample_countdown = HEM_GATE2TRIG_SAMPLE_COUNT;
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
      if (cursor >= 2) cursor = 0;
    }

  /* Called when the encoder for this hemisphere is rotated
   * direction 1 is clockwise
   * direction -1 is counterclockwise
   */
    void OnEncoderMove(int direction) {
        int lastValue = Ch_GateLengthMenu[cursor];
        Ch_GateLengthMenu[cursor] += (direction*10);
        Ch_GateLengthMenu[cursor] = constrain(Ch_GateLengthMenu[cursor], -HEM_GATE2TRIG_LENGTH_HIGH, HEM_GATE2TRIG_LENGTH_HIGH);
        if (Ch_GateLengthMenu[cursor] == 0) {
          Ch_GateLengthMenu[cursor] -= lastValue; // substract last value to go one step further and skip 0!
          Ch_GateInv[cursor] = !Ch_GateInv[cursor];
          GateOut(cursor, Ch_GateInv[cursor]);            
          gate[cursor] = Ch_GateInv[cursor];
        }
    }
        
    /* Each applet may save up to 32 bits of data. When data is requested from
     * the manager, OnDataRequest() packs it up (see HemisphereApplet::Pack()) and
     * returns it.
     */
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,16}, Ch_GateLengthMenu[0]); // 10 bits for bpm, range = 1024
        Pack(data, PackLocation {16,16}, Ch_GateLengthMenu[1]); // 10 bits for bpm, range = 1024
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
        help[HEMISPHERE_HELP_DIGITALS] = "Gate 1 Gate 2";
        help[HEMISPHERE_HELP_CVS]      = "Length mod Ch1 Ch2";
        help[HEMISPHERE_HELP_OUTS]     = "Trig 1 Trig 2";
        help[HEMISPHERE_HELP_ENCODER]  = "Trig Length / inv";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int32_t Ch_GateTicks[2]; // Gate is high as long as there are ticks remaining.
    int16_t Ch_GateLengthMenu[2]; // Length of gate as set by user, in ms
    boolean Ch_GateInv[2];
    int Ch_CurrentGateLength[2]; // Length of gate after CV mod
    uint32_t last_trigger[2];
    int cursor; // Cursor 
    boolean scope[2][64];
    boolean gate[2];
    int scope_sample_nr;
    int sample_countdown;
    int sample_num;
    int gate_debounce[2];

    const uint8_t NOT_bitmap[12] = { 0x08, 0x08, 0x08, 0x7f, 0x7f, 0x3e, 0x1c, 0x1c, 0x08, 0x0a, 0x0c}; // NOT
      /*  {0x22, 0x22, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x3e, 0x08, 0x08, 0x08}, // AND
        {0x22, 0x22, 0x63, 0x77, 0x7f, 0x7f, 0x3e, 0x1c, 0x1c, 0x08, 0x08, 0x08}, // OR
        {0x22, 0x22, 0x77, 0x08, 0x77, 0x7f, 0x3e, 0x1c, 0x1c, 0x08, 0x08, 0x08}, // XOR
        {0x22, 0x22, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x3e, 0x08, 0x0a, 0x0c}, // NAND
        {0x22, 0x22, 0x63, 0x77, 0x7f, 0x7f, 0x3e, 0x1c, 0x1c, 0x08, 0x0a, 0x0c}, // NOR
        {0x22, 0x22, 0x77, 0x08, 0x77, 0x7f, 0x3e, 0x1c, 0x1c, 0x08, 0x0a, 0x0c}  // XNOR*/
    
    void DrawSelector() {
      
        gfxCursor(0, 23 + (cursor * 25), 63);
               
        ForEachChannel(ch)
        {
          
          if (OC::CORE::ticks - last_trigger[ch] < 1667) gfxBitmap(54, 15 + (ch * 25), 8, CLOCK_ICON);
          if (Ch_GateInv[ch]) {
            gfxBitmap(40, 15 + (ch * 25), 12, NOT_bitmap);
            gfxPrint(1, 15 + (ch * 25), -Ch_GateLengthMenu[ch]);
          }
          else gfxPrint(1, 15 + (ch * 25), Ch_GateLengthMenu[ch]);
          
          int y = 15 + (ch * 25);
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
Gate2Trig Gate2Trig_instance[2];

void Gate2Trig_Start(bool hemisphere) {
    Gate2Trig_instance[hemisphere].BaseStart(hemisphere);
}

void Gate2Trig_Controller(bool hemisphere, bool forwarding) {
    Gate2Trig_instance[hemisphere].BaseController(forwarding);
}

void Gate2Trig_View(bool hemisphere) {
    Gate2Trig_instance[hemisphere].BaseView();
}

void Gate2Trig_Screensaver(bool hemisphere) {
    Gate2Trig_instance[hemisphere].BaseScreensaverView();
}

void Gate2Trig_OnButtonPress(bool hemisphere) {
    Gate2Trig_instance[hemisphere].OnButtonPress();
}

void Gate2Trig_OnEncoderMove(bool hemisphere, int direction) {
    Gate2Trig_instance[hemisphere].OnEncoderMove(direction);
}

void Gate2Trig_ToggleHelpScreen(bool hemisphere) {
    Gate2Trig_instance[hemisphere].HelpScreen();
}

uint32_t Gate2Trig_OnDataRequest(bool hemisphere) {
    return Gate2Trig_instance[hemisphere].OnDataRequest();
}

void Gate2Trig_OnDataReceive(bool hemisphere, uint32_t data) {
    Gate2Trig_instance[hemisphere].OnDataReceive(data);
}
