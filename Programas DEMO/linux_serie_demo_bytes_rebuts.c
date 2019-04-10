//      linux_serie_demo_bytes_rebuts.c
//
//This document is copyrighted (c) 1997 Peter Baumann, (c) 2001 Gary Frerking
//and is distributed under the terms of the Linux Documentation Project (LDP)
//license, stated below.
//
//Unless otherwise stated, Linux HOWTO documents are copyrighted by their
//respective authors. Linux HOWTO documents may be reproduced and distributed
//in whole or in part, in any medium physical or electronic, as long as this
//copyright notice is retained on all copies. Commercial redistribution is
//allowed and encouraged; however, the author would like to be notified of any
//such distributions.
//
//All translations, derivative works, or aggregate works incorporating any
//Linux HOWTO documents must be covered under this copyright notice. That is,
//you may not produce a derivative work from a HOWTO and impose additional
//restrictions on its distribution. Exceptions to these rules may be granted
//under certain conditions; please contact the Linux HOWTO coordinator at the
//address given below.
//
//In short, we wish to promote dissemination of this information through as
//many channels as possible. However, we do wish to retain copyright on the
//HOWTO documents, and would like to be notified of any plans to redistribute
//the HOWTOs.
//
//http://www.ibiblio.org/pub/Linux/docs/HOWTO/Serial-Programming-HOWTO

#include <stdlib.h>
#include <sys/types.h>                                                    
#include <sys/stat.h>                                                     
#include <fcntl.h>                                                        
#include <termios.h>                                                      
#include <stdio.h>                                                        
#include <strings.h>
#include <unistd.h>
#include <sys/ioctl.h>     
 
#define BAUDRATE B9600                                                
#define MODEMDEVICE "/dev/ttyS0"        //Conexió IGEP - Arduino
//#define MODEMDEVICE "/dev/ttyUSB0"        //Conexió directa PC(Linux) - Arduino                                   
#define _POSIX_SOURCE 1 /* POSIX compliant source */                       
                                                           
struct termios oldtio,newtio;                                            


int	ConfigurarSerie(void)
{
	int fd;                                                           


	fd = open(MODEMDEVICE, O_RDWR | O_NOCTTY );                             
	if (fd <0) {perror(MODEMDEVICE); exit(-1); }                            

	tcgetattr(fd,&oldtio); /* save current port settings */                 

	bzero(&newtio, sizeof(newtio));                                         
	//newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;             
	newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;             
	newtio.c_iflag = IGNPAR;                                                
	newtio.c_oflag = 0;                                                     

	/* set input mode (non-canonical, no echo,...) */                       
	newtio.c_lflag = 0;                                                     

	newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */         
	newtio.c_cc[VMIN]     = 1;   /* blocking read until 1 chars received */ 

	tcflush(fd, TCIFLUSH);                                                  
	tcsetattr(fd,TCSANOW,&newtio);
	
	return fd;
}               

void TancarSerie(fd)
{
	tcsetattr(fd,TCSANOW,&oldtio);
	close(fd);
}
                                                                                 
int main(int argc, char **argv)                                                               
{                                                                          
	int fd, i = 0, res, res1, res2;                                                           
	char buf[255];
	char missatge[255];
	union{
		long int i;
		char	c[4];
	}conversio;
	int bytes;

	fd = ConfigurarSerie();

	// Enviar el missatge 1
	missatge[0] = 0xAA;
	missatge[1] = 0x01;
	conversio.i = 0;
	missatge[2] = conversio.c[0];
	missatge[3] = conversio.c[1];
	missatge[4] = conversio.c[2];
	missatge[5] = conversio.c[3];

	res = write(fd,missatge,6);

	if (res <0) {tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1); }

	printf("Enviats %d bytes: ",res);
	for (i = 0; i < res; i++)
	{
		printf("0x%x ",missatge[i]);
	}
	printf("\n");

	//Abans de rebre:
	//Esperem un segon
	sleep(1);
	
	//Mirem quans bytes ens ha enviat l'Arduino
  	ioctl(fd, FIONREAD, &bytes);
  	printf("Anem a rebre %d byte(s)\n",bytes);

	//Rebem
	res1 = read(fd,buf,1);
	res2 = read(fd,buf+1,1);
	res = res1 + res2;

	printf("Rebuts %d bytes: ",res);
	for (i = 0; i < res; i++)
	{
		printf("0x%x ",buf[i]);
	}
	printf("\n");
                                                                   
	TancarSerie(fd);
	
	return 0;
}


