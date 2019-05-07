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
#define	MIDA_BUFFER 3600
struct termios oldtio,newtio;
void Enviar(int fd,char *missatge);
void Rebre(int fd,char *buf);
int	ConfigurarSerie(void);
void TancarSerie(int fd);






typedef struct _TipusMostra{
	long int pos;
	char	temperatura[5];
}TipusMostra;

struct{
	TipusMostra *dades;
	int	index_entrada; //Apunta al lloc on es posarà la sagüent mostra
	int nombre_mostres; //Nombre de mostres que hi ha el el buffer circular
}buffer_circular;

void	buffer_cicular_inici(void){
	buffer_circular.dades = malloc(sizeof(TipusMostra)*MIDA_BUFFER);
	buffer_circular.index_entrada = 0;
	buffer_circular.nombre_mostres = 0;
}

void	buffer_cicular_final(void){
	free(buffer_circular.dades);
}

void	buffer_cicular_introduir(TipusMostra dada){
	buffer_circular.dades[buffer_circular.index_entrada] = dada;

	buffer_circular.index_entrada++;
	if (buffer_circular.index_entrada == MIDA_BUFFER){
		buffer_circular.index_entrada = 0; //Continuem pel principi: circular
	}

	if (buffer_circular.nombre_mostres < MIDA_BUFFER){ //Agumentar fins que estigui ple
		buffer_circular.nombre_mostres++;
	}

}

void	buffer_cicular_borrar_tot(void){
	buffer_circular.index_entrada = 0;
	buffer_circular.nombre_mostres = 0;
}

void	buffer_cicular_bolcat_dades(void){
	int i;
	TipusMostra dada;

	if (buffer_circular.nombre_mostres < MIDA_BUFFER){

		for (i=0;i<buffer_circular.nombre_mostres;i++){
			dada = buffer_circular.dades[i];
			printf("Temps: %ld Temperatura: %s\n", dada.pos, dada.temperatura);
		}
	}
	else{
		for (i=buffer_circular.index_entrada;i<MIDA_BUFFER;i++){
			dada = buffer_circular.dades[i];
			printf("Temps: %ld Temperatura: %s\n", dada.pos, dada.temperatura);
		}
		for (i=0;i<buffer_circular.index_entrada;i++){
			dada = buffer_circular.dades[i];
			printf("Temps: %ld Temperatura: %s\n", dada.pos, dada.temperatura);
		}
	}
}






int t;

int main(int argc, char **argv)
{
	int fd,v=10;
	char buf[100];
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
			t=25; //modificació del temps per a complir amb la protecció
			while (t <01 || t>20) //protección valores erroneos
			{
				printf("Temps de mostreig desitjat(1-20):");
				scanf("%i", &t); //guardamos el tiempo en una variable de tipo entero
			}
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
	sleep(1);
	memset(buf,'\0', MIDA);
	Rebre(fd,buf);
	
	
	int j=0;
	int q=0;
	TipusMostra TempSensor;
	while(1)
	{
		printf("captura muestra...%i\n",j);
		memset(missatge,'\0', MIDA);
		sprintf(missatge,"ACZ"); //Encenem LED 13
		Enviar(fd,missatge);
		sleep(t);
		Rebre(fd,buf);
		buffer_cicular_inici();
		TempSensor.pos = j;
		printf("Dada guardada: %c%c%c%c%c\n",buf[3],buf[4],buf[5],buf[6],buf[7]);
		for (q = 3; q < 8; q++) TempSensor.temperatura[q-2] = buf[q];
		buffer_cicular_introduir(TempSensor);
		//buffer_cicular_bolcat_dades();	//Fem un bocat del contingut del buffer circular
		printf("---------------------\n");
		j++;
	};
	TancarSerie(fd);
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

	sleep(2); //Per donar temps a que l'Arduino es recuperi del RESET
	return fd;
}

void TancarSerie(int fd)
{
	tcsetattr(fd,TCSANOW,&oldtio);
	close(fd);
}

void Enviar(int fd,char *missatge)
{
	int res=0;
	
	res = write(fd,missatge,strlen(missatge));
	
	if (res <0) {tcsetattr(fd,TCSANOW,&oldtio); perror(MODEMDEVICE); exit(-1); }
	
	printf("Enviats %d bytes: %s\n",res,missatge);
}
void Rebre(int fd,char *buf)
{
	int k = 0;
	int res = 0;
	int bytes = 0;
	
	ioctl(fd, FIONREAD, &bytes);

	do
	{
		res = res + read(fd,buf+k,1);
		k++;
	}
	while (buf[k-1] != 'Z');	
	printf("Rebuts %d bytes: %s\n",res,buf);
}

