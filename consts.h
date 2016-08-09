#ifndef _CONSTS_H
#define _CONSTS_H

#define TRUE  1
#define FALSE 0
#define DEVLEN 30

#define AM    0          // Specified modes for radio
#define USB   1
#define LSB   2
#define CW    3

// Stores the current radio parameters
long RADIOFREQ;
int  MODE;
int  FILTER;
int  BFO;
int  FREQOFF;
int  VOLUME;
int  STEP;
int  PBSHIFT;
char device[DEVLEN];

#endif
