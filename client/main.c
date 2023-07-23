#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <unistd.h>
 
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
#define RESPONSECODE_UNKNOWN_TYPE '?'
#define RESPONSECODE_CHECKSUM_WRONG '!'
#define RESPONSECODE_TIMEOUT '*'
#define RESPONSECODE_PROGRAMMING_FAILED '.'

// TODO: make sure that the reader thread terminates properly

char *portname = "/dev/ttyUSB0";
int baud_rate = B115200;

FILE *instream;
int fd; // file descriptor of USB


void printHelpMessage();


extern int set_interface_attribs (int fd, int speed, int parity);
extern void set_blocking (int fd, int should_block);

void handle_response_code(uint8_t resp_code, int linecnt);


void error_exit(char *msg1, char *msg2){
  //raise(SIGUSR1); // allow reader thread to interrupt
  fprintf(stderr, "[error] %s %s\n", msg1, msg2);
  exit(EXIT_FAILURE);
}

int get_baudrate(char *str){
  if(strcmp("115200", str) == 0)
    return B115200;
  if(strcmp("9600", str) == 0)
    return B9600;
  error_exit("Baud rate not supported", "");
  return 0;
}
void read_fd(int fd, char *buf, size_t len){
  int total_read = 0;
  while(total_read < len){
    int n = read(fd, buf, len-total_read);
    if(n <= 0){
      if(errno == EINTR){
        printf("interrupted\n");
        continue;
      }
      error_exit("read interrupted", strerror(errno)); 
    } else {
      total_read += n;
    }
  }

}
void write_fd(int fd, char* buf, size_t l){
  int written = 0;
  while(written < l){
    int n = write(fd, buf + written, l - written);
    if(n <= 0){
      if(errno == EINTR){
        printf("interrupted\n");
        continue;
      }
      error_exit("write interrupted", strerror(errno)); 
    } else {
      written += n;
    }
  }
}
void printHelpMessage(){
  printf("Tool to flash the fpga");
  printf("usage: -p port -b baudrate\n");
}


int main(int argc, char **argv){
 // send hex file over uart to the uC
 // ---------------------------------------
  portname = NULL;
  char *infile = NULL; // file which stores the fpga bistream
  int opt;
  while((opt = getopt(argc, argv, "p:b:f:")) != -1){
    switch(opt){
      case 'p':
        portname = optarg;
      break;
      case 'f':
        infile = optarg;
      break;
      case 'b':
        baud_rate = get_baudrate(optarg);
      break;
      case 'h':
        printHelpMessage();
        exit(0);
      break;
      default:
        printHelpMessage();
        error_exit("invalid option", "");
    }
  }

  if(portname == NULL)
    error_exit("-p portname required", "");

  FILE *f = fopen(infile, "rb");
  
  if(f == NULL){
    error_exit("could not open file", strerror(errno));
  }

  

  fd = open (portname, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0) {
    error_exit("error opening serial port:", strerror (errno));
    return 1;
  }
  set_interface_attribs (fd, baud_rate, 0); // set speed to 115,200 bps, 8n1 (no parity)
  set_blocking (fd, 1);

  size_t nread, len = 0;
  char *line;
  uint32_t linecnt = 0;
//  uint8_t readbuf[2];
//  read_fd(fd, readbuf, 2);
//  if(memcmp(readbuf, "hi", 2) != 0){
//    printf("controller did not say hi\n");
//    printf("%c%c\n", readbuf[0], readbuf[1]);
//    exit(EXIT_FAILURE);
//  }

  printf("programming\n");
  while ((nread = getline(&line, &len, f)) != -1) {
    //printf("Retrieved line of length %zd:\n", nread);
    // convert line to bytes:
    char *data = line + 1;
    int l = (nread - 2) / 2; // remove : and newline
    uint8_t numbuf[l];
    for(int i = 0; i < l; i++){
      char hexstring[3] = {0, 0, 0};
      memcpy(hexstring, data + 2 * i, 2);
      numbuf[i] = (int)strtol(hexstring, NULL, 16);
    }
    linecnt ++;
    write_fd(fd, numbuf, l);
    
    read_fd(fd, numbuf, 1);
    handle_response_code(numbuf[0], linecnt);
 
  }
  // Back to the beginning of the file and verify contents
  fseek(f, 0, SEEK_SET);
  linecnt = 0;
  printf("\nverifying\n");
  while ((nread = getline(&line, &len, f)) != -1) {
    char *data = line + 1;
    int l = (nread - 2) / 2; // remove : and newline
    uint8_t numbuf[l];
    for(int i = 0; i < l; i++){
      char hexstring[3] = {0, 0, 0};
      memcpy(hexstring, data + 2 * i, 2);
      numbuf[i] = (int)strtol(hexstring, NULL, 16);
    }
    linecnt ++;
    //write_fd(fd, numbuf, l);
    //printf("line: %s", line);
    if(numbuf[3] == 0){ // this is a data block
      // compose a new message
      uint8_t data_sum = 0;
      for(int i = 0; i < numbuf[0]; i++) data_sum += numbuf[4 + i];
      uint8_t old_len = numbuf[0];
      numbuf[0] = 2; // new length;
      //numbuf[1]
      //numbuf[2]
      numbuf[3] = 5;
      numbuf[4] = old_len;
      numbuf[5] = data_sum;
      uint8_t checksum = 0;
      for(int i = 0; i < 6; i++) checksum += numbuf[i];
      numbuf[6] = -checksum;
      // for(int i = 0; i < 7; i++) printf("%02x", numbuf[i]);
      // printf("\n");
      l = 7; // set length to 6
    }
    write_fd(fd, numbuf, l);
    
    read_fd(fd, numbuf, 1);
    handle_response_code(numbuf[0], linecnt);

  }


  printf("\nDone: Bye\n");
  free(line);
}
void handle_response_code(uint8_t resp_code, int linecnt){
   switch(resp_code){
      case RESPONSECODE_CHECKSUM_WRONG:
        printf("\nwrong checksum: %d\n", resp_code);
        exit(EXIT_FAILURE);
        break;
      case RESPONSECODE_OK:
        printf(".");
        fflush(stdout);
        break;
      case RESPONSECODE_UNKNOWN_TYPE:
        printf("type not supported by uC\n");
        exit(EXIT_FAILURE);
        break;
      case RESPONSECODE_TIMEOUT:
        printf("Timeout data\n");
        exit(EXIT_FAILURE);
        break;
      case RESPONSECODE_PROGRAMMING_FAILED:
        printf("Programming verification failed\n");
        exit(EXIT_FAILURE);
        break;
      default:
        printf("unknown response code: %x\n",resp_code); 
        exit(EXIT_FAILURE);
    }
}