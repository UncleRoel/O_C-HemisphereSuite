// Arpeggiator 
// applet for Hemisphere by Roel Das
// V0.3
// Changelog: 
// - added Offset CV in, BPM mod moves to CV in 2! 
//   (Arp_range modulation wasn't implemented anyway.)
// - Added clock multiplication. (Dividing the clock /32 is not very common anyway. If higher values are needed they can be changed in the hem_arp_divisions[] array


#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "OC_scales.h"
// 11 + 5 + 3 + 5 + 2
// Max and min clocks in bpm
#define HEM_ARP_CLK_HIGH 2048 
#define HEM_ARP_CLK_LOW 1
// Max and min division 
#define HEM_ARP_DIV_LOW 0  //still limited to 5 bits, signed now....
#define HEM_ARP_DIV_HIGH 31 
// Max mod ranges
#define HEM_ARP_CLKCV_LOW 0
#define HEM_ARP_CLKCV_HIGH 7 
#define HEM_ARP_RANGE_LOW 2 
//#define HEM_ARP_RANGE_HIGH 33 // SAVING FIX! 
#define HEM_ARP_RANGE_HIGH 17 
#define HEM_ARP_ORDER_HIGH 3 
#define HEM_ARP_DRAW_CLK_CYCLES 40 

#define SEL_ARP_CLK 0
#define SEL_ARP_CLK_CV 1
#define SEL_ARP_CHORD 2
#define SEL_ARP_ORDER 3
#define SEL_ARP_RANGE 4

#define UP 0
#define DOWN 1

struct hem_arp_chord {
  const char* chord_name;
  const int chord_tones[12];
  const size_t nr_notes;
  const int octave_span;
};

const int Nr_of_arp_chords = 55; // counting from 0....

// 1 b2 2 b3 3 4 b5 5 b6 6 b7  7  8
// 0  1 2  3 4 5  6 7  8 9 10 11 12

const hem_arp_chord Arp_Chords[] = {
  //ARPEGGIATE
  {"Octaves",{0},1,1},
  {"1 5",{0, 7},2,1},
  {"Maj triad",{0, 4, 7}, 3,1},
  {"Maj inv 1",{4, 7, 12}, 3,1},
  {"Maj inv 2",{7, 12, 16}, 3,1},
  {"min triad",{0, 3, 7}, 3,1},
  {"min inv 1",{3, 7, 12}, 3,1},
  {"min inv 2",{7, 12, 15}, 3,1},
  {"dim triad",{0, 3, 6},3,1},
  
  {"aug triad",{0, 4, 8},3,1},
  {"sus2",{0,2,7},3,1},
  {"sus4",{0,5,7},3,1},
  {"add2",{0,2,4,7},4,1},
  {"min(add2)",{0,2,3,7},4,1},
  
  {"add4",{0,4,5,7},4,1},
  {"min(add4)",{0,3,5,7},4,1},
  {"sus4(add2)",{0,2,6,7},4,1},
  {"1 2 3 4 5",{0,2,4,5,7},5,1},
  {"1 2 b3 4 5",{0,2,3,5,7},5,1},
  
  {"add(#11)",{0,4,6,7},4,1},
  {"Maj6",{0,4,7,9},4,1},
  {"min6",{0,3,7,9},4,1},
  {"Maj7",{0,4,7,11},4,1},
  {"7",{0,4,7,10},4,1},
  ////////     20     /////
  {"7sus2",{0,2,7,10},4,1},
  {"7sus4",{0,5,7,10},4,1},
  {"min7",{0,3,7,10},4,1},
  {"dim7",{0,3,6,8},4,1},
  {"Maj9",{0,4,7,11,14},5,1},

  {"Maj6/9",{0,4,7,9,14},5,1},
  {"Maj#11",{0,4,7,11,14,18},6,2},
  {"9",{0,4,7,10,14},5,1},
  {"7(b9)",{0,4,7,10,13},5,1},
  {"7(b9,b13)",{0,4,7,10,13,20},6,2},
  ////////     30     /////  
  {"Ionian",{0,2,4,5,7,9,11},7,1},
  {"Dorian",{0,2,3,5,7,9,10},7,1},
  {"Phrygian",{0,1,3,5,7,8,10},7,1},
  {"Lydian",{0,2,4,6,7,9,11},7,1},
  {"Mixolydian",{0,2,4,5,7,9,10},7,1},

  {"Aeolian",{0,2,3,5,7,8,10},7,1},
  {"Locrian",{0,1,3,5,6,8,10},7,1},
  {"Harm Min",{0,2,3,5,7,8,11},7,1},
  {"Mel Min",{0,2,3,5,7,9,11},7,1},
  {"Penta",{0,2,4,7,9},5,1},
  //////////    40 ////////
  {"min Penta",{0,3,5,7,10},5,1},
  {"Maj Blues",{0,2,3,4,7,9},6,1},
  {"min Blues",{0,3,5,6,7,10},6,1},
  {"Bebop",{0,2,4,5,7,9,10,11},8,1},
  {"WholeTone",{0,2,4,6,8,10},6,1},
  
  {"Dim 1 1/2",{0,2,3,5,6,8,9,11},8,1},
  {"Dim 1/2 1",{0,2,3,5,6,8,9,11},8,1},
  {"Altered",{0,1,3,4,6,8,10},7,1},
  {"Chromatic",{0,1,2,3,4,5,6,7,8,9,10,11},12,1},
  {"All Fourth", {0,5,10,15,20,26,31},7,3},
  //////////    50 ////////
  {"All Fifths", {0,7,14,21,28,35,41},7,4},
  {"Corny Pop", {12,11,12,7,3,7,0,0},8,1}
};


const int hem_arp_cvmods[8] = {0,2,4,8,16,24,32,64};
const char* hem_arp_order_names[] = {"up     ","down   ","updown ","random "};
// Presets for clock division
const int hem_arp_divisions[32] = { /*All multiplications end up -2, so 0 means 1/2 Mult*/ -14, -13,-12,-11, -10,-9,-8,-7, -6, -5,-4,-3, -2,-1,0,/*div*/1,2,3,4, 5,6,7,8, 9,10,11,12, 13,14,15,16,32};


class Arpeggiator : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "Arpeggiate";
    }

	/* Run when the Applet is selected */
    void Start() {
        next_note_countdown = 0;
        bpm = 200;
        current_bpm = bpm;
        divider = 1;
        current_div = hem_arp_divisions[divider];
        clk_cv_range = 4;
        selected = 0;
        note_nr_in_arp = 0;
        selected_chord = 2;
        arp_range = 4;
        current_note = 0;
        receiving_clocks = 0; // If we receive 1 clock in, we switch to external clock!!
        updown_direction = 0;
        last_clock = OC::CORE::ticks;
        next_clock = last_clock;
        tick_at_last_note = last_clock; 
        cycle_time = 0;
        
        quantizer.Init();
        quantizer.Configure(OC::Scales::GetScale(5), 0xffff); // Semi-tone
    }
  /* Run during interrupt service routine, 16667 times per second */
    void Controller() {
        int this_tick = OC::CORE::ticks;  
        // Handle reset in
        if (Clock(1)) {
          next_note_countdown = 0;
          note_nr_in_arp = 0;
          updown_direction = UP;
          next_clock = this_tick;
        }
        // Take CV modulation into account before calculating 
        current_bpm = bpm + Proportion(In(1),HEMISPHERE_MAX_CV,hem_arp_cvmods[clk_cv_range]);
        // Constrain is needed to prevent negative BPM!
        current_bpm = constrain(current_bpm, HEM_ARP_CLK_LOW, HEM_ARP_CLK_HIGH);
        current_div = hem_arp_divisions[divider] + Proportion(In(1),HEMISPHERE_MAX_CV,hem_arp_cvmods[clk_cv_range]);
        //current_div = constrain(current_div, HEM_ARP_DIV_LOW, HEM_ARP_DIV_HIGH);
        // We don't need to constrain! Just get values 0 and -1 out!
        if (current_div <= 0) current_div-=2;
       
        // Handle clock in
        if (Clock(0)) {
            // The input was clocked; set timing info
            cycle_time = this_tick - last_clock;
            // Is this the first clock we receive?
            if (!receiving_clocks) { 
              receiving_clocks = 1; // Once we receive a clock we stay in this mode (until scale is changed!)
              next_note_countdown = 0; // next_note_countdown becomes our divider counter!!
              note_nr_in_arp = 0; // Reset arpeggio
              updown_direction = UP;
            }
            // Handle Clock Div
            if (current_div >0) { // positive value indicates clock division
              // Countdown finished? play next note!
              if (next_note_countdown <= 0) 
              {
                next_note_countdown = current_div;
                PlayNextNote();
                ClockOut(1);
                draw_clock = HEM_ARP_DRAW_CLK_CYCLES;    
              }
              next_note_countdown-=1;
            }
            // Clock Mult: clock received: reset calculation cycle!
            else {  
               // Calculate next clock for multiplication on each clock
              int clock_every = (cycle_time /-current_div);
              next_clock = this_tick + clock_every;
              // if we played a note just a fraction before this new clock, we ignore this one! Prevent retriggering!
              if (int(this_tick-tick_at_last_note)>clock_every/4) {
                PlayNextNote();
                ClockOut(1);
                draw_clock = HEM_ARP_DRAW_CLK_CYCLES;  
              }
              
              last_clock = this_tick;
            }
          
        }
        
        else  // We didn't receive a clock, just a normal cycle
           // If we haven't received any triggers; we use internal clock!!
            if (!receiving_clocks){
              if (next_note_countdown == 0) {
                next_note_countdown = 1000000 /current_bpm; // 1.000.000 ticks/min - 60us/tick - bpm
                PlayNextNote();
                ClockOut(1);
                draw_clock = HEM_ARP_DRAW_CLK_CYCLES;    
              }
              next_note_countdown-=1;
            }
            else // Listening for clocks, but no clock received;
              // handle clock multiplication
              if (current_div < 0) {
                if (this_tick >= next_clock) {
                  int clock_every = (cycle_time / -current_div);
                  next_clock += clock_every;
                  PlayNextNote();
                  ClockOut(1);
                  draw_clock = HEM_ARP_DRAW_CLK_CYCLES;
                  tick_at_last_note = this_tick;    
                }
              }
                    
        

        
        // output note
        int32_t outputter = In(0)+quantizer.Lookup(current_note + 48);
        // Not really sure how to constrain here and if needed.
        //Out(0, constrain(outputter, -HEMISPHERE_MAX_CV, HEMISPHERE_MAX_CV));
        Out(0, outputter);
        
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
      if (selected >= 5) selected = 0;
    }

	/* Called when the encoder for this hemisphere is rotated
	 * direction 1 is clockwise
	 * direction -1 is counterclockwise
	 */
    void OnEncoderMove(int direction) {
      if (receiving_clocks) { // receiving clocks!
        if (selected == SEL_ARP_CLK) {
          divider += direction;
          divider = constrain(divider, HEM_ARP_DIV_LOW, HEM_ARP_DIV_HIGH);
          next_note_countdown = 0; // Reset division
        }
      }
      else { // internal clock!
        if (selected == SEL_ARP_CLK) {
          bpm += direction;
          bpm = constrain(bpm, HEM_ARP_CLK_LOW, HEM_ARP_CLK_HIGH);
        }
      }      
      if (selected == SEL_ARP_CLK_CV) {
        clk_cv_range += direction;
        clk_cv_range = constrain(clk_cv_range, HEM_ARP_CLKCV_LOW, HEM_ARP_CLKCV_HIGH);
      }
      if (selected == SEL_ARP_CHORD) {
        selected_chord += direction;
        selected_chord = constrain(selected_chord, 0, Nr_of_arp_chords);
        receiving_clocks = 0;
        updown_direction = UP;
        note_nr_in_arp = 0;
      }
      if (selected == SEL_ARP_ORDER) {
        arp_order += direction;
        arp_order = constrain(arp_order, 0, HEM_ARP_ORDER_HIGH);
        updown_direction = UP; // reset direction on select!
        note_nr_in_arp = 0; // reset note
      }      
      if (selected == SEL_ARP_RANGE) {
        arp_range += direction;
        arp_range = constrain(arp_range, HEM_ARP_RANGE_LOW, HEM_ARP_RANGE_HIGH);
      }
    }
        
    /* Each applet may save up to 32 bits of data. When data is requested from
     * the manager, OnDataRequest() packs it up (see HemisphereApplet::Pack()) and
     * returns it.
     */
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,11}, bpm-1 );
        Pack(data, PackLocation {11,3}, clk_cv_range);
        Pack(data, PackLocation {14,5}, divider+15 );
        //SAVING FIX
        //Pack(data, PackLocation {19,5}, arp_range-2);
        //Pack(data, PackLocation {24,6}, selected_chord );
        //Pack(data, PackLocation {30,2}, arp_order );
        Pack(data, PackLocation {19,4}, arp_range-2);
        Pack(data, PackLocation {23,6}, selected_chord );
        Pack(data, PackLocation {29,2}, arp_order );
        // example: pack property_name at bit 0, with size of 8 bits
        // Pack(data, PackLocation {0,8}, property_name); 
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
        bpm = Unpack(data, PackLocation {0,11}) + 1 ;
        clk_cv_range = Unpack(data, PackLocation {11,3}) ;
        divider = Unpack(data, PackLocation {14,5})-15;
        //SAVING FIX
        //arp_range = Unpack(data, PackLocation {19,5}) +2;
        //selected_chord = Unpack(data, PackLocation {24,6}) ;
        //arp_order = Unpack(data, PackLocation {30,2}) ;
        arp_range = Unpack(data, PackLocation {19,4}) +2;
        selected_chord = Unpack(data, PackLocation {23,6}) ;
        arp_order = Unpack(data, PackLocation {29,2}) ;
    }

protected:
    /* Set help text. Each help section can have up to 18 characters. Be concise! */
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clk 2=Reset";
        help[HEMISPHERE_HELP_CVS]      = "1=CV Offset 2=BPM";
        help[HEMISPHERE_HELP_OUTS]     = "A=V/oct B=Trig";
        help[HEMISPHERE_HELP_ENCODER]  = "Arpeggiator!";
        //                               "------------------" <-- Size Guide
    }
    
private:
    braids::Quantizer quantizer;
    uint16_t bpm; // Amount of clocks per minute
    uint32_t next_note_countdown; // Tick number for the next output (for clock multiply)
    int selected;
    int draw_clock;
    int clk_cv_range;
    int current_bpm; // BPM with CV added
    int divider;
    int current_div;
    int note_nr_in_arp; // Number of the note in the arpeggio
    int selected_chord;
    int arp_range;
    int current_note;
    boolean receiving_clocks;
    int arp_order;
    boolean updown_direction;
    int last_note; // to prevent repeated notes in random mode!
    int next_clock; // Tick number for the next output (for clock multiply)
    int last_clock; // The tick number of the last received clock
    int cycle_time; // Cycle time between the last two clock inputs
    uint32_t tick_at_last_note; //Last tick a note was played, to prevent very short notes!
    
    void DrawSelector() {
        int x=0, l ,y ;
        if (selected <3) { l=63; y = 16 + (selected * 12);}
        else {
          y = 52;
          if ( selected == 3) l = 41;
          else {x = 41; l = 22;}
        }
        gfxCursor(x, y + 9, l);
        
        if (receiving_clocks) {
          // Dividing
          if (current_div > 0){
            gfxPrint(2, 16, "1/");
            gfxPrint(current_div);
            gfxPrint(" DIV");
          }
          // Multiplicating
          else {
            gfxPrint(2, 16, "X");
            gfxPrint(-current_div);
            gfxPrint(" MULT");
          }
        }
        else {
          gfxPrint(2, 16, current_bpm);
          gfxPrint(" BPM");
        }
        gfxPrint(2, 28, "mod ");
        gfxPrint(hem_arp_cvmods[clk_cv_range]);
        gfxPrint(2, 40, Arp_Chords[selected_chord].chord_name);
        gfxPrint(2, 52, hem_arp_order_names[arp_order]);
        gfxPrint(arp_range);   
        
        if (draw_clock > 0) {
          gfxRect(54,16,7,7);
          draw_clock -= 1;        
        }
        else gfxFrame(54,16,7,7);
    }

    void PlayNextNote() {
      // UP
      switch (arp_order) 
      {
        case 0: // UP
          if (note_nr_in_arp >= arp_range) note_nr_in_arp = 0; // reduce the notes to the number specified in the range
          GetNote();
          note_nr_in_arp+=1;
          break;
        case 1: // DOWN
          if (note_nr_in_arp == 0) note_nr_in_arp = arp_range; // reduce the notes to the number specified in the range
          note_nr_in_arp-=1;
          GetNote();
          break;          
        case 2: // UPDOWN
          if (note_nr_in_arp >= arp_range) {updown_direction = DOWN; note_nr_in_arp = arp_range-1;} // reduce the notes to the number specified in the range
          if ((note_nr_in_arp == 0)&(updown_direction==DOWN)) {updown_direction = UP; note_nr_in_arp = 1;}// reduce the notes to the number specified in the range
          if (updown_direction) note_nr_in_arp-=1;
          GetNote();
          if (!updown_direction) note_nr_in_arp+=1;
          break;
        case 3: // RANDOM
          while (last_note == note_nr_in_arp) note_nr_in_arp = rand() % arp_range; // get repeats out!
          last_note = note_nr_in_arp;
          GetNote();
          break;
      }

    }
    void GetNote() {
      // Find the chord tone + add octaves if there more notes! 
      int octaves = (note_nr_in_arp / Arp_Chords[selected_chord].nr_notes) *12 * Arp_Chords[selected_chord].octave_span;
      current_note = Arp_Chords[selected_chord].chord_tones[(note_nr_in_arp % Arp_Chords[selected_chord].nr_notes)] + octaves;
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
Arpeggiator Arpeggiator_instance[2];

void Arpeggiator_Start(int hemisphere) {
    Arpeggiator_instance[hemisphere].BaseStart(hemisphere);
}

void Arpeggiator_Controller(int hemisphere, bool forwarding) {
    Arpeggiator_instance[hemisphere].BaseController(forwarding);
}

void Arpeggiator_View(int hemisphere) {
    Arpeggiator_instance[hemisphere].BaseView();
}

void Arpeggiator_Screensaver(int hemisphere) {
    Arpeggiator_instance[hemisphere].BaseScreensaverView();
}

void Arpeggiator_OnButtonPress(int hemisphere) {
    Arpeggiator_instance[hemisphere].OnButtonPress();
}

void Arpeggiator_OnEncoderMove(int hemisphere, int direction) {
    Arpeggiator_instance[hemisphere].OnEncoderMove(direction);
}

void Arpeggiator_ToggleHelpScreen(int hemisphere) {
    Arpeggiator_instance[hemisphere].HelpScreen();
}

uint32_t Arpeggiator_OnDataRequest(int hemisphere) {
    return Arpeggiator_instance[hemisphere].OnDataRequest();
}

void Arpeggiator_OnDataReceive(int hemisphere, uint32_t data) {
    Arpeggiator_instance[hemisphere].OnDataReceive(data);
}
