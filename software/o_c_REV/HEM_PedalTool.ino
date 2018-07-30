// Trigger to inverted gate convertor + not gate
// applet for Hemisphere by Roel Das
//
// Specific application tool, combining parts of 2 apps. Might not be usefull for others... 


// Max and min gate lengths in ms
#define HEM_PEDALTOOL_LENGTH_HIGH 9990
// Max mod ranges
#define HEM_PEDALTOOL_CV_RANGE 2000
// number of display refresh cycles to display clock-output
#define HEM_PEDALTOOL_DRAW_CLK_CYCLES 100 
// Samplecounter (scope speed)
#define HEM_PEDALTOOL_SAMPLE_COUNT 500
// Cursor selectors
//#define SEL_T2G_LENGTH_1 0
//#define SEL_T2G_LENGTH_2 1

class PedalTool : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "Pedal Tool";
    }

  /* Run when the Applet is selected */
    void Start() {
        //ForEachChannel(ch)
        //{
          //initialise channels
          Ch_GateTicks=-1; // Ticks the gate has been high. -1 means gate is low.
          last_trigger=0;
          Ch_GateLengthMenu = 1000;        
        //}
        cursor = 0;
        
    }
  /* Run during interrupt service routine, 16667 times per second, every 60us*/
    void Controller() {
        // Handle trigger ins
        //ForEachChannel(ch)
        //{
          // inverted?
          if (Ch_GateLengthMenu <0) Ch_GateInv=1;
          else Ch_GateInv=0;

          if (Clock(0)) {
            // handle clock: start counting ticks + set output 
            Ch_GateTicks=0;
            GateOut(0, !Ch_GateInv);            
            gate[0]=!Ch_GateInv;
            last_trigger = OC::CORE::ticks;
          }
          
          // Take CVs modulation into account
          //  ticks = length(ms) *1000 us /60 ticks/us
          if (!Ch_GateInv) Ch_CurrentGateLength = Ch_GateLengthMenu + Proportion(In(0),HEMISPHERE_MAX_CV,HEM_PEDALTOOL_CV_RANGE);
          else Ch_CurrentGateLength = -Ch_GateLengthMenu + Proportion(In(0),HEMISPHERE_MAX_CV,HEM_PEDALTOOL_CV_RANGE);
          Ch_CurrentGateLength = constrain(Ch_CurrentGateLength, 1, HEM_PEDALTOOL_LENGTH_HIGH);
          Ch_CurrentGateLength = Ch_CurrentGateLength * 1000 /60; // Convert to ticks!!

          // Check if gate time has passed
          if (Ch_GateTicks>=Ch_CurrentGateLength) {
            // Reset gate
            Ch_GateTicks=-1;
            GateOut(0, Ch_GateInv);
            gate[0]= Ch_GateInv;
          }
          
          // While gate is high, increase ticks! and output high gate! (No gate means -1!!)
          if (Ch_GateTicks>=0) {
            Ch_GateTicks++;    
          }

          // NOT-gate code for channel 2
          if (Gate(1)) {
            GateOut(1, 0);
            gate[1]=0;             
          }
          else {
            GateOut(1, 1);
            gate[1]=1;               
          }
      
          // Scope_code
          if (--sample_countdown <1)
          {
            sample_countdown = HEM_PEDALTOOL_SAMPLE_COUNT;
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
      //cursor = 1 - cursor;
      //if (cursor >= 2) cursor = 0;
    }

  /* Called when the encoder for this hemisphere is rotated
   * direction 1 is clockwise
   * direction -1 is counterclockwise
   */
    void OnEncoderMove(int direction) {
      //if (cursor == SEL_T2G_LENGTH_1) {
        int lastValue = Ch_GateLengthMenu;
        Ch_GateLengthMenu += (direction*10);
        Ch_GateLengthMenu = constrain(Ch_GateLengthMenu, -HEM_PEDALTOOL_LENGTH_HIGH, HEM_PEDALTOOL_LENGTH_HIGH);
        if (Ch_GateLengthMenu == 0) {
          Ch_GateLengthMenu -= lastValue; // substract last value to go one step further and skip 0!
          Ch_GateInv =!Ch_GateInv;
          GateOut(0, Ch_GateInv);            
          gate[0] = Ch_GateInv;
        }
    }
        
    /* Each applet may save up to 32 bits of data. When data is requested from
     * the manager, OnDataRequest() packs it up (see HemisphereApplet::Pack()) and
     * returns it.
     */
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,16}, Ch_GateLengthMenu); // 10 bits for bpm, range = 1024
        //Pack(data, PackLocation {16,16}, Ch_GateLength[1]);
        return data;
    }

    /* When the applet is restored (from power-down state, etc.), the manager may
     * send data to the applet via OnDataReceive(). The applet should take the data
     * and unpack it (see HemisphereApplet::Unpack()) into zero or more of the applet's
     * properties.
     */
    void OnDataReceive(uint32_t data) {
        Ch_GateLengthMenu = Unpack(data, PackLocation {0,16}); 
        //Ch_GateLength[1] = Unpack(data, PackLocation {16,16});
    }

protected:
    /* Set help text. Each help section can have up to 18 characters. Be concise! */
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "Trig 1 Gate 2";
        help[HEMISPHERE_HELP_CVS]      = "Length mod Ch1";
        help[HEMISPHERE_HELP_OUTS]     = "!Gate 1 !Gate 2";
        help[HEMISPHERE_HELP_ENCODER]  = "Trig2invGate + NOT";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int32_t Ch_GateTicks; // Gate is high as long as there are ticks remaining.
    int16_t Ch_GateLengthMenu; // Length of gate as set by user, in ms
    boolean Ch_GateInv;
    int Ch_CurrentGateLength; // Length of gate after CV mod
    uint32_t last_trigger;
    int cursor; // Cursor 
    boolean scope[2][64];
    boolean gate[2];
    int scope_sample_nr;
    int sample_countdown;
    int sample_num;

    const uint8_t NOT_bitmap[12] = { 0x08, 0x08, 0x08, 0x7f, 0x7f, 0x3e, 0x1c, 0x1c, 0x08, 0x0a, 0x0c}; // NOT
      /*  {0x22, 0x22, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x3e, 0x08, 0x08, 0x08}, // AND
        {0x22, 0x22, 0x63, 0x77, 0x7f, 0x7f, 0x3e, 0x1c, 0x1c, 0x08, 0x08, 0x08}, // OR
        {0x22, 0x22, 0x77, 0x08, 0x77, 0x7f, 0x3e, 0x1c, 0x1c, 0x08, 0x08, 0x08}, // XOR
        {0x22, 0x22, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x7f, 0x3e, 0x08, 0x0a, 0x0c}, // NAND
        {0x22, 0x22, 0x63, 0x77, 0x7f, 0x7f, 0x3e, 0x1c, 0x1c, 0x08, 0x0a, 0x0c}, // NOR
        {0x22, 0x22, 0x77, 0x08, 0x77, 0x7f, 0x3e, 0x1c, 0x1c, 0x08, 0x0a, 0x0c}  // XNOR*/
    
    void DrawSelector() {

        gfxCursor(0, 23, 63);
  
        gfxPrint(1, 15, Ch_GateLengthMenu);
        gfxPrint("ms");
        gfxBitmap(40, 15, 12, NOT_bitmap);
        if (OC::CORE::ticks - last_trigger < 1667) gfxBitmap(54, 15, 8, clock_icon);
        if (Ch_GateInv) gfxBitmap(40, 15, 12, NOT_bitmap);
        gfxPrint(1, 40, "Ch2:");
        gfxBitmap(33, 40, 12, NOT_bitmap);
        
        ForEachChannel(ch)
        {
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
PedalTool PedalTool_instance[2];

void PedalTool_Start(int hemisphere) {
    PedalTool_instance[hemisphere].BaseStart(hemisphere);
}

void PedalTool_Controller(int hemisphere, bool forwarding) {
    PedalTool_instance[hemisphere].BaseController(forwarding);
}

void PedalTool_View(int hemisphere) {
    PedalTool_instance[hemisphere].BaseView();
}

void PedalTool_Screensaver(int hemisphere) {
    PedalTool_instance[hemisphere].BaseScreensaverView();
}

void PedalTool_OnButtonPress(int hemisphere) {
    PedalTool_instance[hemisphere].OnButtonPress();
}

void PedalTool_OnEncoderMove(int hemisphere, int direction) {
    PedalTool_instance[hemisphere].OnEncoderMove(direction);
}

void PedalTool_ToggleHelpScreen(int hemisphere) {
    PedalTool_instance[hemisphere].HelpScreen();
}

uint32_t PedalTool_OnDataRequest(int hemisphere) {
    return PedalTool_instance[hemisphere].OnDataRequest();
}

void PedalTool_OnDataReceive(int hemisphere, uint32_t data) {
    PedalTool_instance[hemisphere].OnDataReceive(data);
}
