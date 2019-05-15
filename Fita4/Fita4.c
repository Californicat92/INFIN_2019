/***************************************************************************
                          main.c  -  threads
                             -------------------
    begin                : ene 30 19:49:08 CET 2002
    copyright            : (C) 2002 by A. Moreno
    email                : amoreno@euss.es
 ***************************************************************************/

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

//****************************VARIABLES GLOBALS************************************//
int v, temps[2], num, mostres, nmitja=1;
float minim=100000,maxim=0,mitja=0;
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
	int			sockAddrSize;
	int			sFd;
	int			newFd, d, c;
	int			nRead;
	int 		result;
	char		buffer[256];
	char		missatge[20];
	

	pthread_t thread;
	
	

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
		

		if(buffer[0]=='{'){ //comprobamos que el mensaje empieza por '}'
			switch(buffer[1]){ //en el segundo bit del array se escribira la funcion que queramos hacer segun la letra M,U,X...
			
				case 'M':
					if (strlen(buffer)!=7){  //comprobamos que la array sea de 7 bits '{''M''v'"temp""temps""Num"'}'
						sprintf(missatge,"{M1}");// error en el protocolo
						break;
					}
			
					if(buffer[2]==49 || buffer[2]==48){ //comprobamos si el array[2] 'v' es 0(48 ASCII) o 1(49 ASCII)
						v=buffer[2];  //le damos el valor a la variable v
						sprintf(missatge,"{M0}");//en el caso de que sea 0 paramos el programa y mostramos 0 conforme no ha habido ningun error
						break;
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
						temps[0]=buffer[3]; //en el caso de que todo este correcto damos los valores de la array a la variable temps
						temps[1]=buffer[4];
					}
					if(buffer[5]!=48){ //en el bit 6 de la array tiene q ser un valor entre 1 y 9
						num=buffer[5]; //si es valor es diferente a 0 damos el valor a la variable num
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
					printf("\nEl valor de marxa(v) es: %c", v);
					printf("\nEl temps per mostra es: %c%c", temps[0], temps[1]);
					printf("\nEl valor del nombre de mostres per fer la mitjana: %c", num);
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
					if(buffer[2]=='}'){ //comprobamos que el mensaje en el array termine cn '}'
						maxim=00.00; //reset del maxim
						minim=00.00; //reset del minim
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
						sprintf(missatge,"{B0%.4d}",mostres);
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






void* codi_fill(void* parametre){ // codi thread fill
   
   
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
	float temp;
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
