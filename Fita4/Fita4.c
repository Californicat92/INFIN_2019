/***************************************************************************
                          main.c  -  threads
                             -------------------
    begin                : ene 30 19:49:08 CET 2002
    copyright            : (C) 2002 by A. Moreno
    email                : amoreno@euss.es
 ***************************************************************************/
//gcc Fita4.c -lpthread -lrt -o Fita4
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <fcntl.h>                                                        
#include <termios.h>       
#include <sys/ioctl.h>  

//****************************SERVIDOR************************************//
#define SERVER_PORT_NUM		5001
#define SERVER_MAX_CONNECTIONS	4

#define REQUEST_MSG_SIZE	1024
#define CONTAS 30

//****************************CONTROL SENSOR************************************//
//-------------------------------------DEFINICION DE CONSTANTES PARA EL PROGRAMA---------------------------------------
#define BAUDRATE B9600				//Velocitat port serie (BAUDRATE)
//#define MODEMDEVICE "/dev/ttyS0"	//Conexió IGEP - Arduino
#define MODEMDEVICE "/dev/ttyACM0"	//Conexió directa PC(Linux) - Arduino
#define _POSIX_SOURCE 1				//POSIX compliant source
#define MIDA 100					//Mida array de lectura i escritura
#define	MIDAS_BUFFER 3600			//MIDA ARRAY CIRCULAR

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

//****************************VARIABLES GLOBALS************************************//
int v=10, temps=100, mostres, compt_iteracions=0;
float minim=100000,maxim=0,mitja=0;
pthread_t thread;
pthread_mutex_t mutex;
TipusMostra dada;

void buffer_cicular_introduir(TipusMostra dada);
void buffer_cicular_borrar_tot(void);
void buffer_cicular_bolcat_dades(void);
void buffer_cicular_inici(void);
void buffer_cicular_final(void);

//****************************DECLARACIÓ DE FUNCIÓ CONTROL SENSOR************************************//

void* codi_fill(void* parametre); // codi thread fill


//****************************PROGRAMA PRINCIPAL SERVIDOR************************************//
int main(int argc, char *argv[])
{
	struct sockaddr_in	serverAddr;
	struct sockaddr_in	clientAddr;
	unsigned int			sockAddrSize;
	int			sFd;
	int			newFd;
	int 		result;
	char		buffer[256];
	char		missatge[20];
	char		tempt[2];
	


	

	printf("Proces pare 1 PID(%d) \n",getpid() );
	pthread_create(&thread, NULL, codi_fill, 0); //Es crea el thread fill
	printf("Proces pare 2 PID(%d) \n",getpid() );// Proces Pare
  
	/*Preparar l'adreÃ§a local*/
	sockAddrSize=sizeof(struct sockaddr_in);
	bzero ((char *)&serverAddr, sockAddrSize); //Posar l'estructura a zero
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT_NUM);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	/*Crear un socket*/
	sFd=socket(AF_INET, SOCK_STREAM, 0);
	
	/*Nominalitzar el socket*/
	result = bind(sFd, (struct sockaddr *) &serverAddr, sockAddrSize);
	
	/*Crear una cua per les peticions de connexio*/
	result = listen(sFd, SERVER_MAX_CONNECTIONS);
	/*Bucle s'acceptaciÃ³ de connexions*/

	while(1){
	

		
		printf("\nServidor esperant connexions\n");
		memset(missatge,'\0',20);
		
		/*Esperar conexio. sFd: socket pare, newFd: socket fill*/
		newFd=accept(sFd, (struct sockaddr *) &clientAddr, &sockAddrSize);
		printf("ConnexiÃ³n acceptada del client: adreÃ§a %s, port %d\n",	inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		/*Rebre*/
		result = read(newFd, buffer, 256);
		printf("Missatge rebut del client(bytes %d): %s\n",	result, buffer);
		
		pthread_mutex_lock(&mutex);// inici mutex
		if(buffer[0]=='{'){ //comprobamos que el mensaje empieza por '}'
			switch(buffer[1]){ //en el segundo bit del array se escribira la funcion que queramos hacer segun la letra M,U,X...
			
				case 'M':
					if (strlen(buffer)!=7){  //comprobamos que la array sea de 7 bits '{''M''v'"temp""temps""Num"'}'
						sprintf(missatge,"{M1}");// error en el protocolo
						break;
					}
			
					if(buffer[2]==49 || buffer[2]==48){ //comprobamos si el array[2] 'v' es 0(48 ASCII) o 1(49 ASCII)
						v=buffer[2]-'0';  //le damos el valor a la variable v
						//compt_iteracions=0;	//reset del comptador
						//buffer_cicular_borrar_tot(); //reset array circular
						if(v==0){
							sprintf(missatge,"{M0}");//en el caso de que sea 0 paramos el programa y mostramos 0 conforme no ha habido ningun error
							break;
						}
					}
					else{		
						sprintf(missatge,"{M2}");//en el caso de que se un numero diferente a 0 o 1 imprimimos el error 2 de error en los parametros
						break;
					}
					
					if(buffer[3]>=50 && buffer[4]!=48){ // el tiempo tiene que se como maximo 20 asi que el primer bit como mucho es 2(50) y el segundo si es 2 tiene que ser 0(48)
						sprintf(missatge,"{M2}");//error de parametros 2
						break;
					} 	
					else {
						sprintf(tempt,"%c%c",buffer[3],buffer[4]);
						temps=atoi(tempt); // convertimos char a float
					}

					if(buffer[5]!=48){ //en el bit 6 de la array tiene q ser un valor entre 1 y 9
						mostres=buffer[5]-'0'; //si es valor es diferente a 0 damos el valor a la variable num
					}	
					else{
						sprintf(missatge,"{M2}");//si es numero es 0 imprimimos el error 2 de parametros
						break;
					}
					
					if(buffer[6]=='}'){ //comprobamos que el mensaje en el array termine cn '}'
						sprintf(missatge,"{M0}");//manda el mensaje que todo OK
					}	 
					else{
						sprintf(missatge,"{M1}");//manda el mensaje de error en el protocolo
					}
					printf("\nEl valor de marxa(v) es: %d", v);
					printf("\nEl temps per mostra es: %d", temps);
					printf("\nEl valor del nombre de mostres per fer la mitjana: %d", mostres);
				break;

				case 'U':
					if(strlen(buffer)!=3){ //comprobamos que el array tenga 3 bits
						sprintf(missatge,"{U1}");//error de protocolo
					break;
					}
					if(buffer[2]=='}'){ //comprobamos que el mensaje en el array termine cn '}'
						sprintf(missatge,"{U0%2.2f}",mitja);
					}	 
					else{
						sprintf(missatge,"{U1}");//manda el mensaje de error en el protocolo
					}
				break;
			
				case 'X':
					if(strlen(buffer)!=3){ //comprobamos que el array tenga 3 bits
						printf("{X1}"); //error de protocolo
						sprintf(missatge,"{X1}");
					break;
					}
					if(buffer[2]=='}'){ //comprobamos que el mensaje en el array termine cn '}'
						if (maxim < 10){sprintf(missatge,"{X00%.2f}",maxim);}else{sprintf(missatge,"{X0%.2f}",maxim);}
					}	
					else{
						sprintf(missatge,"{X1}"); //manda el mensaje de error en el protocolo
					}
				break;
				
				case 'Y':
					if(strlen(buffer)!=3){ //comprobamos que el array tenga 3 bits
						printf("{Y1}"); //error de protocolo
						sprintf(missatge,"{Y1}");
					break;
					}
					if(buffer[2]=='}'){ //comprobamos que el mensaje en el array termine cn '}'
						if (minim < 10){sprintf(missatge,"{Y00%.2f}",minim);}else{sprintf(missatge,"{Y0%.2f}",minim);}
						
					}	 
					else{
						sprintf(missatge,"{Y1}"); //manda el mensaje de error en el protocolo
					}			
				break;
				
				case 'R':
					if(strlen(buffer)!=3){ //comprobamos que el array tenga 3 bits
						sprintf(missatge,"{R1}"); //error de protocolo
					break;
					}
					if(buffer[2]=='}'){ 	//comprobamos que el mensaje en el array termine cn '}'
						maxim=00.00;		//reset del maxim
						minim=00.00;		//reset del minim
						//mitja=00.00;		//reset de la mitja
						//compt_iteracions=0;	//reset del comptador
						//buffer_cicular_borrar_tot(); //reset array circular
						sprintf(missatge,"{R0}");
					}	
					else{
						sprintf(missatge,"{R1}");//manda el mensaje de error en el protocolo
					}
				break;

				case 'B':
					if(strlen(buffer)!=3){ //comprobamos que el array tenga 3 bits
						sprintf(missatge,"{B10000}"); //error de protocolo
					break;
					}
					if(buffer[2]=='}'){ //comprobamos que el mensaje en el array termine con '}'
						sprintf(missatge,"{B0%.4d}",compt_iteracions);
					}	
					else{
						sprintf(missatge,"{B10000}");//manda el mensaje de error en el protocolo
					}
				break;
				
				default: 
					printf("{B1}");
					sprintf(missatge,"{B1}");
				break;
			}
			pthread_mutex_unlock(&mutex);			//Final del mutex
			/*Enviar*/
			strcpy(buffer,missatge); //Copiar missatge a buffer
			result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena
			printf("Missatge enviat a client(bytes %d): %s\n",	result, missatge);
			memset(buffer,'\0',256);		
		}
		/*Tancar el socket fill*/
		result = close(newFd);		
	}
	
	pthread_join(thread, NULL);
	
}
//****************************PROGRAMA ADQUISICIO DADES ARDUINO************************************//

void* codi_fill(void* parametre){ // codi thread fill

	int fd;
	char buf[100];
	char miss[100];
	printf("Espera a que el sistema inicie\n");
	
	fd = ConfigurarSerie();
	while(1)
	{
		memset(miss,'\0', MIDA);								//Borrar contenido de miss
		memset(buf,'\0', MIDA);									//Borrar contenido de buf	
		v=20;													//Se da el valor de 20 a <<v>> para evitar que entre en el bucle antes de que se reciba un mensaje en el servidor 
		while (v !=0 || v !=1) 									//Protección valores erroneos
		{

			if (v==1){ 											//Si se pone en marcha realizamos acciones		
				pthread_mutex_lock(&mutex);						//Inici mutex

				sprintf(miss,"AM%i%.2iZ",v,temps);				//Cargamos a la variable miss el mensaje de marcha y tiempo definido en el servidor
				pthread_mutex_unlock(&mutex);					//Final mutex

				break;
			}
			else if (v==0)										//Si se pone en paro se paran acciones y vuelve a preguntar
			{ 
				sprintf(miss,"AM000Z");							//Cargamos a la variable miss el mensaje de paro
			}
		}
		if (v==0) {printf("Adquisicio aturada.\n");}			//Mensaje de paro de adquisicion en paro fuera del bucle para que no se repita.
		Enviar(fd,miss);										//Llamada a la funcion de envio de mensaje para enviar --> se copia el contenido de miss en fd para el envio
		sleep(1);
		memset(buf,'\0', MIDA);									//Borrar contenido de varible buf
		Rebre(fd,buf);											//Llamada a la funcion de recibo de mensaje --> se copia el contenido de fd en buf para el recivo
		
		int w=0;												//variable que se usa para alternar el encendido y apagado del pin 13 de arduino
		char lecturatemp[4];
		buffer_cicular_inici();
		float temp;
		while(v==1)
		{
			printf("capturando muestra...Número[%i]\n",compt_iteracions);
			memset(miss,'\0', MIDA);
			memset(buf,'\0', MIDA);
			//ENCENEM/APAGAMOS LED 13 PER INFORMAR DE COMUNICACIONS
			if (w==0){w=1;}else{w=0;}
			sprintf(miss,"AS13%iZ",w); 
			Enviar(fd,miss);
			Rebre(fd,buf);
			usleep(500000);
			memset(miss,'\0', MIDA);
			memset(buf,'\0', MIDA);
			//-----------------------------
			//Encenem Lectura de mostra
			sprintf(miss,"ACZ");
			Enviar(fd,miss);
			sleep(temps-0.5);
			Rebre(fd,buf);
			//-----------------------------
			pthread_mutex_lock(&mutex);// inici mutex
			dada.pos = compt_iteracions;
			sprintf(lecturatemp,"%c%c%c%c%c",buf[3],buf[4],buf[5],buf[6],buf[7]);
			temp=atof(lecturatemp); // convertimos char a float
			temp=temp*(70.0/1024.0);    // convertim de 0 a 0 i de 1024 a 70ºC
			dada.temperatura = temp;
			buffer_cicular_introduir(dada);
			if (temp>maxim)	{maxim = temp;}	
			if (temp<minim)	{minim = temp;}
			buffer_cicular_bolcat_dades();	//***********************************************************************************************************
			int q=0,c=0,z=0;
			if (mostres<compt_iteracions+2)
			{
				if (buffer_circular.index_entrada==0) {z = MIDAS_BUFFER ;}
				else {z=buffer_circular.index_entrada;}
				for (q = z-1,c=1,temp=0; c<=mostres; q--, c++)
				{
					if (q==-1){q=MIDAS_BUFFER-1;}			
					temp = temp + buffer_circular.dades[q].temperatura;				
				}
				mitja = temp / mostres;			
			}
			pthread_mutex_unlock(&mutex);// inici mutex
			//buffer_cicular_bolcat_dades();	//Fem un bocat del contingut del buffer circular
			printf("Maxim[%.2f ºC]---------------------Minim[%.2f ºC]-------mitja[%.2f ºC]\n",maxim,minim,mitja);
			compt_iteracions++;
		}
	}
	TancarSerie(fd);
	pthread_exit(NULL);
	return NULL;
	
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
