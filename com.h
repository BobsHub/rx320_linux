#ifndef _COM_H
#define _COM_H
#define RXBUF     512 		// Receive buffer limit
#define TXBUF      50 		// Transmit buffer limit
char resp[RXBUF];		// Receive Buffer
char cmmd[TXBUF];		// Holds the send command
int _fd;                        // File descriptor

int set_interface_attribs(int fd, int speed, int parity);
void set_blocking(int fd, int should_block);
int OpenPort(char* port);
int WritePort(char* out, unsigned len);
int ReadPort(char* in);
void ResponseWait();			
void ClosePort();

#endif
