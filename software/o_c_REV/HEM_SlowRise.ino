// Roel's SlowRise for Hemisphere

#define HEM_SLOWRISE_STATE_ZERO 0
#define HEM_SLOWRISE_STATE_RUNNING 1
#define HEM_SLOWRISE_STATE_PAUSED 2
#define HEM_SLOWRISE_STATE_FINISHED 3
#define HEM_SLOWRISE_MENU_STARTPAUSE 0
#define HEM_SLOWRISE_MENU_SETTIME 1
#define HEM_SLOWRISE_MENU_RISEFALLRESET 2
// To be implemented:
#define HEM_SLOWRISE_MENU_SETVOLT 3
#define HEM_SLOWRISE_RISE 0
#define HEM_SLOWRISE_FALL 1
#define HEM_SLOWRISE_VOLTAGE_INCREMENTS 128

// MenuItems:
// RESET       | PAUSED   | RUNNING
// 0 Start     | ctu      | Pause
// 1 Set Time  | Set Time | Set Time
// 2 Rise/Fall | RESET    |
// 3 MaxVolt   | MaxVolt (to be impl; in RESET state: output max volt!)   


class SlowRise : public HemisphereApplet {
public:

    const char* applet_name() {
        return "SlowRise";
    }

    void Start() {
      SlowRiseTicks = 0; // time in ms
      SlowRiseState = HEM_SLOWRISE_STATE_ZERO;
      MenuState = HEM_SLOWRISE_MENU_STARTPAUSE;
      TimeSetState = 0; //set time!
      VoltSetState = 0;
      RiseFall = 0;
      SlowRiseDuration[0] = 0;
      SlowRiseDuration[1] = 1;
      SlowRiseDuration[2] = 0;
      SlowRiseDuration_runtime[0] = 0;
      SlowRiseDuration_runtime[1] = 1;
      SlowRiseDuration_runtime[2] = 0;
      SetSlowRiseTicksDuration();
      max_voltage = (5 * (12 << 7)) / HEM_SLOWRISE_VOLTAGE_INCREMENTS; // 5V
      ResetSignal();
    }
//*****************************************************
    void Controller() {
      // Left trigger? Start!
      if (Clock(0)) {
        //StartSlowRise
        if ((SlowRiseState == HEM_SLOWRISE_STATE_ZERO)||(SlowRiseState == HEM_SLOWRISE_STATE_PAUSED)) SlowRiseState = HEM_SLOWRISE_STATE_RUNNING;
        else if (SlowRiseState == HEM_SLOWRISE_STATE_RUNNING) SlowRiseState = HEM_SLOWRISE_STATE_PAUSED;
        if (SlowRiseState == HEM_SLOWRISE_STATE_FINISHED) 
        {
          ResetRiseFall();
          SlowRiseState = HEM_SLOWRISE_STATE_RUNNING;
        }
      }
      
      // On right trigger: Reset!
      if (Clock(1)) {
        ResetRiseFall();
      }
      
      int TimeLeftTicks = SlowRiseTicksDuration-SlowRiseTicks;
      simfloat target;
      target = RiseFall ? 0 : int2simfloat(max_voltage * HEM_SLOWRISE_VOLTAGE_INCREMENTS);
      
      // SlowriseTimer
      if ( SlowRiseState == HEM_SLOWRISE_STATE_RUNNING) 
      {
        SlowRiseTicks+=1;
        // When finished:
        if (TimeLeftTicks <= 0) 
        {
          SlowRiseState = HEM_SLOWRISE_STATE_FINISHED;
          MenuState = HEM_SLOWRISE_MENU_RISEFALLRESET;
          TimeSetState = 0;
          // OUTPUT MAX! (or min)
          signal = target;
          ClockOut(1); // EOC out
        }
        else
        {
          // Prevent division by 0
          // GET DIRECTION RIGHT!!
          if (signal != target) {
          simfloat remaining = target - signal; 
          simfloat delta = remaining / TimeLeftTicks;
          signal += delta;
          }
        }
      }
      
     
      if (VoltSetState) Out(0,(max_voltage * HEM_SLOWRISE_VOLTAGE_INCREMENTS) );
      else Out(0, simfloat2int(signal));
      
      
      // Calculate and set outputs!
    }
//*****************************************************
    void View() {
        gfxHeader(applet_name());
        DrawMenu();
        // Add other view code as private methods
    }
//*****************************************************
    void ScreensaverView() {
      DrawTime((SlowRiseMsDuration - (SlowRiseTicks *6 / 100)),28);
    }
//*****************************************************
    void OnButtonPress() {
      // If TimeSetState? Set time routine!
      if (TimeSetState > 0)
      {
        TimeSetState = TimeSetState + 1;
        // Prevent being able to set time to 0!
        // We always set the runtime duration
        if ((SlowRiseState != HEM_SLOWRISE_STATE_ZERO) && (TimeSetState == 3) && (SlowRiseDuration_runtime[0]==0) && (SlowRiseDuration_runtime[1]==0)&&(SlowRiseDuration_runtime[2]==0)) SlowRiseDuration_runtime[2]= 1;
        if ((SlowRiseState == HEM_SLOWRISE_STATE_ZERO) && (TimeSetState == 3) && (SlowRiseDuration[0]==0) && (SlowRiseDuration[1]==0)&&(SlowRiseDuration[2]==0)) SlowRiseDuration[2]= 1;
        
        if (TimeSetState == 4) 
        {
          // Calculate new time!
          if (SlowRiseState == HEM_SLOWRISE_STATE_ZERO) 
            {
            // If initial state, copy duration to runtime setting!
            SlowRiseDuration_runtime[0]=SlowRiseDuration[0];
            SlowRiseDuration_runtime[1]=SlowRiseDuration[1];
            SlowRiseDuration_runtime[2]=SlowRiseDuration[2];
            }
          SetSlowRiseTicksDuration();
          SlowRiseTicks = 0;
          TimeSetState = 0;
          if ((SlowRiseState == HEM_SLOWRISE_STATE_PAUSED)||(SlowRiseState == HEM_SLOWRISE_STATE_RUNNING)) MenuState = HEM_SLOWRISE_MENU_STARTPAUSE;
        }
      }
      // Enter TimeSetState!
      else if (MenuState == HEM_SLOWRISE_MENU_SETTIME) 
      {
        TimeSetState = 1;
        // Which time to set? Set begin time
        if (SlowRiseState != HEM_SLOWRISE_STATE_ZERO) 
        {
          int GetSeconds = (SlowRiseMsDuration - (SlowRiseTicks *6 / 100))/1000; // Tital time in seconds
          int GetMinutes = GetSeconds / 60;
          GetSeconds = GetSeconds % 60;
          int GetHours = GetMinutes / 60;
          GetMinutes = GetMinutes % 60;
          SlowRiseDuration_runtime[0]=GetHours;
          SlowRiseDuration_runtime[1]=GetMinutes;
          SlowRiseDuration_runtime[2]=GetSeconds;
        }
      }
      else if (MenuState == HEM_SLOWRISE_MENU_RISEFALLRESET) 
      {
        if (SlowRiseState == HEM_SLOWRISE_STATE_ZERO) 
        {
          RiseFall=!RiseFall;
          ResetSignal();
        }
        else if ((SlowRiseState == HEM_SLOWRISE_STATE_PAUSED)||(SlowRiseState == HEM_SLOWRISE_STATE_FINISHED)) // RESET;
        {
          // RESET TIME!!
          // copy duration to runtime setting!
          ResetRiseFall();
        }
      }
      
      // If chrono is running, pause or set time!!
      else if (SlowRiseState == HEM_SLOWRISE_STATE_RUNNING)
      {
        if (MenuState == HEM_SLOWRISE_MENU_STARTPAUSE) SlowRiseState = HEM_SLOWRISE_STATE_PAUSED;
      }
      // If chrono is paused 
      else if (MenuState == HEM_SLOWRISE_MENU_STARTPAUSE)  
      {
        // Reset state
        if (SlowRiseState == HEM_SLOWRISE_STATE_ZERO) 
        {
          SlowRiseState = HEM_SLOWRISE_STATE_RUNNING;            
        }
        // Pause state
        else if (SlowRiseState == HEM_SLOWRISE_STATE_PAUSED) SlowRiseState = HEM_SLOWRISE_STATE_RUNNING;
        // Running
        else if (SlowRiseState == HEM_SLOWRISE_STATE_RUNNING) SlowRiseState = HEM_SLOWRISE_STATE_PAUSED;        
      } 
      else if (MenuState == HEM_SLOWRISE_MENU_SETVOLT) VoltSetState=!VoltSetState;
    }
//*****************************************************
    void OnEncoderMove(int direction) {
      // Timesetstate 1 = hours, 2 = minutes, 3 = seconds
      if (TimeSetState >0)
      {
        if (SlowRiseState == HEM_SLOWRISE_STATE_ZERO) 
          {
          SlowRiseDuration[TimeSetState-1]+= direction;
          if (TimeSetState == 1)
          {
            if (SlowRiseDuration[0] < 0) SlowRiseDuration[0] = 9;
            if (SlowRiseDuration[0] > 9) SlowRiseDuration[0] = 0;
          }
          else
          {
            int i = 0;
            if ((TimeSetState==3) && (SlowRiseDuration[0] == 0)  && (SlowRiseDuration[1] == 0)) i = 1;      
            if (SlowRiseDuration[TimeSetState-1] < (0 + i)) SlowRiseDuration[TimeSetState-1] = 59;
            if (SlowRiseDuration[TimeSetState-1] > 59) SlowRiseDuration[TimeSetState-1] = 0 + i;
          }
        }
        else
          {
          SlowRiseDuration_runtime[TimeSetState-1]+= direction;
          if (TimeSetState == 1)
          {
            if (SlowRiseDuration_runtime[0] < 0) SlowRiseDuration_runtime[0] = 9;
            if (SlowRiseDuration_runtime[0] > 9) SlowRiseDuration_runtime[0] = 0;
          }
          else
          {
            int i = 0;
            if ((TimeSetState==3) && (SlowRiseDuration_runtime[0] == 0)  && (SlowRiseDuration_runtime[1] == 0)) i = 1;      
            if (SlowRiseDuration_runtime[TimeSetState-1] < (0 + i)) SlowRiseDuration_runtime[TimeSetState-1] = 59;
            if (SlowRiseDuration_runtime[TimeSetState-1] > 59) SlowRiseDuration_runtime[TimeSetState-1] = 0 + i;
          }
        }       
      }
      else if (VoltSetState)
      {
            int max = HEMISPHERE_MAX_CV / HEM_SLOWRISE_VOLTAGE_INCREMENTS;
            max_voltage = constrain(max_voltage + direction, 0, max);
            ResetSignal();
      }
      
      else
      {
        // Change menu
        MenuState += direction;
        if (MenuState < 0) 
        {
          MenuState = 2;
          if (SlowRiseState == HEM_SLOWRISE_STATE_PAUSED) MenuState = 2;
          if (SlowRiseState == HEM_SLOWRISE_STATE_RUNNING) MenuState = 1;
        }
        if ((SlowRiseState == HEM_SLOWRISE_STATE_RUNNING) && (MenuState == 2)) MenuState = 0;
        if ((SlowRiseState == HEM_SLOWRISE_STATE_PAUSED) && (MenuState == 3)) MenuState = 0;
        if (MenuState == 4) MenuState = 0;
        // If finished? No use turning! Only reset.
        if (SlowRiseState == HEM_SLOWRISE_STATE_FINISHED) MenuState = HEM_SLOWRISE_MENU_RISEFALLRESET;
      }
    }
//*****************************************************
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        // example: pack property_name at bit 0, with size of 8 bits
        // Pack(data, PackLocation {0,8}, property_name); 
        return data;
    }

    void OnDataReceive(uint32_t data) {
        // example: unpack value at bit 0 with size of 8 bits to property_name
        // property_name = Unpack(data, PackLocation {0,8}); 
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "Start/pause  RESET";
        help[HEMISPHERE_HELP_CVS]      = "";
        help[HEMISPHERE_HELP_OUTS]     = "Out1: CV Out2: EOC";
        help[HEMISPHERE_HELP_ENCODER]  = "Settings menu     ";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int32_t SlowRiseTicks; // Ticks Counting
    int32_t SlowRiseTicksDuration; // Duration in Ticks
    int32_t SlowRiseMsDuration; // Duration in Ticks
    short SlowRiseDuration[3]; // Hours, minutes, seconds
    short SlowRiseDuration_runtime[3]; // Hours, minutes, seconds
    int SlowRiseState;
    int MenuState;
    int TimeSetState;
    boolean VoltSetState;
    boolean LapState;
    uint32_t LapTicks;
    boolean RiseFall;
    int max_voltage;
    simfloat signal;

    void ResetRiseFall()
    {
      SlowRiseDuration_runtime[0]=SlowRiseDuration[0];
      SlowRiseDuration_runtime[1]=SlowRiseDuration[1];
      SlowRiseDuration_runtime[2]=SlowRiseDuration[2];
      MenuState = HEM_SLOWRISE_MENU_STARTPAUSE;
      TimeSetState = 0;
      SlowRiseTicks = 0;
      SlowRiseState = HEM_SLOWRISE_STATE_ZERO;
      ResetSignal();
      SetSlowRiseTicksDuration();
    }


    void ResetSignal()
    {
      signal = RiseFall ? int2simfloat(max_voltage * HEM_SLOWRISE_VOLTAGE_INCREMENTS) : 0;
    }
    
    void SetSlowRiseTicksDuration()
    {
      SlowRiseMsDuration = ((((SlowRiseDuration_runtime[0] *60) + SlowRiseDuration_runtime[1]) * 60) + SlowRiseDuration_runtime[2]) * 1000;
      SlowRiseTicksDuration = SlowRiseMsDuration * 100 / 6;
    }
    
    void DrawTime(int32_t showtime, int y)
    {
      if (showtime <= 0)
      {
        gfxPrint(1, y, "0:00:00:00");
      }
      else
      {
        // Draw time in SlowRiseCounter       
        int GetCentiSeconds = showtime / 10; // Total time in 1/100th seconds
        int GetSeconds = GetCentiSeconds / 100; // Tital time in seconds
        GetCentiSeconds = GetCentiSeconds % 100; // Get full seconds out
        int GetMinutes = GetSeconds / 60;
        GetSeconds = GetSeconds % 60;
        int GetHours = GetMinutes / 60;
        GetMinutes = GetMinutes % 60;
        
        gfxPrint(1,y,GetHours);
        gfxPrint(":");

        if (GetMinutes < 10) gfxPrint("0");
        gfxPrint(GetMinutes);
        gfxPrint(":");

        if (GetSeconds < 10) gfxPrint("0");
        gfxPrint(GetSeconds);
        gfxPrint(":");

        if (GetHours < 10) 
        {
          if (GetCentiSeconds < 10) gfxPrint("0");
          gfxPrint(GetCentiSeconds);
        }
        else
        {  // If over 10 hours, we only print deciseconds...
          int GetDeciSeconds = (GetCentiSeconds / 10) % 10; 
          if (GetDeciSeconds == 0) gfxPrint("0");
          else gfxPrint(GetDeciSeconds);          
        }                
      }
    }
    

    void DrawMenu() 
    { 
      
      DrawTime((SlowRiseMsDuration - (SlowRiseTicks *6 / 100)),28);
      gfxPrint(1, 40,"  ");
      // Always show voltage unless reset
      if (SlowRiseState != HEM_SLOWRISE_STATE_ZERO) gfxPrintVoltage(simfloat2int(signal));
            
      if ((TimeSetState > 0) || (MenuState == HEM_SLOWRISE_MENU_SETTIME))
      {
        // SetTimeDisplay   
        if (SlowRiseState == HEM_SLOWRISE_STATE_ZERO)
        {
          gfxPrint(1,16,SlowRiseDuration[0]);
          gfxPrint(":");
          if (SlowRiseDuration[1] < 10) gfxPrint("0");
          gfxPrint(SlowRiseDuration[1]);
          gfxPrint(":");
          if (SlowRiseDuration[2] < 10) gfxPrint("0");
          gfxPrint(SlowRiseDuration[2]);
          gfxPrint(1, 52, " Set Time");
        }
        else
        {
          gfxPrint(1,16,SlowRiseDuration_runtime[0]);
          gfxPrint(":");
          if (SlowRiseDuration_runtime[1] < 10) gfxPrint("0");
          gfxPrint(SlowRiseDuration_runtime[1]);
          gfxPrint(":");
          if (SlowRiseDuration_runtime[2] < 10) gfxPrint("0");
          gfxPrint(SlowRiseDuration_runtime[2]);
          gfxPrint(1, 52, "  Adjust");
        }
        if (TimeSetState == 1) gfxCursor(1, 24, 7);
        else if (TimeSetState == 2) gfxCursor(13, 24, 13);
        else if (TimeSetState == 3) gfxCursor(31, 24, 13);
        else if (SlowRiseState == HEM_SLOWRISE_STATE_ZERO) gfxCursor(7, 61, 49);
        else gfxCursor(13, 61, 36);
        
      }
// MenuItems:
// 0 Start/ctu |   Pause
// 1 Set Time  |   Set Time
// 2 RiseFall/RESET |   
// 3 MaxVolt
/*
#define HEM_SLOWRISE_MENU_STARTPAUSE 0
#define HEM_SLOWRISE_MENU_SETTIME 1
#define HEM_SLOWRISE_MENU_RISEFALLRESET 2
*/
      
      else 
      {
        if (SlowRiseState == HEM_SLOWRISE_STATE_FINISHED)
            {
              gfxPrint(1, 52,"  RESET");
              gfxCursor(13, 61, 31);
            }          
        if (MenuState == HEM_SLOWRISE_MENU_STARTPAUSE) 
          {
            if (SlowRiseState == HEM_SLOWRISE_STATE_RUNNING)
            {
              gfxPrint(1, 52, "  Pause");
              gfxCursor(13, 61, 31);
            }
            else if (SlowRiseState == HEM_SLOWRISE_STATE_ZERO) 
            {
              gfxPrint(1, 52,"  Start");
              gfxCursor(13, 61, 31);
            }
            else if (SlowRiseState == HEM_SLOWRISE_STATE_PAUSED)
            {
              gfxPrint(1, 52," Continue");
              gfxCursor(7, 61, 49);
            }           
        }

        else if (MenuState == HEM_SLOWRISE_MENU_RISEFALLRESET) 
          {
            if (SlowRiseState == HEM_SLOWRISE_STATE_ZERO)
            {
              if (RiseFall) gfxPrint(1, 52, "  Fall");
              else gfxPrint(1, 52, "  Rise");
              gfxCursor(13, 61, 26);
            }
            else 
            {
              gfxPrint(1, 52,"  RESET");
              gfxCursor(13, 61, 31);
            }  
          }
        else if (MenuState == HEM_SLOWRISE_MENU_SETVOLT) 
        {
          gfxPrintVoltage(max_voltage * HEM_SLOWRISE_VOLTAGE_INCREMENTS);
          gfxPrint(1, 52," Max Volt");
          VoltSetState ? gfxCursor(14, 48, 36) : gfxCursor(7, 61, 49);
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
SlowRise SlowRise_instance[2];

void SlowRise_Start(bool hemisphere) {
    SlowRise_instance[hemisphere].BaseStart(hemisphere);
}

void SlowRise_Controller(bool hemisphere, bool forwarding) {
    SlowRise_instance[hemisphere].BaseController(forwarding);
}

void SlowRise_View(bool hemisphere) {
    SlowRise_instance[hemisphere].BaseView();
}

void SlowRise_Screensaver(bool hemisphere) {
    SlowRise_instance[hemisphere].BaseScreensaverView();
}

void SlowRise_OnButtonPress(bool hemisphere) {
    SlowRise_instance[hemisphere].OnButtonPress();
}

void SlowRise_OnEncoderMove(bool hemisphere, int direction) {
    SlowRise_instance[hemisphere].OnEncoderMove(direction);
}

void SlowRise_ToggleHelpScreen(bool hemisphere) {
    SlowRise_instance[hemisphere].HelpScreen();
}

uint32_t SlowRise_OnDataRequest(bool hemisphere) {
    return SlowRise_instance[hemisphere].OnDataRequest();
}

void SlowRise_OnDataReceive(bool hemisphere, uint32_t data) {
    SlowRise_instance[hemisphere].OnDataReceive(data);
}
