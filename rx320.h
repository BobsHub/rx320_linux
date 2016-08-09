#ifndef _RX320_H
#define _RX320_H

#define LINEOUT 0        // Specified jacks for audio output
#define SPEAKER 1
#define ALLVOL  2
#define DISPLAY_LEN 63

char meter[17];          // Meter display string
int AdjustToStep = TRUE; // Whether or not to round frequency to step  after pressing '>' or '<' 
                         // Cleans up display. For example entering 14.100372 with step of 1Khz,  then '<' 
                         // will show 14099.000 instead of 14099.372

int Filters[34] = {      // Useable DSP filters for radio
    6000, 5700, 5400, 5100, 4800, 4500, 4200, 3900, 3600, 3300, 3000, 2850,
    2700, 2550, 2400, 2250, 2100, 1950, 1800, 1650, 1500, 1350, 1200, 1050, 
    900, 750, 675, 600, 525, 450, 375, 330, 300, 8000 };

int band_low[] = {
    530, 1800, 2300, 3200, 3500, 3900,
    4750, 5900, 7000, 7200, 9400, 10100,
    11600, 13570, 14000, 15100, 17480,
    18068, 18900, 21000, 21450, 24890,
    25600, 26965, 28000 };
int band_high[] = {
    1700, 2000, 2495, 3400, 3900, 4000,
    5060, 6200, 7200, 7450, 9900, 10150,
    12100, 13870, 14350, 15800, 17900,
    18168, 19020, 21450, 21850, 24990,
    26100, 27405, 29700 };
char* band_name[] = {
    "[MW Broadcast] ",
    "[160m Ham]     ",
    "[120m Tropical]",
    "[90m Tropical] ",
    "[80m Ham]      ",
    "[75m Tropical] ",
    "[60m Tropical] ",
    "[49m Broadcast]",
    "[40m Ham]      ",
    "[41m Broadcast]",
    "[31m Broadcast]",
    "[30m Ham]      ",
    "[25m Broadcast]",
    "[22m Broadcast]",
    "[20m Ham]      ",
    "[19m Broadcast]",
    "[16m Broadcast]",
    "[17m Ham]      ",
    "[15m Broadcast]",
    "[15m Ham]      ",
    "[13m Broadcast]",
    "[12m Ham]      ",
    "[11m Broadcast]",
    "[CB]           ",
    "[10m Ham]      " }; 

int  initRadio(void);          // Initialize COM port set first time radio modes
int  checkRadioOn(void);       // Check if radio responds to input
void printStatus(void);        // Display frequency, mode and filter
void showKeyCommands(void);    // Keyboard shortcut list
int  setFreq(int prompt);      // Send Frequency command from RADIOFREQ value
int  setFilter(int prompt);    // Send Filter command for selected useable Filters[FILTER]         
int  setMode(int prompt);      // Send Mode command from MODE value
int  setVolume(int prompt, int jack);  // Sets the volume of the specified output jack
int  setAGC(void);             // Send Automatic Gain Control command (slow, medium, fast)
void setStep(void);            // Assign STEP frequency for '<' and '>' step keys
int  setBFO(void);             // Assign desired tone (BFO) for CW listening
void stepDown(void);           // Move frequency down by STEP value when '<' key is pressed
void stepUp(void);             // Move frequency up by STEP value when '>' key is pressed
int  freqCorrection(void);     // Assign a frequency calibration value (+/-Hz) 
int  passbandShift(void);      // Moves the passband relative to the frequency reference
char* freqAlloc(void);         // Fetches band name from RADIOFREQ
void setBand(void);            // Select a band
double getMeterLevel(void);	   // Meter level from received station
void   printMeter(void);	   // Print a meter face
void   showLevel(void);	       // Continuous signal strength meter
#endif
