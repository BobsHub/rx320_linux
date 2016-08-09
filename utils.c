#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include "utils.h"

// Reads 1 line of string input from the keyboard
char* fetchLine(char* line, size_t len)
{
    char* p;
    p = fgets(line, len, stdin);
    if(p) {
        size_t last = strlen(line) - 1;
        if(line[last] == '\n') 
            line[last] = '\0';
        else
            while( getchar() != '\n' );
    }
    return p;
}

// Obtain a double from the keyboard
int readDouble(double* d)
{
    char dblbuff[BUFFLEN];
    char* end;

    if( !fetchLine(dblbuff, BUFFLEN) ) 
        return FALSE;
    
    errno = 0;
    *d = strtod(dblbuff, &end);
    if( errno || (strlen(dblbuff) == 0) || *end )
        return FALSE;

    return TRUE;
}

// Obtain a long int from the keyboard
int readLong(long* n)
{
    char longbuff[BUFFLEN];
    char* end;

    if( !fetchLine(longbuff, BUFFLEN) )
        return FALSE;

    errno = 0;
    *n = strtol(longbuff, &end, 10);
    if( errno || (strlen(longbuff) == 0) || *end )
        return FALSE;

    return TRUE;
}

// Searches buffer for a response string
int findData(char* buff, char* key, size_t maxlen)
{
    size_t i, j, cmp;
    size_t keylen = strlen(key);

    for(i = 0; i < (maxlen - keylen); i++) {
        for(j = 0, cmp = 1; j < keylen; j++) {
            if( buff[i+j] != key[j] ) {
                cmp = 0; break;
            }
        }
        if(cmp) break;
    }
    return (cmp ? i : -1);
}

double round(double number)
{
    return number < 0.0 ? ceil(number - 0.5) : floor(number + 0.5);
}

char chupper(char chr)
{
    return (chr >= 'a' && chr <= 'z') ? chr - 32 : chr;
}

char chlower(char chr)
{
    return (chr >= 'A' && chr <= 'Z') ? chr + 32 : chr;
}

/* reads from keypress, doesn't echo */
int getch(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}

/* reads from keypress, echoes */
int getche(void)
{
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}

int kbhit(void)
{
  struct termios oldt, newt;
  int ch;
  int oldf;
 
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  oldf = fcntl(STDIN_FILENO, F_GETFL, 0);          //
  fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK); //
 
  ch = getchar();

  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
  fcntl(STDIN_FILENO, F_SETFL, oldf);              //

  if(ch == EOF)
	return FALSE;	
 
  ungetc(ch, stdin);
  return TRUE;
}

void printColoredText(char* text, unsigned style)
{
	int colorTable[] = { 0, 1, 2, 3, 4, 7, 9, 
		30, 31, 32, 33, 34, 35, 36, 37, 39,
        40, 41, 42, 43, 44, 45, 46, 47, 49 };

	for(int i = 0; i < (sizeof(colorTable) / sizeof(colorTable[0])); i++) {
		if(style & 1) 
			printf("\x1B[%dm", colorTable[i]);
		style >>= 1;
	}
	
	printf("%s", text);
	printf("\x1B[0m");
}

