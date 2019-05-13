/*      Fita3.c
Para compilar: gcc Fita3.c -o Fita3
Para ejecutar: ./Fita3
*/

//-------------------------------------LIBRERIAS---------------------------------------
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
//-------------------------------------DEFINICION DE CONSTANTES PARA EL PROGRAMA---------------------------------------
#define BAUDRATE B9600				//Velocitat port serie (BAUDRATE)
//#define MODEMDEVICE "/dev/ttyS0"	//Conexió IGEP - Arduino
#define MODEMDEVICE "/dev/ttyACM0"	//Conexió directa PC(Linux) - Arduino
#define _POSIX_SOURCE 1				//POSIX compliant source
#define MIDA 100					//Mida array de lectura i escritura
#define	MIDAS_BUFFER 5			//MIDA ARRAY CIRCULAR

//-------------------------------------ESCTRUCTURA PARA FUNCION SERIE---------------------------------------
struct termios oldtio,newtio;
void Enviar(int fd,char *missatge);	//Subrutina per a enviar dades per el port serie
void Rebre(int fd,char *buf);		//Subrutina per a rebre dades per el port serie
int	ConfigurarSerie(void);			//Configuració del port serie obert
void TancarSerie(int fd);			//Tancar comunicació

//-------------------------------------ESCTRUCTURA PARA FUNCION ARRAY CIRCULAR---------------------------------------
typedef struct _TipusMostra{
	long int pos;
	float	temperatura;
}TipusMostra;
struct{
	TipusMostra *dades;
	int	index_entrada;				//Apunta al lloc on es posarà la sagüent mostra
	int nombre_mostres;				//Nombre de mostres que hi ha el el buffer circular
}buffer_circular;

TipusMostra dada;

void buffer_cicular_introduir(TipusMostra dada);
void buffer_cicular_borrar_tot(void);
void buffer_cicular_bolcat_dades(void);
void buffer_cicular_inici(void);
void buffer_cicular_final(void);

//-------------------------------------INICIO PROGRAMA---------------------------------------
int t;
int main(int argc, char **argv)
{
	int fd,v=10,mostres=10,t=25;
	char buf[100];
	char missatge[100];
	printf("Espera a que el sistema inicie\n");
	
	fd = ConfigurarSerie();
	memset(missatge,'\0', MIDA);
	memset(buf,'\0', MIDA);
												// Enviar el missatge 1
	while (v !=0 || v !=1) 						//protección valores erroneos
	{
		printf("Posar en marxa [1] o parar [0]:");
		scanf("%i", &v);
		while (v != 1 && v!=0) 					//protección valores erroneos
		{
			printf("Posar en marxa [1] o parar [0]:");
			scanf("%i", &v);
		}
		if (v==1){ 								//si se pone en marcha realizamos acciones
			printf("Es posa en marxa l'adquisicio.\n");
			while (t <01 || t>20) 				//protección valores erroneos
			{
				printf("Temps de mostreig desitjat(1-20):");
				scanf("%i", &t); 				//guardamos el tiempo en una variable de tipo entero
			}
			while (mostres <01 || mostres>9) 	//protección valores erroneos
			{
				printf("Numero de mostres per fer la mitjana(1-9):");
				scanf("%i", &mostres);
			}
			sprintf(missatge,"AM%i%.2iZ",v,t);	//cargem a la variable a enviar les dades
			break;
		}
		else if (v==0)							//si se presiona finalizar volvemos a preguntar
		{ 
			printf("Adquisicio aturada.\n");
			sprintf(missatge,"AM000Z");			//cargem a la variable a enviar les dades
		}
	}
	
	Enviar(fd,missatge);
	sleep(1);
	memset(buf,'\0', MIDA);
	Rebre(fd,buf);
	
	int j=0,w=0;
	char lecturatemp[4];
	buffer_cicular_inici();
	float temp,minim=100000,maxim=0,mitja=0;
	while(1)
	{
		printf("capturando muestra...Número[%i]\n",j);
		memset(missatge,'\0', MIDA);
		memset(buf,'\0', MIDA);
		//ENCENEM/APAGAMOS LED 13 PER INFORMAR DE COMUNICACIONS
		if (w==0){w=1;}else{w=0;}
		sprintf(missatge,"AS13%iZ",w); 
		Enviar(fd,missatge);
		Rebre(fd,buf);
		usleep(500000);
		memset(missatge,'\0', MIDA);
		memset(buf,'\0', MIDA);
		//-----------------------------
		//Encenem Lectura de mostra
		sprintf(missatge,"ACZ");
		Enviar(fd,missatge);
		sleep(t-0.5);
		Rebre(fd,buf);
		//-----------------------------
		dada.pos = j;
		sprintf(lecturatemp,"%c%c%c%c%c",buf[3],buf[4],buf[5],buf[6],buf[7]);
		temp=atof(lecturatemp); // convertimos char a float
		dada.temperatura = temp;
		buffer_cicular_introduir(dada);
		if (temp>maxim)	{maxim = temp;}
		if (temp<minim)	{minim = temp;}
		buffer_cicular_bolcat_dades();	//***********************************************************************************************************
		int q=0,c=0,z=0;
		if (mostres<j+2)
		{
			if (buffer_circular.index_entrada==0) {z = MIDAS_BUFFER ;}
			else {z=buffer_circular.index_entrada;}
			for (q = z-1,c=1,temp=0; c<=mostres; q--, c++)
			{
				if (q==-1){q=MIDAS_BUFFER-1;}
//				printf("-->[%i]<--",q);
//				dada = buffer_circular.dades[q];
//				temp = temp + dada.temperatura;				
				temp = temp + buffer_circular.dades[q].temperatura;				
			}
			mitja = temp / mostres;			
		}
		//buffer_cicular_bolcat_dades();	//Fem un bocat del contingut del buffer circular
		printf("Maxim[%.2f]---------------------Minim[%.2f]-------mitja[%.2f]\n",maxim,minim,mitja);
		j++;
	};
	TancarSerie(fd);
	return 0;
}

//-------------------------------------SUBRUTINAS COMUNICACION SERIE---------------------------------------
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
	
	//printf("Enviats %d bytes: %s\n",res,missatge);	//***********************************************************************************************************
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
	//printf("Rebuts %d bytes: %s\n",res,buf);	//***********************************************************************************************************
}


//-------------------------------------SUBRUTINAS PARA BUFFER CIRCULAR---------------------------------------

void	buffer_cicular_borrar_tot(void){
	buffer_circular.index_entrada = 0;
	buffer_circular.nombre_mostres = 0;
}

void	buffer_cicular_bolcat_dades(void){
	int i;

	if (buffer_circular.nombre_mostres < MIDAS_BUFFER){

		for (i=0;i<buffer_circular.nombre_mostres;i++){
			dada = buffer_circular.dades[i];
			printf("Pos: %ld Temperatura: %.2f\n", dada.pos, dada.temperatura);
		}
	}
	else{
		for (i=buffer_circular.index_entrada;i<MIDAS_BUFFER;i++){
			dada = buffer_circular.dades[i];
			printf("Pos: %ld Temperatura: %.2f\n", dada.pos, dada.temperatura);
		}
		for (i=0;i<buffer_circular.index_entrada;i++){
			dada = buffer_circular.dades[i];
			printf("Pos: %ld Temperatura: %.2f\n", dada.pos, dada.temperatura);
		}
	}
}

void	buffer_cicular_inici(void){
	buffer_circular.dades = malloc(sizeof(TipusMostra)*MIDAS_BUFFER);
	buffer_circular.index_entrada = 0;
	buffer_circular.nombre_mostres = 0;
}

void	buffer_cicular_final(void){
	free(buffer_circular.dades);
}

void	buffer_cicular_introduir(TipusMostra dada){
	buffer_circular.dades[buffer_circular.index_entrada] = dada;

	buffer_circular.index_entrada++;
	if (buffer_circular.index_entrada == MIDAS_BUFFER){
		buffer_circular.index_entrada = 0; //Continuem pel principi: circular
	}

	if (buffer_circular.nombre_mostres < MIDAS_BUFFER){ //Agumentar fins que estigui ple
		buffer_circular.nombre_mostres++;
	}

}
