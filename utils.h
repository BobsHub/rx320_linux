#ifndef _UTILS_H
#define _UTILS_H

#define BUFFLEN  40
#define FALSE    0
#define TRUE     1

enum { 
  ANSICOL_OFF        = 0x1, 
  ANSICOL_BOLD       = 0x2,
  ANSICOL_GRAY       = 0x4, 
  ANSICOL_ITALIC     = 0x8,  
  ANSICOL_UNDERLINE  = 0x10,
  ANSICOL_INVERSE    = 0x20,
  ANSICOL_STRIKEOUT  = 0x40,
  ANSICOL_BLACK      = 0x80, 
  ANSICOL_RED        = 0x100,
  ANSICOL_GREEN      = 0x200,
  ANSICOL_YELLOW     = 0x400, 
  ANSICOL_BLUE       = 0x800,
  ANSICOL_MAGENTA    = 0x1000,
  ANSICOL_CYAN       = 0x2000,
  ANSICOL_WHITE      = 0x4000,
  ANSICOL_DEFAULT    = 0x8000,
  ANSICOL_BGBLACK    = 0x10000,
  ANSICOL_BGRED      = 0x20000,
  ANSICOL_BGGREEN    = 0x40000,
  ANSICOL_BGYELLOW   = 0x80000,
  ANSICOL_BGBLUE     = 0x100000,
  ANSICOL_BGMAGENTA  = 0x200000,
  ANSICOL_BGCYAN     = 0x400000,
  ANSICOL_BGWHITE    = 0x800000,
  ANSICOL_BGDEFAULT  = 0x1000000
};

char*  fetchLine(char* line, size_t len);
int    readDouble(double* d);
int    readLong(long* n);
int    findData(char* buff, char* key, size_t maxlen);
double round(double number);
char   chupper(char chr);
char   chlower(char chr);
int    getch(void);
int    getche(void);
int    kbhit(void);
void   printColoredText(char* text, unsigned style);

#endif
