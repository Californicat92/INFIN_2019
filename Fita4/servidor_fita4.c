#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//-------------------------------------LIBRERIAS---------------------------------------
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>
#include <fcntl.h>                                                        
#include <termios.h>       
#include <sys/ioctl.h>   

//-------------------------------------DEFINICION DE CONSTANTES PARA LA COMUNICACIÓN SERIE---------------------------------------
#define BAUDRATE B9600				//Velocitat port serie (BAUDRATE)
//#define MODEMDEVICE "/dev/ttyS0"	//Conexió IGEP - Arduino
#define MODEMDEVICE "/dev/ttyACM0"	//Conexió directa PC(Linux) - Arduino
#define _POSIX_SOURCE 1				//POSIX compliant source
#define MIDA 100					//Mida array de lectura i escritura
#define	MIDAS_BUFFER 3600			//MIDA ARRAY CIRCULAR
//-------------------------------------DEFINICION DE CONSTANTES PARA LA COMUNICACION TCP/IP---------------------------------------
#define SERVER_PORT_NUM		5001
#define SERVER_MAX_CONNECTIONS	4

#define REQUEST_MSG_SIZE	1024
#define CONTAS 30


//-------------------------------------DEFINICION DE VARIABLES GLOBALES---------------------------------------
int v=0, temps[2], num, mostres, nmitja=1;
float mitjana=00.00, maxim=00.00, minim=00.00;

void* codi_fill(void* parametre){ // codi thread fill
		while(1){
  		printf("MARXA= %i", v);
	}
    pthread_exit(NULL);
    return NULL;
}


int main(int argc, char *argv[])
{
	pthread_t thread;
	struct sockaddr_in	serverAddr;
	struct sockaddr_in	clientAddr;
	int			sockAddrSize;
	int			sFd;
	int			newFd;
	int			nRead;
	int 		result;
	char		buffer[256];
	char		missatge[20];
	
	 pthread_create(&thread, NULL, codi_fill, 0); //Es crea el thread fill

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
			
				case 'M': //marxa-paro
					if (strlen(buffer)!=7){  //comprobamos que la array sea de 7 bits '{''M''v'"temp""temps""Num"'}'
						sprintf(missatge,"{M1}");// error en el protocolo
						break;
					}
			
					if(buffer[2]==49 || buffer[2]==48){ //comprobamos si el array[2] 'v' es 0(48 ASCII) o 1(49 ASCII)
						v=buffer[2];  //le damos el valor a la variable v
						sprintf(missatge,"{M0}");//en el caso de que sea 0 paramos el programa y mostramos 0 conforme no ha habido ningun error
						//comptador_dades(&maxim,&minim,&mitjana,&mostres,nmitja);
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

				case 'U': //mitjana
					if(strlen(buffer)!=3){ //comprobamos que el array tenga 3 bits
						sprintf(missatge,"{U1}");//error de protocolo
					break;
					}
					if(buffer[2]=='}'){ //comprobamos que el mensaje en el array termine cn '}'
						sprintf(missatge,"{U0%2.2f}",mitjana);
					}	 
					else{
						sprintf(missatge,"{U1}");//manda el mensaje de error en el protocolo
					}
				break;
			
				case 'X': //maximo
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
				
				case 'Y': //minimo
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
				
				case 'R': //reset maxim y minim
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

				case 'B': //numero de muestras
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
}
