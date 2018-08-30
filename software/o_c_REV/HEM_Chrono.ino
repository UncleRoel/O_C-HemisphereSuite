// Roel's Chronometer for Hemisphere

#define HEM_CHRONO_STATE_ZERO 0
#define HEM_CHRONO_STATE_RUNNING 1
#define HEM_CHRONO_STATE_PAUSED 2
#define HEM_CHRONO_MENU_STARTPAUSE 0
#define HEM_CHRONO_MENU_RESETLAP 1


class Chrono : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Chrono";
    }

    void Start() {
      ChronoTicks = 0; // time in ms
      ChronoState = HEM_CHRONO_STATE_ZERO;
      MenuState = HEM_CHRONO_MENU_STARTPAUSE;
      LapState = 0;
    }

    void Controller() {
      if (Clock(0)) {
        if (ChronoState == HEM_CHRONO_STATE_RUNNING) ChronoState = HEM_CHRONO_STATE_PAUSED;
        else 
        {
          ChronoState = HEM_CHRONO_STATE_RUNNING;
        }
      }
      // On left trigger in: RESET
      if (Clock(1)) {
        ChronoState = HEM_CHRONO_STATE_ZERO;
        LapState = 0;
        ChronoTicks = 0;   
      }      
      if ( ChronoState == HEM_CHRONO_STATE_RUNNING) ChronoTicks+=1;
    }

    void View() {
        gfxHeader(applet_name());
        DrawChrono(ChronoTicks,28);
        if (LapState) DrawChrono(LapTicks,16);
        DrawMenu();
        // Add other view code as private methods
    }

    void ScreensaverView() {
        DrawChrono(ChronoTicks,28);
        if (LapState) DrawChrono(LapTicks,16);
    }

    void OnButtonPress() {
      // If chrono is running, pause or LAP!
      if (ChronoState == HEM_CHRONO_STATE_RUNNING)
      {
        if (MenuState == HEM_CHRONO_MENU_STARTPAUSE) ChronoState = HEM_CHRONO_STATE_PAUSED;
        // SET LAP TIME
        else {
          LapState = 1; 
          LapTicks = ChronoTicks;
        }
      }
      // If chrono is reset: start!
      else if (ChronoState == HEM_CHRONO_STATE_ZERO) {
        ChronoState = HEM_CHRONO_STATE_RUNNING;
      }
      // If chrono is paused!
      else if (MenuState == HEM_CHRONO_MENU_STARTPAUSE)  
      {
        ChronoState = HEM_CHRONO_STATE_RUNNING;
      } // RESET
      else if (MenuState == HEM_CHRONO_MENU_RESETLAP)
      {
        ChronoState = HEM_CHRONO_STATE_ZERO;
        ChronoTicks = 0;  
        LapState = 0;   
      }
      MenuState = HEM_CHRONO_MENU_STARTPAUSE;
    }

    void OnEncoderMove(int direction) {
      if ((ChronoState == HEM_CHRONO_STATE_PAUSED)||(ChronoState == HEM_CHRONO_STATE_RUNNING)) MenuState = !MenuState;
    }
        
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
        help[HEMISPHERE_HELP_OUTS]     = "";
        help[HEMISPHERE_HELP_ENCODER]  = "Start/pause/reset";
        //                               "------------------" <-- Size Guide
    }
    
private:
    uint32_t ChronoCounter; // time in ms
    uint32_t ChronoTicks;
    int ChronoState;
    boolean MenuState;
    boolean LapState;
    uint32_t LapTicks;

    void DrawChrono(uint32_t ticks, int y)
    {
      if (ticks == 0)
      {
        gfxPrint(1, y, "0:00:00:00");
      }
      else
      {
        uint64_t showtime = ticks*6/100; // Convert to ms
        // Draw time in ChronoCounter       
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
      if (ChronoState == HEM_CHRONO_STATE_RUNNING)
      {
        if (MenuState == HEM_CHRONO_MENU_STARTPAUSE) 
        {
          gfxPrint(1, 52, "  Pause");
          gfxCursor(13, 61, 31);
        }
        else
        {
          gfxPrint(1, 52, "   Lap");
          gfxCursor(19, 61, 20);
        }
      }
      else if (ChronoState == HEM_CHRONO_STATE_ZERO) 
      {
        gfxPrint(1, 52,"  Start");
        gfxCursor(13, 61, 31);
      }
      else if (ChronoState == HEM_CHRONO_STATE_PAUSED){
        if (MenuState == HEM_CHRONO_MENU_STARTPAUSE) 
        {
          gfxPrint(1, 52," Continue");
          gfxCursor(7, 61, 49);
        }
        else 
        {
          gfxPrint(1, 52,"  RESET");
        gfxCursor(13, 61, 31);
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
Chrono Chrono_instance[2];

void Chrono_Start(bool hemisphere) {
    Chrono_instance[hemisphere].BaseStart(hemisphere);
}

void Chrono_Controller(bool hemisphere, bool forwarding) {
    Chrono_instance[hemisphere].BaseController(forwarding);
}

void Chrono_View(bool hemisphere) {
    Chrono_instance[hemisphere].BaseView();
}

void Chrono_Screensaver(bool hemisphere) {
    Chrono_instance[hemisphere].BaseScreensaverView();
}

void Chrono_OnButtonPress(bool hemisphere) {
    Chrono_instance[hemisphere].OnButtonPress();
}

void Chrono_OnEncoderMove(bool hemisphere, int direction) {
    Chrono_instance[hemisphere].OnEncoderMove(direction);
}

void Chrono_ToggleHelpScreen(bool hemisphere) {
    Chrono_instance[hemisphere].HelpScreen();
}

uint32_t Chrono_OnDataRequest(bool hemisphere) {
    return Chrono_instance[hemisphere].OnDataRequest();
}

void Chrono_OnDataReceive(bool hemisphere, uint32_t data) {
    Chrono_instance[hemisphere].OnDataReceive(data);
}
