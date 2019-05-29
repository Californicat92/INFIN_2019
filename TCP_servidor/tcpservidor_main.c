/***************************************************************************
                          main.c  -  server
                             -------------------
    begin                : lun feb  4 15:30:41 CET 2002
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
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <fcntl.h>                                                        
#include <termios.h>       
#include <sys/ioctl.h>  

#define SERVER_PORT_NUM		5001
#define SERVER_MAX_CONNECTIONS	4

#define REQUEST_MSG_SIZE	1024
#define CONTAS 30

/************************
*
*
* tcpServidor
*
*
*/
void comptador_dades(float *maxim, float *minim, float *mitja, int *comptador, int nmitja);

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
	char 		tempt[2];
	int v=10, temps=0, num, mostres, nmitja=1;
	float mitjana=00.00, maxim=00.00, minim=10000;



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
	
	/*Crear 3600 dades aleatories*/
	comptador_dades(&maxim,&minim,&mitjana,&mostres,nmitja);
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
						v=buffer[2]-'0';  //le damos el valor a la variable v
						sprintf(missatge,"{M0}");//en el caso de que sea 0 paramos el programa y mostramos 0 conforme no ha habido ningun error
						if(v==0){
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
					printf("\nEl valor de marxa(v) es: %d", v);
					printf("\nEl temps per mostra es: %d", temps);
					printf("\nEl valor del nombre de mostres per fer la mitjana: %c", num);
				break;

				case 'U':
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
}


void comptador_dades(float *maxim, float *minim, float *mitja, int *comptador, int nmitja){
	*comptador=0;
	*minim = 50;
	*maxim = 0;
	
	int i,c;
	float dades[CONTAS],mostra=0,temp;	
	while(*comptador<CONTAS){
		dades[*comptador] = (rand()%10)/0.27;
		mostra = dades[*comptador%CONTAS];
		if (mostra>*maxim)
		{
			*maxim = mostra;
		}
		else if (mostra<*minim)
		{
			*minim=mostra;
		}
		temp = mostra;
	
		for (i = *comptador,c=1; c == nmitja; i--, c++)
		{
			temp = temp + dades[i];
		}
		*mitja = temp / nmitja;
		*comptador = *comptador + 1;
	}
}
