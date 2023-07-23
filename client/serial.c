// https://blog.mbedded.ninja/programming/operating-systems/linux/linux-serial-ports-using-c-cpp/

#include <stdio.h>
#include <errno.h>
#include <fcntl.h> 
#include <string.h>
#include <termios.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include <stdbool.h>
#include <signal.h>

#define RESPONSECODE_OK ';'

int set_interface_attribs (int fd, int speed, int parity) {
    struct termios tty = {0};
    if (tcgetattr (fd, &tty) != 0) // this is required by PosiX
    {
        fprintf(stderr, "error %d from tcgetattr\n", errno);
        return -1;
    }

    cfsetospeed (&tty, speed);
    cfsetispeed (&tty, speed);


    // control mode flags
    tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;   // 8-bit chars
    tty.c_cflag &= ~(PARENB | PARODD);   // shut off parity
    tty.c_cflag &= ~CSTOPB; // one stop bit
    tty.c_cflag &= ~CRTSCTS; // disable rts/cts hardware flow control
    tty.c_cflag |= (CLOCAL | CREAD);// ignore modem controls, enable reading

    // input mode flags
    tty.c_iflag &= ~IGNBRK;     // disable break processing
    tty.c_iflag &= ~(IXON | IXOFF | IXANY); // shut off xon/xoff ctrl
    tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes
    //if(omit_0xd) tty.c_iflag |= IGNCR;

    // output mode flags
    tty.c_oflag = 0;        // no remapping, no delays
    tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
    tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

    // control characters
    tty.c_cc[VMIN] = 0;      // read doesn't block
    tty.c_cc[VTIME] = 5;      // 0.5 seconds read timeout

    // local mode flags
    //tty.c_lflag = 0;        // no signaling chars, no echo,
    tty.c_lflag &= ~ECHO; // Disable echo
    tty.c_lflag &= ~ECHOE; // Disable erasure
    tty.c_lflag &= ~ECHONL; // Disable new-line echo
    tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
    tty.c_lflag &= ~ICANON;

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
    {
        fprintf(stderr, "error %d from tcsetattr\n", errno);
        return -1;
    }
    return 0;
}

void set_blocking (int fd, int should_block){
    struct termios tty;
    memset (&tty, 0, sizeof tty);
    if (tcgetattr (fd, &tty) != 0)
    {
        fprintf(stderr, "error %d from tggetattr\n", errno);
        return;
    }

    tty.c_cc[VMIN] = should_block ? 1 : 0;
    tty.c_cc[VTIME] = should_block ? 5 : 0;      // 0.5 seconds read timeout

    if (tcsetattr (fd, TCSANOW, &tty) != 0)
      fprintf(stderr, "error %d setting term attributes\n", errno);
}

