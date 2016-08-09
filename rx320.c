//    Ten-Tec Rx320 Console Controller
//    --------------------------------------
//
//    Key commands are (case insensitive):
//    
//    [General Commands]
//    
//    (T)  Tune to a frequency in MHz
//    (M)  Set mode to am, usb, lsb, or cw
//    (F)  Select a filter
//    (<)  Step down in freq
//    (>)  Step up in freq
//    (S)  Set the tuning step for increment (>), and decrement (<) keys
//    (A)  Select an AGC speed
//    (B)  Band select
//    (Q)  Quit
//    
//    [Volume Commands]
//    
//    (K)  Speaker volume
//    (O)  Line volume
//    (V)  Both speaker and line volume
//    
//    [Extra Commands]
//
//    (P)  Passband Shift for USB or LSB
//    (Z)  Frequency error correction
//    (X)  Displays signal strength meter
//    (Y)  Select a BFO offset for CW mode
//    
//    [Memory Commands]
//    
//    (E)  Enter current frequency and settings in the database
//    (R)  Load a station that was stored
//    (C)  Clear a station entry in the database
//    (L)  Lists all stations stored in the database

#include <stdio.h>       // for printf() and getchar()
#include <stdlib.h>      // for exit()
#include <string.h>      // for strcmp()
#include <math.h>        // for log10()
#include "consts.h"	     // global constants
#include "rx320.h"
#include "utils.h"       // for getch(), getche() and kbhit()
#include "com.h"         // for communication channel
#include "stations.h"    // for memories database

int main(int argc, char* argv[])
{
    char todo;

    // First time use default values
    RADIOFREQ = 10000000;
    MODE      = AM;
    FILTER    = 33;
    BFO       = 600;
    FREQOFF   = 0;
    VOLUME    = 25;
    STEP      = 5000;
    PBSHIFT   = 0;

	printf("\n    ");
	printColoredText("Ten-Tec RX320 Console Controller\n\n",
		ANSICOL_UNDERLINE);

    initList();  // Create a new station memories file if one doesn't exist

    // Load parameters from when radio was used last
    loadState(&RADIOFREQ, &STEP, &MODE, &FILTER, &BFO, &VOLUME, &FREQOFF, &PBSHIFT, device);

    if( !initRadio() )    // Power on and Initialize modes
        return -1;

    do    {
		printf("\x1B[37D");
        printStatus();    // Prints freq, mode, filter

        // Prompts for keys to set a radio parameter
        todo = chlower( (char) getch() );
	
		switch(todo) {
			case ',': stepDown();	continue;
			case '.': stepUp();		continue;
			case 'x': showLevel();	continue;
			default:                break;
		}
		
		printf("\x1B[1K\x1B[37D");
        switch(todo) {
            case 'm': setMode(TRUE);    break;
            case 'f': setFilter(TRUE);  break;
            case 'o': setVolume(TRUE, LINEOUT); break; 
            case 'k': setVolume(TRUE, SPEAKER); break;
            case 'v': setVolume(TRUE, ALLVOL);  break;
            case 'a': setAGC();         break;
            case 'y': setBFO();         break;
            case 'b': setBand();        break;
            case 't': setFreq(TRUE);    break;
            case 's': setStep();        break;
            case 'e':    // Save the station
                addEntry(RADIOFREQ, STEP, MODE, FILTER, BFO, PBSHIFT);
                break;
            case 'r':    // Tune to recalled freq
                recallEntry(&RADIOFREQ, &STEP, &MODE, &FILTER, &BFO, &PBSHIFT);
                setFilter(FALSE); setMode(FALSE);
                AdjustToStep = TRUE;             // Step keys will then round frequency to step 
                break;
            case 'l': listEntries();    break;
            case 'c': deleteEntry();    break;
            case 'p': passbandShift();  break;
            case 'z': freqCorrection(); break;
            case 'q': break;
            default:
				printf("\x1B[2J\x1B[0;0H\n    ");
  				printColoredText("Ten-Tec RX320 Console Controller\n\n",
					ANSICOL_UNDERLINE);
	            showKeyCommands();
                break;
        }
    } 
    while(todo != 'q');    // Program exit

    // Will mute radio if user desires
    printf("\n\nMute radio on exit? (Y/N): ");
    todo = chupper( (char) getche() );
    if(todo != 'N') {
        cmmd[0]='C'; cmmd[1]=0; cmmd[2]=63; cmmd[3]='\r';
        WritePort(cmmd, 4);
    }
    ClosePort();    // Close connection
    // Save the listening state for next use
    saveState(RADIOFREQ, STEP, MODE, FILTER, BFO, VOLUME, FREQOFF, PBSHIFT, device);

	printf("\n");
    return 0;
}

void showKeyCommands(void)
{
	printf("    ");
    printColoredText(" (T)une, (M)ode,  (F)ilter, (V)olume, (S)tep,  (B)and,  (A)gc  ",
		ANSICOL_BLACK | ANSICOL_BGBLUE);
	printf("\n    ");
	printColoredText(" <,  >,  (X)meter (E)nter,  (R)ecall, (L)ist,  (C)lear, (Q)uit ",
         ANSICOL_BLACK | ANSICOL_BGBLUE);
	printf("\n\n");
}

void stepDown(void)            
{
    RADIOFREQ = (RADIOFREQ - STEP) > 100000 ? RADIOFREQ - STEP : 30000000;

    if(AdjustToStep) {
        RADIOFREQ = (long) ((RADIOFREQ + STEP / 2) / STEP) * STEP;
        AdjustToStep = FALSE;  // Don't need to keep rounding every time a step key is pressed
    }
    setFreq(FALSE);
}

void stepUp(void)
{
    RADIOFREQ = (RADIOFREQ + STEP) < 30000000 ? RADIOFREQ + STEP : 100000;

    if(AdjustToStep) {      // Round frequency to step once
        RADIOFREQ = (long) ((RADIOFREQ + STEP / 2) / STEP) * STEP;  
        AdjustToStep = FALSE; 
    }
    setFreq(FALSE);
}

int freqCorrection(void)
{   // User defined frequency calibration value for radio
    long freqOff = FREQOFF;
    printf("Freq correction = %d Hz\nEnter a frequency correction offset (+/-120 Hz): ", FREQOFF);
    if( (!readLong(&freqOff)) || (freqOff > 120) || (freqOff < -120) ) {
        printf("Out of range!\n\n");
        return FALSE;
    }
    FREQOFF = (int) freqOff;
    if(MODE != AM) setFreq(FALSE);  // Re-adjust frequency
    // Save the calibration value FREQOFF in the channels.db header
    saveState(RADIOFREQ, STEP, MODE, FILTER, BFO, VOLUME, FREQOFF, PBSHIFT, device);
    printf("Frequency adjusted.\n\n");
    return TRUE;
}

int passbandShift(void)
{
    long pbs = PBSHIFT;
    // Desired passband location relative to the frequency reference
    // Useful for eliminating interference near the passband in SIDEBAND modes
    if(MODE == CW || MODE == AM) {   
        printf("This option is for LSB/USB\n\n");
        return TRUE;
    }
    printf("Passband shift = %d Hz\nNew Passband shift (+/-1000 Hz): ", PBSHIFT);
    if( (!readLong(&pbs)) || (pbs > 2000) || (pbs < -2000) ) {
        printf("Out of range! Try (-2000 to 2000)\n\n");
        return FALSE;
    }
    PBSHIFT = (int) pbs;	
    setFreq(FALSE);
    return TRUE;
}

int setFreq(int prompt)
{   
    double fTune;    
    long RF, LO1;  
    int coarseVal, fineVal, bfoVal, adjFreq;

    if(prompt) {  // Manually enter a frequency in KHz        
        printf("Freq (Khz): ");
        if( (!readDouble(&fTune)) || (fTune < 100.0) || (fTune > 30000.0) ) {
            printf("Out of range! Try (100.0 to 30000.0)\n\n");
            return FALSE;
        }
        RADIOFREQ = (long) (fTune * 1E3);  // Frequency stord as a long integer in Hz
        AdjustToStep = TRUE;               // Step keys will then round frequency to step
        printf("\n");
    }

    // Adjust frequency to any calibration value
    RF = RADIOFREQ + FREQOFF;                
    switch(MODE) {
        case AM: bfoVal = 0;
            break;
        case USB:
            adjFreq = Filters[FILTER] / 2 + 200 + PBSHIFT;  // Sideband offset  
            RF += adjFreq;                                  // Move frequency above display reference
            // Moves bfo below DSP_IF center 
            bfoVal = (int) (8000 + adjFreq);  
            break;
        case LSB:
            adjFreq = Filters[FILTER] / 2 + 200 + PBSHIFT;  // Sideband offset
            RF -= adjFreq;                                  // Move frequency below display reference
            // Moves bfo above DSP_IF center 
            bfoVal = (int) (8000 + adjFreq);                // Radios LSB mode will flip passband, so still (DSP_IF + adjFreq)
            break;
        case CW:
            // Set bfo above DSP_IF by desired CW tone
            bfoVal = (int) (8000 + BFO);                    // DSP_IF +/- BFO 
            break;
    }

    LO1 = RF + 44997500L;                    // RF + LO2 + IF2   (Ideal LO1)
    coarseVal = (int)((LO1 + 1250) / 2500);  // coarseVal \N  
    LO1 = (long) (coarseVal * 2500);         // LO1 rounded to nearest 2500Hz 
    fineVal = (int)(RF - LO1 + 44998750L);   // RF - LO1 + LO2 + IF2 + 1250;        
    fineVal = (int) (fineVal * 5.46);        // fineTune \N 
    bfoVal  = (int) (bfoVal * 2.73);         // bfoVal \N 

    cmmd[0] = 'N';        // Construct (set frequency) command
    cmmd[1] = (coarseVal >> 8) & 0xFF;
    cmmd[2] = coarseVal & 0xFF;
    cmmd[3] = (fineVal >> 8) & 0xFF;
    cmmd[4] = fineVal & 0xFF;
    cmmd[5] = (bfoVal >> 8) & 0xFF;
    cmmd[6] = bfoVal & 0xFF;
    cmmd[7] = '\r';

    // Send set freq command
    WritePort(cmmd, 8); 
    return TRUE;
}

int setFilter(int prompt)
{
    long flt;
    int selection[6] = { 33, 0, 7, 12, 20, 28 };  // FILTER values for default options

    if(prompt) {
        printf("Select a filter (Hz):\n");    // Prompt for default filter options
        printf("1 (8000), 2 (6000), 3 (3900), 4 (2700), 5 (1500), 6 (525), 7 (more)\n");
        flt = getch();

        if(flt > '7' || flt < '1') {
            printf("Invalid selection!\n\n");
            return FALSE;
        }
        if(flt == '7') {                      // User selected to view all filters 
            printf("\n");
            for(flt = 0; flt < 34; flt++) {   // Display all filter optins
                printf("%2ld %4d    ", flt+1, Filters[flt]);
                if(flt % 6 == 5) printf("\n");
            }
            printf("\n\nFilter: ");           // Prompt for filter
            if( (!readLong(&flt)) || (flt < 1) || (flt > 34) ) {
                printf("Invalid input!\n\n");
                return FALSE;
            }
            FILTER = flt - 1;
        }
        else 
            FILTER = selection[flt - '1'];
        printf("\n");
    }

    // Send 'set filter' command to radio
    cmmd[0] = 'W'; cmmd[1] = (char) FILTER; cmmd[2] = '\r';
    WritePort(cmmd, 3);

    return TRUE;
}

int setMode(int prompt)
{
    char md;

    if(prompt) {  // Prompt for a mode
        printf("Select a mode:\n");
        printf("(A)M, (U)SB, (L)SB, (C)W\n");
        md = chupper( (char) getch() );

        switch(md) {  // Assign default step and filter for selected mode
            case 'A': MODE=AM;  STEP=5000; FILTER=33; break;  // 8000 Hz
            case 'U': MODE=USB; STEP=1000; FILTER=12; break;  // 2700 Hz
            case 'L': MODE=LSB; STEP=1000; FILTER=12; break;  // 2700 Hz
            case 'C': MODE=CW;  STEP=1000; FILTER=20; break;  // 1500 Hz
            default:  printf("Invalid selection!\n\n"); return FALSE;
        }
        printf("\n");
        if( !setFilter(FALSE) )     // Set default filter of selected mode
            return FALSE;
        // Adjust frequency to step
        RADIOFREQ = (long) ((RADIOFREQ + STEP / 2) / STEP) * STEP; 
    }
    cmmd[0]='M'; cmmd[2] = '\r';
    cmmd[1] = (char) (MODE + '0');  // Ascii '0' (am), '1' (usb), '2' (lsb), or '3' (cw)

    // Send set mode command to radio
    WritePort(cmmd, 3);
    
    return setFreq(FALSE);
}

int setVolume(int prompt, int jack)
{   // minimum loudness of atten (0) loudest, to (63) mute. 
    long atten, slider, vol, MIN = 40;   

    if(prompt) {
        if      (jack == LINEOUT) vol = 0;  //  0 - 10 lineout
        else if (jack == SPEAKER) vol = 11; // 11 - 21 speaker
        else                      vol = 22; // 22 - 32 both

        printf("Set volume level (0 - 10): ");
        if( (!readLong(&slider)) || (slider < 0) || (slider > 10) ) {
            printf("Invalid Input!\n\n");
            return FALSE;
        }
        VOLUME = vol + slider;
        printf("\n");
    } 
    if     (VOLUME > 21) cmmd[0] = 'C';  // both line and speaker
    else if(VOLUME > 10) cmmd[0] = 'V';  // speaker
    else                 cmmd[0] = 'A';  // lineout
    cmmd[1]=0; cmmd[2]=63; cmmd[3]='\r'; 

    // Attenuation 1.5db per step
    atten = (VOLUME % 11) ? (MIN - (int) (log10((double) (VOLUME % 11)) * MIN) ) : 63;
    cmmd[2] = (char) atten;

    // Send volume command
    WritePort(cmmd, 4);
    return TRUE;
}

int setAGC(void)
{
    int agc;
    printf("Set AGC level:\n1 (slow), 2 (medium), 3 (fast)\n");
    agc = getch();  // Prompt for AGC

    switch(agc) {
        case '1': printf("AGC = slow\n"); break;
        case '2': printf("AGC = medium\n"); break;
        case '3': printf("AGC = fast\n"); break;
        default:
            printf("Invalid Selection!\n\n"); 
            return FALSE;
    }
    // Send agc command
    cmmd[0]='G'; cmmd[1]='2'; cmmd[3]='\r';
    cmmd[1] = (char) agc;    // Ascii '1' (slow), '2' (medium), '3' (fast)
    printf("\n");
    WritePort(cmmd, 3);

    return TRUE;
}

void setStep(void)
{
    int step;

    printf("Select tuning step:\n");
    printf("1 (5 KHz), 2 (2.5 KHz), 3 (1 KHz), 4 (100 Hz), 5 (10 Hz), 6 (1 Hz)\n");
    step = getch();    // Prompt for frequency step

    switch(step) {
        case '1': STEP = 5000; break;  // Step = 5 KHz 
        case '2': STEP = 2500; break;  // Step = 2.5 KHz
        case '3': STEP = 1000; break;  // Step = 1 KHz
        case '4': STEP = 100;  break;  // Step = 100 Hz
        case '5': STEP = 10;   break;  // Step = 10 Hz
        case '6': STEP = 1;    break;  // Step = 1 Hz
        default:
            printf("Invalid selection!\n\n");
            return;
    }
    // Adjust frequency to step
    RADIOFREQ = (long) ((RADIOFREQ + STEP / 2) / STEP) * STEP;
    setFreq(FALSE);
    printf("step = %dHz\n\n", STEP);
}

int setBFO(void)
{
    int cwBfo = BFO;

    if(MODE != CW) {  
        printf("mode is not CW\n\n");
        return FALSE;
    }  // Set a new BFO value
    printf("BFO = %d\nNew BFO value (Hz): ", BFO);
    if( (!readLong( (long*)&BFO)) || (BFO < 0) || (BFO > 2000) ) {
        printf("Out of range! Try (0 - 2000)\n\n");
        BFO = cwBfo;
        return FALSE;
    }
    printf("\n");
    return setFreq(FALSE);
}

void setBand(void)
{
    int band;
    int targets[] = { -1, 160, 120, 90, 80, 75, 60, 49, 40, 41, 31, 30,
        25, 22, 20, 19, 16, 17, 15, -1, 13, 12, 11, -1, 10 };
    int count = sizeof(band_low) / sizeof(band_low[0]);
    printf("Band: ");
    readLong((long*)&band);
    int i;
    for(i = 0; i < count; ++i) {
        if(band == targets[i]) {
            // tune to band_low[i];
            RADIOFREQ = band_low[i] * 1000;
            setFreq(FALSE);
        }
    }
	printf("\n");
}

int initRadio(void)
{
	
    // Select the communication device
    if( device[0] == '\0' ) {	
        printf("Enter the  communication device to radio. eg: /dev/ttyS0\n");
        fetchLine(device, DEVLEN);
    } else {
		printf("Opening device %s\n", device);
	}
	
    if( !OpenPort(device) ) { 
        device[0] = '\0';	
        return FALSE;
    }

    // Check for a response from radio ( Firmware revision number )
    if( !checkRadioOn() ) {
        printf("Radio is not responding.\n");
        return FALSE;
    }
    if( !setMode(FALSE) )     // Sets initial mode and frequency
        return FALSE;
    if( !setFilter(FALSE) )   // Set initial filter
        return FALSE;
    if( !setVolume(FALSE, ALLVOL) )  // Set initial volume
		return FALSE;
	
	printf("\n"); showKeyCommands();	
	return TRUE;
}

void printStatus(void)
{   // Display the frequency and mode
    double fTune = RADIOFREQ / 1E3;   // Frequency in KHz       
    char mode[4] = "AM ";             // Mode string 
	char freqStr[20];   	

    switch(MODE) {
        case USB: strcpy(mode, "USB"); break;
        case LSB: strcpy(mode, "LSB"); break;
        case CW: strcpy(mode,  "CW "); break;
    }

    //_30000.000 KHz AM  [120m Tropical]___  37 chars
	sprintf(freqStr, "%10.3lf KHz %s ", fTune, mode);
	printColoredText(freqStr, ANSICOL_BOLD | ANSICOL_ITALIC);
	printColoredText(freqAlloc(), ANSICOL_GREEN);
	printf("   ");
}

int checkRadioOn(void)
{
    cmmd[0]='?'; cmmd[1]='\r';   // Request firmware revision # (Radio ON check)

    WritePort(cmmd, 2);
    ResponseWait();
    ReadPort(resp);

    if( findData(resp, "VER", RXBUF) != -1 )
        return TRUE;
    
    return FALSE;
}

void showLevel()
{
	printf("\x1B[s\x1B[20C");
	while( !kbhit() ) {
		printf("\x1B[20D");
		printMeter();
		printf("    ");
	}
	printf("\x1B[u\x1B[K\x1B[1K");
}

void printMeter()
{
	int gain = getMeterLevel() * 15 / 80;	// 0 to 15 for meter
	for(int i = 0; i < 16; i++) {           // print meter "_____|__________"
		if(i == gain) 
			printColoredText("|", ANSICOL_BOLD | ANSICOL_RED | ANSICOL_BGWHITE);
		else
			printColoredText("'", ANSICOL_BLACK | ANSICOL_BGWHITE);					
    }
} 

double getMeterLevel()
{
	unsigned int gain;
	WritePort("X\r", 2);
	ResponseWait();
	ReadPort(resp);
	if(resp[0] != 'X') 
		return 0;
	
	gain = (unsigned char) resp[1];
	gain <<= 8;
	gain += (unsigned char) resp[2];	// Signal range from near 0 to near 10000
	return 20.0 * log10(gain);			// dB range
}

// Return a string describing the tuned frequency
char* freqAlloc()
{
  
  int freq = (int) (RADIOFREQ / 1e3);
  int count = sizeof(band_low) / sizeof(band_low[0]);
  int i;
  char* blank = "[      -      ]";

  for(i = 0; i < count; ++i) 
      if(freq >= band_low[i] && freq <= band_high[i]) 
          return band_name[i];

  return blank;  // No band detected
}

