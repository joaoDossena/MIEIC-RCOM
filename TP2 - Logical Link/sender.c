/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

#define BUF_SIZE 255

//Flags
#define FLAG 0x7E
#define A 0x03
#define C_SET 0x03
#define C_UA 0x07

volatile int STOP=FALSE;

unsigned char SET[5];
unsigned char UA[5];

int main(int argc, char** argv)
{
    int fd,c;
    struct termios oldtio,newtio;
    int i, sum = 0, speed = 0;
    char buf[BUF_SIZE];
    
    if ( (argc < 2) || 
  	     (
          (strcmp("/dev/ttyS10", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS11", argv[1])!=0) &&
          (strcmp("/dev/ttyS0", argv[1])!=0) && 
          (strcmp("/dev/ttyS1", argv[1])!=0)
         ) ) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */


    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 char received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");

    char send[BUF_SIZE];
    fgets(send, BUF_SIZE, stdin);

    int res;
    res = write(fd, send, strlen(send)+1);   
    printf("%d bytes written\n", res);

    for(int i = 0; i < res; i++)
    {
      printf("%d: %c\n", send[i], send[i]); 
    }

    char receive[BUF_SIZE];
    strcpy(receive, "");

    while (STOP==FALSE) {       /* loop for input */
      res = read(fd,buf,1);   /* returns after 5 chars have been input */
      buf[res]=0;               /* so we can printf... */
      //printf(":%s:%d\n", buf, res);
      if (buf[0]=='\n') 
      {
        STOP=TRUE;
        printf("\n");
      }
      receive[strlen(receive)] = buf[0];
    }

    printf("sent: %s", send);
    printf("received: %s\n", receive);

    if(strcmp(send, receive) != 0)
    {
      perror("Message corrupted!\n");
      exit(-1);
    }
 
    printf("Sucess!\n");

   
    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }




    close(fd);
    return 0;
}
