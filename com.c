#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <termio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "consts.h"
#include "com.h"

int set_interface_attribs (int fd, int speed, int parity)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
		perror("Error from tcgetattr");
                exit(1);
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     // 8-bit chars
        // disable IGNBRK for mismatched speed tests; otherwise receive break
        // as \000 chars
        tty.c_iflag &= ~IGNBRK;         // disable break processing
        tty.c_lflag = 0;                // no signaling chars, no echo,
                                        // no canonical processing
        tty.c_oflag = 0;                // no remapping, no delays
        tty.c_cc[VMIN]  = 0;            // read doesn't block
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl

        tty.c_cflag |= (CLOCAL | CREAD);		// ignore modem controls,
                                        		// enable reading
        tty.c_cflag &= ~(PARENB | PARODD);      // shut off parity
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
		perror("Error from tcsetattr");
                exit(1);
        }
        return 0;
}

void set_blocking (int fd, int should_block)
{
        struct termios tty;
        memset (&tty, 0, sizeof tty);
        if (tcgetattr (fd, &tty) != 0)
        {
		perror("Error from tggetattr");
		exit(1);
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            // 0.5 seconds read timeout

        if (tcsetattr (fd, TCSANOW, &tty) != 0) {
		perror("Error setting term attributes");
		exit(1);
	}
}

int OpenPort(char* port)
{
	_fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
	if(_fd < 0) {
		perror("Error opening port");
		if(errno == EACCES) {
			printf("Perhaps your not a member of the dialout group! Try:\n");
			printf("sudo gpasswd -a {USER} dialout\n");
			printf("Log out then back in and re-launch the program.\n");
		}
		return FALSE;
	}

	set_interface_attribs(_fd, B1200, 0);	// 1200baud 8n1 (no parity)
	set_blocking(_fd, 0);
	return TRUE;
}

int WritePort(char* out, unsigned len)
{
	return write(_fd, out, len);
}

int ReadPort(char* in)
{
	return read(_fd, in, RXBUF);
}

void ResponseWait()
{
	usleep(200000);	
}


void ClosePort()
{
	close(_fd);
}


