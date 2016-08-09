#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "consts.h"
#include "stations.h"
#include "utils.h"

int initList(void)
{
    FILE* fptr;
    if((fptr = fopen(DB_FILE, "r+b")) == NULL) {
        // First time radio settings (defined in constants.h)
        HEADER Header;
        Header.num_empty = Header.num_entries = 0;
        Header.freqOff     = (char) FREQOFF;
        Header.lastFreq    = RADIOFREQ;
        Header.lastStep    = (short) STEP;
        Header.lastMode    = (char)  MODE;
        Header.lastFilter  = (char)  FILTER;
        Header.lastBfo     = (short) BFO;
        Header.lastPbshift = (short) PBSHIFT;
        Header.lastVolume  = (char)  VOLUME;
	Header.devname[0] = '\0';

        // Open a new save file if one doesn't exist
        if((fptr = fopen(DB_FILE, "wb")) == NULL) {
            fprintf(stderr, "Error creating file %s. %s\n", DB_FILE, strerror(errno));
            exit(ERR_CREATE_FILE);
        }
        errno = 0;
        // Write a blank header
        if( (fwrite(&Header, sizeof(Header), 1, fptr) != 1) || errno ) {
            fprintf(stderr, "Error creating header in file %s. %s\n", DB_FILE, strerror(errno));
            fclose(fptr);
            exit(ERR_WRITE_FILE);
        }
    }
    fclose(fptr);
    return TRUE;
}

int loadState(long* freq, int* step, int* mode, int* filter, int* bfo, int* vol, int* freqOff, int* pbshift, char* devname)
{
    FILE* fptr;
    HEADER Header;
    errno = 0;

    if((fptr = fopen(DB_FILE, "rb")) == NULL) {   // read existing file (binary mode)
        fprintf(stderr, "Error opening file %s. %s\n", DB_FILE, strerror(errno));
        exit(ERR_OPEN_FILE);
    }
    // Read header
    if( (fread(&Header, sizeof(Header), 1, fptr) != 1) || errno || feof(fptr) ) {
        fprintf(stderr, "Error reading header in file %s. %s\n", DB_FILE, strerror(errno));
        fclose(fptr);
        exit(ERR_READ_FILE);
    }
    fclose(fptr);

    *freq    = Header.lastFreq;       // Assign the radio state 
    *step    = Header.lastStep;
    *mode    = Header.lastMode;
    *filter  = Header.lastFilter;
    *bfo     = Header.lastBfo;
    *pbshift = Header.lastPbshift;
    *vol     = Header.lastVolume;
    *freqOff = Header.freqOff;
    memcpy(devname, &Header.devname[0], DEVLEN);	

    return TRUE;
}

int saveState(long freq, int step, int mode, int filter, int bfo, int vol, int freqOff, int pbshift, char* devname)
{
    FILE* fptr;
    HEADER Header;
    errno = 0;

    if((fptr = fopen(DB_FILE, "r+b")) == NULL) {   // open file for writing (binary mode)
        fprintf(stderr, "Error opening file %s. %s\n", DB_FILE, strerror(errno));
        exit(ERR_OPEN_FILE);
    }
    // Read header
    if( (fread(&Header, sizeof(Header), 1, fptr) != 1) || errno || feof(fptr) ) {
        fprintf(stderr, "Error reading header in file %s. %s\n", DB_FILE, strerror(errno));
        fclose(fptr);
        exit(ERR_READ_FILE);
    }
    Header.lastFreq    = freq;   // Save the radio state
    Header.lastStep    = (short) step;
    Header.lastMode    = (char)  mode;
    Header.lastFilter  = (char)  filter;
    Header.lastBfo     = (short) bfo;
    Header.lastVolume  = (char)  vol;
    Header.freqOff     = (char)  freqOff;
    Header.lastPbshift = (short) pbshift;
    memcpy(&Header.devname[0], devname, DEVLEN);

    // Update header
    if( fseek(fptr, 0, SEEK_SET) ) {
        fprintf(stderr, "fseek() failed in file %s. %s\n", DB_FILE, strerror(errno));
        fclose(fptr);
        exit(ERR_SEEK_FILE);
    }
    if( (fwrite(&Header, sizeof(Header), 1, fptr) != 1) || errno ) {
        fprintf(stderr, "Error writing header in file %s. %s\n", DB_FILE, strerror(errno));
        fclose(fptr);
        exit(ERR_READ_FILE);
    }
    fclose(fptr);
    return TRUE;
}

int addEntry(long freq, int step, int mode, int filter, int bfo, int pbshift)
{
    FILE* fptr;
    HEADER Header;
    CHANNEL Channel;
    unsigned int n, empty = FALSE;
    errno = 0;

    if((fptr = fopen(DB_FILE, "r+b")) == NULL) {   // read/write existing file (binary mode)
        fprintf(stderr, "Error opening file %s. %s\n", DB_FILE, strerror(errno));
        exit(ERR_OPEN_FILE);
    }
    // Read header
    if( (fread(&Header, sizeof(Header), 1, fptr) != 1) || errno || feof(fptr) ) {
        fprintf(stderr, "Error reading header in file %s. %s\n", DB_FILE, strerror(errno));
        fclose(fptr);
        exit(ERR_READ_FILE);
    }
    
    for(n = 0; n < Header.num_entries; n++) {
        // Read successive record positions until an empty one is found
        if( (fread(&Channel, sizeof(Channel), 1, fptr) != 1) || errno || feof(fptr) ) {
            fprintf(stderr, "Error reading record %d in file %s. %s\n", n+1, DB_FILE, strerror(errno));
            fclose(fptr);
            exit(ERR_READ_FILE);
        }
        if(Channel.ref == 0) {
            --Header.num_empty;    // We'll be using this empty record
            empty = TRUE;
            break;
        }    
    }    // Or up to the last record
    
    Channel.ref     = n+1;
    Channel.freq    = freq;
    Channel.step    = (short) step;
    Channel.mode    = (char)  mode;
    Channel.filter  = (char)  filter;
    Channel.bfo     = (short) bfo;
    Channel.pbshift = (short) pbshift;

    printf("\nStation/Desc: ");
    fetchLine(Channel.station, DESCLEN);
   	printf("\n");
 
    // Update header
    if( fseek(fptr, 0, SEEK_SET) ) {
        fprintf(stderr, "fseek() failed in file %s. %s\n", DB_FILE, strerror(errno));
        fclose(fptr);
        exit(ERR_SEEK_FILE);
    }
    if(!empty)
        ++Header.num_entries;

    if( (fwrite(&Header, sizeof(Header), 1, fptr) != 1) || errno ) {
        fprintf(stderr, "Error updating header in file %s. %s\n", DB_FILE, strerror(errno));
        fclose(fptr);
        exit(ERR_WRITE_FILE);
    }

    // Add the record
    if( fseek(fptr, sizeof(Channel) * n, SEEK_CUR) ) {
        fprintf(stderr, "fseek() failed in file %s. %s\n", DB_FILE, strerror(errno));
        fclose(fptr);
        exit(ERR_SEEK_FILE);
    }
    if( (fwrite(&Channel, sizeof(Channel), 1, fptr) != 1) || errno ) {
        fprintf(stderr, "Error writing record %d in file %s. %s\n", n+1, DB_FILE, strerror(errno));
        fclose(fptr);
        exit(ERR_WRITE_FILE);
    }

    fclose(fptr);
    listEntries();

    return TRUE;
}

int listEntries(void)
{
    FILE* fptr;
    HEADER Header;
    CHANNEL Channel;
    char mode[4];
    unsigned int n;
    double freq;
    errno = 0;

    switch(MODE) {
        case AM:  strcpy(mode, "am");  break;
        case USB: strcpy(mode, "usb"); break;
        case LSB: strcpy(mode, "lsb"); break;
        case CW:  strcpy(mode, "cw");  break;
    }

    if( (fptr = fopen(DB_FILE, "rb")) == NULL ) {
        fprintf(stderr, "Error opening file %s. %s\n", DB_FILE, strerror(errno));
        exit(ERR_OPEN_FILE);
    }

    // Read the header
    if( (fread(&Header, sizeof(Header), 1, fptr) != 1) || errno || feof(fptr) ) {
        fprintf(stderr, "Error reading header in file %s. %s\n", DB_FILE, strerror(errno));
        fclose(fptr);
        exit(ERR_READ_FILE);
    }
	char dbline[DESCLEN];
	printf("    ");
	printColoredText(" Ref# ", ANSICOL_UNDERLINE | ANSICOL_ITALIC | ANSICOL_YELLOW);
	printf("\t");
	printColoredText(" Freq (KHz)  ", ANSICOL_UNDERLINE | ANSICOL_ITALIC | ANSICOL_YELLOW);
	printf("\t");
	printColoredText(" Station           ", ANSICOL_UNDERLINE | ANSICOL_ITALIC | ANSICOL_YELLOW);

	if( (Header.num_entries - Header.num_empty) == 0 ) {
		printColoredText("\n    List empty\n\n", ANSICOL_YELLOW);
		return 0;
	}

    for(n = 0; n < Header.num_entries; n++) {
        // Read each record entry
        if( (fread(&Channel, sizeof(Channel), 1, fptr) != 1) || errno || feof(fptr) ) {
            fprintf(stderr, "Error reading record %d in file %s. %s\n", n+1, DB_FILE, strerror(errno));
            fclose(fptr);
            exit(ERR_READ_FILE);
        }
        if(Channel.ref > 0) {    
            // Print non-empty records
            freq = (double) (Channel.freq / 1E3);
            sprintf(dbline, "\n     %d\t\t%9.3f %s\t%s", Channel.ref, freq, mode, Channel.station);
			printColoredText(dbline, ANSICOL_GRAY);
        }
    }
    printf("\n\n");
    fclose(fptr);
    return (n);
}

int deleteEntry(void)
{
    FILE* fptr;
    HEADER Header;
    CHANNEL Channel = { 0, 0, 0, 0, 0, 0, 0, "" };
    long n;
    errno = 0;

    if(listEntries() == 0)   // Nothing to delete
        return TRUE;

    if( (fptr = fopen(DB_FILE, "r+b")) == NULL ) {
        fprintf(stderr, "Error opening file %s. %s\n", DB_FILE, strerror(errno));
        exit(ERR_OPEN_FILE);
    }

    // Read header
    if( (fread(&Header, sizeof(Header), 1, fptr) != 1) || errno || feof(fptr) ) {
        fprintf(stderr, "Error reading header in file %s. %s\n", DB_FILE, strerror(errno));
        fclose(fptr);
        exit(ERR_READ_FILE);
    }
    ++Header.num_empty;    

    printf("Ref #: ");
    if( !readLong(&n) ) {
        fclose(fptr);
		printf("Invalid selection\n\n");
        return FALSE;
    } else
		printf("\n");

    if(n < 1 || n > Header.num_entries) {
        printf("Invalid selection.\n\n");
        fclose(fptr);
        return -1;
    }

    // Update header
    if( fseek(fptr, 0, SEEK_SET) ) {
        fprintf(stderr, "fseek() failed in file %s. %s\n", DB_FILE, strerror(errno));
        fclose(fptr);
        exit(ERR_SEEK_FILE);
    }
    if( (fwrite(&Header, sizeof(Header), 1, fptr) != 1) || errno ) {
        fprintf(stderr, "Error updating header in file %s. %s\n", DB_FILE, strerror(errno));
        fclose(fptr);
        exit(ERR_WRITE_FILE);
    }

    // Proceed to the record to delete
    if( fseek(fptr, sizeof(Channel) * (n-1), SEEK_CUR) ) {
        fprintf(stderr, "fseek() failed in file %s. %s\n", DB_FILE, strerror(errno));
        fclose(fptr);
        exit(ERR_SEEK_FILE);
    }

    // Mark record as empty
    if( (fwrite(&Channel, sizeof(Channel), 1, fptr) != 1) || errno ) {    
        fprintf(stderr, "Error deleting record %ld in file %s. %s", n, DB_FILE, strerror(errno));
        fclose(fptr);
        exit(ERR_WRITE_FILE);
    }

    fclose(fptr);
    listEntries();

    return TRUE;
}

int recallEntry(long* freq, int* step, int* mode, int* filter, int* bfo, int* pbshift)
{

    FILE* fptr;
    HEADER Header;
    CHANNEL Channel;
    long n, m;

    errno = 0;

    m = listEntries();

    if( (fptr = fopen(DB_FILE, "rb")) == NULL ) {
        fprintf(stderr, "Error opening file %s. %s\n", DB_FILE, strerror(errno));
        exit(ERR_OPEN_FILE);
    }

    // Read header
    if( (fread(&Header, sizeof(Header), 1, fptr) != 1) || errno || feof(fptr) ) {
        fprintf(stderr, "Error reading header in file %s. %s\n", DB_FILE, strerror(errno));
        fclose(fptr);
        exit(ERR_READ_FILE);
    }

    if( (Header.num_entries - Header.num_empty) == 0 ) {    
        fclose(fptr);
        return 0;
    }

    printf("Ref #: ");
    if( !readLong(&n) ) {
        fclose(fptr);
		printf("Invalid selection.\n\n");
        return FALSE;
    }

    if(n < 1 || n > Header.num_entries) {
        printf("Invalid selection.\n\n");
        fclose(fptr);
        return -1;
    }

    // Proceed to record to read
    if( fseek(fptr, sizeof(Channel) * (n-1), SEEK_CUR) ) {
        fprintf(stderr, "fseek() failed in file %s. %s\n", DB_FILE, strerror(errno));
        fclose(fptr);
        exit(ERR_SEEK_FILE);
    }

    // Recall info
    if( (fread(&Channel, sizeof(Channel), 1, fptr) != 1) || errno || feof(fptr) ) {
        fprintf(stderr, "Error reading record %ld in file %s. %s\n", n, DB_FILE, strerror(errno));
        fclose(fptr);
        exit(ERR_READ_FILE);
    }
    fclose(fptr);   

    if(n < 1 || n > m || Channel.ref == 0) {
        printf("Invalid selection.\n\n");
        return -1;
    }

    *bfo     = Channel.bfo;
    *freq    = Channel.freq;
    *mode    = Channel.mode;
    *filter  = Channel.filter;
    *step    = Channel.step;
    *pbshift = Channel.pbshift;

    printf("\n");
    return TRUE;
}
