#ifndef _STATIONS_H
#define _STATIONS_H

#define DESCLEN         60    // Station description length
#define ERR_CREATE_FILE 1
#define ERR_OPEN_FILE   2
#define ERR_WRITE_FILE  3
#define ERR_READ_FILE   4
#define ERR_SEEK_FILE   5
#define DB_FILE			"channels.db"

typedef struct _CHANNEL {    // Station database structure
    unsigned int  ref;
    long  freq;
    short step;
    char  mode;
    char  filter;
    short bfo;
    short pbshift;
    char  station[DESCLEN];
} CHANNEL;

typedef struct _HEADER {    // Common variables for database file
    unsigned int  num_entries;
    unsigned int  num_empty;
    char   freqOff;
    long   lastFreq;
    short  lastStep;
    char   lastMode;
    char   lastFilter;
    short  lastBfo;
    short  lastPbshift;
    char   lastVolume;
    char   devname[DEVLEN]; // DEVLEN in consts.h
} HEADER;

int initList(void);
int addEntry(long freq, int step, int mode, int filter, int bfo, int pbshift);
int saveState(long freq, int step, int mode, int filter, int bfo, int vol, int freqOff, int pbshift, char* devname);
int listEntries(void);
int deleteEntry(void);
int recallEntry(long* freq, int* step, int* mode, int* filter, int* bfo, int* pbshift);
int loadState(long* freq, int* step, int* mode, int* filter, int* bfo, int* vol, int* freqOff, int* pbshift, char* devname);

#endif
