/* UDP echo server program -- echo-server-udp.c */


#include <stdio.h>      /* standard C i/o facilities */
#include <stdlib.h>     /* needed for atoi() */
#include <string.h>     /* needed for strcpy */
#include <unistd.h>     /* defines STDIN_FILENO, system calls,etc */
#include <fcntl.h>      /* defines O_WRONLY */
#include <sys/types.h>  /* system data type definitions */
#include <sys/socket.h> /* socket specific definitions */
#include <netinet/in.h> /* INET constants and stuff */
#include <arpa/inet.h>  /* IP address conversion stuff */
#include <netdb.h>      /* gethostbyname */


void led(int channel,int density) {
  int fd;
  char file[100];
  char str_density[10];
  
  printf("file: /sys/class/pwm/gpio_pwm:%d/duty_ns\n", channel); 
  printf("density: %d\n", density);  

  sprintf(file, "/sys/class/pwm/gpio_pwm:%d/duty_ns", channel); 
  sprintf(str_density, "%d0000", density);  

  fd = open(file, O_WRONLY);
  write (fd, str_density, strlen(str_density));
  close(fd);
}

int get_led_duty(int channel) {
  int fd;
  int density;
  char file[100];
  char str_density[10];
  
  printf("file: /sys/class/pwm/gpio_pwm:%d/duty_ns\n", channel); 

  sprintf(file, "/sys/class/pwm/gpio_pwm:%d/duty_ns", channel); 

  fd = open(file, O_RDONLY);
  read (fd, str_density, 10);
  close(fd);

  sscanf( str_density, "%3d0000", &density);
  printf("density: %d\n", density);  

  return density;
}

/* this routine echos any messages (UDP datagrams) received */
#define MAXBUF 1024*1024
void echo( int sd ) {
    int len,n;
    char bufin[MAXBUF];
    char cmd;
    char arg[20];
    char str_val[10];
    struct sockaddr_in remote;
    int chan,rchan,gchan,bchan;
    int val,rval,gval,bval;

    /* need to know how big address struct is, len must be set before the
       call to recvfrom!!! */

    len = sizeof(remote);

    while (1) {
      /* read a datagram from the socket (put result in bufin) */
      n=recvfrom(sd,bufin,MAXBUF,0,(struct sockaddr *)&remote,&len);

      /* print out the address of the sender */
      printf("Got a datagram from %s port %d\n",
             inet_ntoa(remote.sin_addr), ntohs(remote.sin_port));

      if (n<0) {
        perror("Error receiving data");
      } else {
        printf("GOT %d BYTES\n",n);
 	
	strcpy(arg, "");

	sscanf( bufin, "%c%s",&cmd, &arg );
	printf("cmd:%c-arg:%s\n", cmd,arg);

	if(cmd == 'X') {
		sscanf( arg, "%3d%3d%3d%3d%3d%3d",&rchan,&gchan,&bchan,&rval,&gval,&bval);
		printf("rchan: %d rval: %d\n", rchan, rval);
		printf("gchan: %d gval: %d\n", gchan, gval);
		printf("bchan: %d bval: %d\n", bchan, bval);
		led(rchan,rval);
		led(gchan,gval);
		led(bchan,bval);
	}
	else if(cmd == 'S') {
		sscanf( arg, "%3d%3d",&chan,&val);
		printf("chan: %d val: %d\n", chan, val);
		led(chan,val);
	}	
	else if(cmd == 'R') {
		sscanf( arg, "%3d",&chan);
		printf("read chan: %03d\n", chan);
		val = get_led_duty(chan);
		sprintf(str_val, "%03d\n", val);
                sendto(sd,str_val,4,0,(struct sockaddr *)&remote,len);
	}
	else if(cmd == 'Y') {
		sscanf( arg, "%3d%3d%3d",&rchan,&gchan,&bchan);
		printf("read rchan: %03d\n", rchan);
		printf("read gchan: %03d\n", gchan);
		printf("read bchan: %03d\n", bchan);
		rval = get_led_duty(rchan);
		gval = get_led_duty(gchan);
		bval = get_led_duty(bchan);
		printf("read rval: %03d\n", rval);
		printf("read gval: %03d\n", gval);
		printf("read bval: %03d\n", bval);
		printf("%03d%03d%03d\n", rval, gval, bval);
		sprintf(str_val, "%03d%03d%03d\n", rval, gval, bval);
                sendto(sd,str_val,10,0,(struct sockaddr *)&remote,len);
	}

        /* Got something, just send it back */
        //sendto(sd,bufin,n,0,(struct sockaddr *)&remote,len);
      }
    }
}

/* server main routine */

int main() {
  int ld;
  struct sockaddr_in skaddr;
  int length;

  /* create a socket
     IP protocol family (PF_INET)
     UDP protocol (SOCK_DGRAM)
  */

  if ((ld = socket( PF_INET, SOCK_DGRAM, 0 )) < 0) {
    printf("Problem creating socket\n");
    exit(1);
  }

  /* establish our address
     address family is AF_INET
     our IP address is INADDR_ANY (any of our IP addresses)
     the port number is assigned by the kernel
  */

  skaddr.sin_family = AF_INET;
  skaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  skaddr.sin_port = htons(10000);

  if (bind(ld, (struct sockaddr *) &skaddr, sizeof(skaddr))<0) {
    printf("Problem binding\n");
    exit(0);
  }

  /* find out what port we were assigned and print it out */

  length = sizeof( skaddr );
  if (getsockname(ld, (struct sockaddr *) &skaddr, &length)<0) {
    printf("Error getsockname\n");
    exit(1);
  }

  /* port number's are network byte order, we have to convert to
     host byte order before printing !
  */
  printf("The server UDP port number is %d\n",ntohs(skaddr.sin_port));

  /* Go echo every datagram we get */
  echo(ld);
  return(0);
}

