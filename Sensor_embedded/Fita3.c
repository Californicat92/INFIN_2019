//      Fita3.c
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

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>                                                        
#include <termios.h>       
#include <sys/ioctl.h>    
 
#define BAUDRATE B9600
//#define MODEMDEVICE "/dev/ttyS0"		//Conexió IGEP - Arduino
#define MODEMDEVICE "/dev/ttyACM0"		//Conexió directa PC(Linux) - Arduino
#define _POSIX_SOURCE 1					//POSIX compliant source
#define MIDA 100
struct termios oldtio,newtio;
void Enviar(int fd,char *missatge);
void Rebre(int fd,char *buf);
int	ConfigurarSerie(void);
void TancarSerie(int fd);

char buf[100];

int main(int argc, char **argv)
{
	int fd,v=10,t=25;
	
	char missatge[100];
	printf("Espera a que el sistema inicie\n");
	
	fd = ConfigurarSerie();
	memset(missatge,'\0', MIDA);
	memset(buf,'\0', MIDA);
	// Enviar el missatge 1
	while (v !=0 || v !=1) //protección valores erroneos
	{
		printf("Posar en marxa [1] o parar [0]:");
		scanf("%i", &v);
		while (v != 1 && v!=0) //protección valores erroneos
		{
			printf("Posar en marxa [1] o parar [0]:");
			scanf("%i", &v);
		}
		if (v==1){ //si se pone en marcha realizamos acciones
			printf("Es posa en marxa l'adquisicio.\n");
			while (t <01 || t>20) //protección valores erroneos
			{
				printf("Temps de mostreig desitjat(1-20):");
				scanf("%i", &t); //guardamos el tiempo en una variable de tipo entero
			}
			t=t/2; //Guardamos el tiempo en ms pero a mitad de tiempo (enunciado)
			sprintf(missatge,"AM%i%.2iZ",v,t); //cargem a la variable a enviar les dades
			break;
		}
		else if (v==0)//si se para finalizamos
		{ 
			printf("Adquisicio aturada.\n");
			sprintf(missatge,"AM000Z"); //cargem a la variable a enviar les dades
		}
	}
	Enviar(fd,missatge);
	
printf("----\n");

sleep(3);	
	
	Rebre(fd,buf);
	/*
	int j=0;
	while(1)
	{
		printf("captura muestra...%i",j);
		memset(missatge,'\0', MIDA);
		sprintf(missatge,"AS131Z"); //Encenem LED 13
		Enviar(fd,missatge);
		Rebre(fd,buf);
		sleep(t);
		memset(missatge,'\0', MIDA);
		sprintf(missatge,"AS130Z"); //Apaguem LED 13
		Enviar(fd,missatge);
		Rebre(fd,buf);
		sleep(t);
		j++;
	};*/

	return 0;
}

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

	sleep(5); //Per donar temps a que l'Arduino es recuperi del RESET
	return fd;
}

void TancarSerie(int fd)
{
	tcsetattr(fd,TCSANOW,&oldtio);
	close(fd);
}

void Enviar(int fd,char *missatge)
{
	int i=0;
	int res=0;
	
	res = write(fd,missatge,strlen(missatge));
	
	if (res <0) {tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1); }
	
	printf("Enviats %d bytes: ",res);
	for (i = 0; i <= res; i++)
	{
		printf("%c",missatge[i]);
	}
	printf("\n");
}
void Rebre(int fd,char *buf)
{
	int k = 1;
	int res = 0;
	int bytes = 0;
	
	ioctl(fd, FIONREAD, &bytes);
	
	printf("bytes = %d\n",bytes);	
	res = read(fd,buf,1);
	printf("read = %c\n",buf[0]);
	do
	{
		res = res + read(fd,buf+k,1);
		printf("read = %c\n",buf[k]);
		k++;
	}
	while (buf[k-1] != 'Z');
	printf("Rebuts %d bytes: ",res);

	
	printf("%s\n",buf);
	TancarSerie(fd);
}

