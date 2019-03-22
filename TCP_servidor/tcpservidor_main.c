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

#define SERVER_PORT_NUM		5001
#define SERVER_MAX_CONNECTIONS	4

#define REQUEST_MSG_SIZE	1024


/************************
*
*
* tcpServidor
*
*
*/
void contador_dadas();

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
	char		missatge[20], mitjana2[5], maxim2[5], minim2[5];
	int v, temps[2], num;
	float mitjana=37.43, maxim=40.34, minim=12.93;
	char error[]="{  }";
	char dada[]="{ 1     }";
	int cont=0;



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
	
	/*Crear una cua per les peticions de connexiÃ³*/
	result = listen(sFd, SERVER_MAX_CONNECTIONS);
	
	/*Bucle s'acceptaciÃ³ de connexions*/
	while(1){
		
		printf("\nServidor esperant connexions\n");
		memset(buffer,'\0',256);
		memset(missatge,'\0',20);
		/*Esperar conexiÃ³. sFd: socket pare, newFd: socket fill*/
		newFd=accept(sFd, (struct sockaddr *) &clientAddr, &sockAddrSize);
		printf("ConnexiÃ³n acceptada del client: adreÃ§a %s, port %d\n",	inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

		/*Rebre*/
		result = read(newFd, buffer, 256);
		printf("Missatge rebut del client(bytes %d): %s\n",	result, buffer);
		

		if(buffer[0]=='{'){ //comprobamos que el mensaje empieza por '}'
			switch(buffer[1]){ //en el segundo bit del array se escribira la funcion que queramos hacer segun la letra M,U,X...
			
				case 'M':
					if (strlen(buffer)!=7){  //comprobamos que la array sea de 7 bits '{''M''v'"temp""temps""Num"'}'
						
						memset(missatge,'\0',20);
						
						error[1]='M';
						error[2]='1';
						strcat(missatge, error);// error en el protocolo
						
					break;
					}
			
					if(buffer[2]==49){ //comprobamos si el array[2] 'v' es 0(48 ASCII) o 1(ASCII)
					v=buffer[2];  //le damos el valor a la variable v
					}	else if(buffer[2]==48){
							memset(missatge,'\0',20);
							v=buffer[2];
							error[1]='M';
							error[2]='0';
							strcat(missatge, error);//en el caso de que sea 0 paramos el programa y mostramos 0 conforme no ha habido ningun error
						break;
						} 	else{		
								memset(missatge,'\0',20);
								error[1]='M';
								error[2]='2';
								strcat(missatge, error); //en el caso de que se un numero diferente a 0 o 1 imprimimos el error 2 de error en los parametros
							break;
							}
					
					if(buffer[3]>=50 && buffer[4]!=48){ // el tiempo tiene que se como maximo 20 asi que el primer bit como mucho es 2(50) y el segundo si es 2 tiene que ser 0(48)
							memset(missatge,'\0',20);
							error[1]='M';
							error[2]='2';
							strcat(missatge, error);//error de parametros 2
					break;
					} 	else {
						temps[0]=buffer[3]; //en el caso de que todo este correcto damos los valores de la array a la variable temps
						temps[1]=buffer[4];
					}
					if(buffer[5]!=48){ //en el bit 6 de la array tiene q ser un valor entre 1 y 9
						num=buffer[5]; //si es valor es diferente a 0 damos el valor a la variable num
					}	else{
							memset(missatge,'\0',20);
							error[1]='M';
							error[2]='2';
							strcat(missatge, error); //si es numero es 0 imprimimos el error 2 de parametros
						break;
						}	
					if(buffer[6]=='}'){ //comprobamos que el mensaje en el array termine cn '}'
							memset(missatge,'\0',20);
							error[1]='M';
							error[2]='0';
							strcat(missatge, error);//manda el mensaje que todo OK
					}	 else{
							memset(missatge,'\0',20);
							error[1]='M';
							error[2]='1';
							strcat(missatge, error); //manda el mensaje de error en el protocolo
						}
					printf("\nel valor de marxa(v) es: %c", v);
					printf("\nel temps per muostra es: %c%c", temps[0], temps[1]);
					printf("\nel valor del nombre de mostres per fer la mitjana: %c", num);
				break;

			
				case 'U':
					if(strlen(buffer)!=3){ //comprobamos que el array tenga 3 bits
						missatge[0]='{';
						missatge[1]='U';
						missatge[2]='1';
						missatge[3]='}'; //error de protocolo
					break;
					}
					if(buffer[2]=='}'){ //comprobamos que el mensaje en el array termine cn '}'
							memset(missatge,'\0',20);
							missatge[0]='{';
							missatge[1]='U';
							missatge[2]='0';
							gcvt(mitjana, 4, mitjana2);						
							strcat(missatge, mitjana2);
							missatge[8]='}';
							

							
										
						//printf("{U0%.2f}", mitjana); //imprimimos el mensaje con la mitjana (esta mitjana tiene que tener 5 bits contando el punto)
					}	 else{
						memset(missatge,'\0',20);
						missatge[0]='{';
						missatge[1]='U';
						missatge[2]='1';
						missatge[3]='}'; //manda el mensaje de error en el protocolo
						}
				
				break;
			
			
				case 'X':
					if(strlen(buffer)!=3){ //comprobamos que el array tenga 3 bits
						printf("{X1}"); //error de protocolo
					break;
					}
					if(buffer[2]=='}'){ //comprobamos que el mensaje en el array termine cn '}'
							memset(missatge,'\0',20);
							missatge[0]='{';
							missatge[1]='X';
							missatge[2]='0';
							gcvt(maxim, 4, maxim2);						
							strcat(missatge, maxim2);
							missatge[8]='}';//imprimimos el mensaje con el maximo (este maximo tiene que tener 5 bits contando el punto)
					}	 else{
						printf("{X1}"); //manda el mensaje de error en el protocolo
						}
					
				break;
				
				
				case 'Y':
					if(strlen(buffer)!=3){ //comprobamos que el array tenga 3 bits
						printf("{Y1}"); //error de protocolo
					break;
					}
					if(buffer[2]=='}'){ //comprobamos que el mensaje en el array termine cn '}'
							memset(missatge,'\0',20);
							missatge[0]='{';
							missatge[1]='Y';
							missatge[2]='0';
							gcvt(minim, 4, minim2);						
							strcat(missatge, minim2);
							missatge[8]='}';  //imprimimos el mensaje con el minimo (este minimo tiene que tener 5 bits contando el punto)
					}	 else{
						printf("{Y1}"); //manda el mensaje de error en el protocolo
						}			
			
				break;
				
				
				case 'R':
					if(strlen(buffer)!=3){ //comprobamos que el array tenga 3 bits
						printf("{R1}"); //error de protocolo
					break;
					}
					if(buffer[2]=='}'){ //comprobamos que el mensaje en el array termine cn '}'
						maxim=0; //reset del maxim
						minim=1000; //reset del minim
						printf("{R0}");
					}	 else{
						printf("{R1}"); //manda el mensaje de error en el protocolo
						}
				
				break;

				case 'B':
					if(strlen(buffer)!=3){ //comprobamos que el array tenga 3 bits
						printf("{B1}"); //error de protocolo
					break;
					}
					
				break;
			
				default: printf("{B1}");
		
			}
	
		
		/*Enviar*/
		strcpy(buffer,missatge); //Copiar missatge a buffer
		result = write(newFd, buffer, strlen(buffer)+1); //+1 per enviar el 0 final de cadena
		printf("Missatge enviat a client(bytes %d): %s\n",	result, buffer);
		
	}
		/*Tancar el socket fill*/
		result = close(newFd);
	}
}


void contador_dadas(){
	int i;
	int cont=0;
	for (i = 0; i <=3600; i++)
	{
		cont++;
	}
	
	}
