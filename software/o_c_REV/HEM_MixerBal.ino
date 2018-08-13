#define MIXER_MAX_VALUE 255

class MixerBal : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Mixer:Bal";
    }

    void Start() {
        balance = 127;
    }

    void Controller() {
        int signal1 = In(0);
        int signal2 = In(1);

        int mix1 = Proportion(balance, MIXER_MAX_VALUE, signal2)
                 + Proportion(MIXER_MAX_VALUE - balance, MIXER_MAX_VALUE, signal1);

        int mix2 = Proportion(balance, MIXER_MAX_VALUE, signal1)
                 + Proportion(MIXER_MAX_VALUE - balance, MIXER_MAX_VALUE, signal2);

        Out(0, mix1);
        Out(1, mix2);
    }

    void View() {
        gfxHeader(applet_name());
        DrawBalanceIndicator();
        gfxSkyline();
    }

    void ScreensaverView() {
        DrawBalanceIndicator();
        gfxSkyline();
    }

    void OnButtonPress() {
    }

    void OnEncoderMove(int direction) {
        balance = constrain(balance + direction, 0, 255);
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,8}, balance);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        balance = Unpack(data, PackLocation {0,8});
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "";
        help[HEMISPHERE_HELP_CVS]      = "1,2 Signals";
        help[HEMISPHERE_HELP_OUTS]     = "A=Mix Out B=Comp";
        help[HEMISPHERE_HELP_ENCODER]  = "Balance";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor;
    int balance;
    
    void DrawBalanceIndicator() {
        gfxFrame(1, 15, 62, 6);
        int x = Proportion(balance, MIXER_MAX_VALUE, 62);
        gfxLine(x, 15, x, 20);
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to MixerBal,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
MixerBal MixerBal_instance[2];

void MixerBal_Start(int hemisphere) {
    MixerBal_instance[hemisphere].BaseStart(hemisphere);
}

void MixerBal_Controller(int hemisphere, bool forwarding) {
    MixerBal_instance[hemisphere].BaseController(forwarding);
}

void MixerBal_View(int hemisphere) {
    MixerBal_instance[hemisphere].BaseView();
}

void MixerBal_Screensaver(int hemisphere) {
    MixerBal_instance[hemisphere].BaseScreensaverView();
}

void MixerBal_OnButtonPress(int hemisphere) {
    MixerBal_instance[hemisphere].OnButtonPress();
}

void MixerBal_OnEncoderMove(int hemisphere, int direction) {
    MixerBal_instance[hemisphere].OnEncoderMove(direction);
}

void MixerBal_ToggleHelpScreen(int hemisphere) {
    MixerBal_instance[hemisphere].HelpScreen();
}

uint32_t MixerBal_OnDataRequest(int hemisphere) {
    return MixerBal_instance[hemisphere].OnDataRequest();
}

void MixerBal_OnDataReceive(int hemisphere, uint32_t data) {
    MixerBal_instance[hemisphere].OnDataReceive(data);
}
