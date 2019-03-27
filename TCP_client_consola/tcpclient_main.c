/***************************************************************************
                          main.c  -  client
                             -------------------
    begin                : lun feb  4 16:00:04 CET 2002
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
#include <unistd.h>
#include <string.h>


#define REQUEST_MSG_SIZE	1024
#define REPLY_MSG_SIZE		500
#define SERVER_PORT_NUM		5001


void E_R_Datos(char missatge[10],char buffer[10]);
void ImprimirMenu(void);

 /************************
*
*
* tcpClient
*
*
*/
int main(int argc, char *argv[]){

	char input,enviat[10],rebut[10];

	ImprimirMenu();
	scanf("%s", &input);
	while (input != 's')
	{
		memset (enviat,'\0',10);
		memset (rebut,'\0',10);	
		switch (input)
		{
			// declaración de variables para bucles dentro del switch (escribir cadenas)
			int i;
			//Limpiamos el espacio de memoria de enviat para que no se cuelen caracteres extraños	
			case '1':
				printf("Heu seleccionat l'opció 1\n");	
				int v=10; //variable de marxa o paro
				while (v !=0 || v !=1) //protección valores erroneos
				{
		
					printf("Posar en marxa [1] o parar [0]:");
					scanf("%i", &v);
					while (v != 1 && v!=0) //protección valores erroneos
					{
						printf("Posar en marxa [1] o parar [0]:");
						scanf("%i", &v);
					}
					if (v==1){ //si se pone en marxa realizamos acciones
						printf("Es posa en marxa l'adquisicio.\n");

						int t=25;
						char temps[2]; //declaramos un array			
						memset(temps,'\0', 10); 	
						while (t <01 || t>20) //protección valores erroneos
						{
							printf("Temps de mostreig desitjat(1-20):");
							scanf("%i", &t); //guardamos el tiempo en una variable de tipo entero
						}
						int mostres=10;
						while (mostres <01 || mostres>9) //protección valores erroneos
						{
							printf("Numero de mostres per fer la mitjana(1-9):");
							scanf("%i", &mostres);
						}
						//Guardem la dada a enviar
						sprintf(enviat,"{M%i%.2d%i}",v,t,mostres); //cargem a la variable a enviar les dades
					}
					else if (v==0){ //si se para finalizamos
						printf("Adquisicio aturada.\n");
						sprintf(enviat,"{M0000}"); //cargem a la variable a enviar les dades
					}
					E_R_Datos(enviat, rebut);   
					printf("\nS'ha rebut el codi d'error %c\n",rebut[2]);
					break;              
				}
				ImprimirMenu();  
				break;
				
			case '2':
				printf("Heu seleccionat l'opció 2\n");
				sprintf(enviat,"{U}"); //cargem a la variable a enviar les dades
				E_R_Datos(enviat, rebut);
				printf("S'ha rebut el codi d'error %c\n",rebut[2]);
				printf("La temperatura mitja rebuda del servidor es: ");
				for (i = 3; i < 8; i++) printf("%c",rebut[i]); //muestra los valores de las posiciones del array que guardan la temperatura
				printf("ºC");
				ImprimirMenu();                             
				break;
			case '3':
				printf("Heu seleccionat l'opció 3\n");	
				sprintf(enviat,"{X}"); //cargem a la variable a enviar les dades
				E_R_Datos(enviat, rebut);
				printf("S'ha rebut el codi d'error %c\n",rebut[2]);
				printf("La temperatura màxima rebuda del servidor es: ");
				for (i = 3; i < 8; i++) printf("%c",rebut[i]); //muestra los valores de las posiciones del array que guardan la temperatura
				printf("ºC");
				ImprimirMenu();                             
				break;
			case '4':
				printf("Heu seleccionat l'opció 4\n");	
				sprintf(enviat,"{Y}"); //cargem a la variable a enviar les dades
				E_R_Datos(enviat, rebut);
				printf("S'ha rebut el codi d'error %c\n",rebut[2]);
				printf("La temperatura mínima rebuda del servidor es: ");
				for (i = 3; i < 8; i++) printf("%c",rebut[i]); //muestra los valores de las posiciones del array que guardan la temperatura
				printf("ºC");
				ImprimirMenu();                             
				break;
			case '5':
				printf("Heu seleccionat l'opció 5\n");	
				sprintf(enviat,"{R}"); //cargem a la variable a enviar les dades
				E_R_Datos(enviat, rebut);
				printf("S'ha rebut el codi d'error %c\n",rebut[2]);
				ImprimirMenu();                             
				break;
			case '6':
				printf("Heu seleccionat l'opció 6\n");	
				sprintf(enviat,"{B}"); //cargem a la variable a enviar les dades
				E_R_Datos(enviat, rebut);
				printf("S'ha rebut el codi d'error %c\n",rebut[2]);
				printf("Nombre de mostres guardades: ");
				for (i = 3; i < 7; i++) printf("%c",rebut[i]); //muestra los valores de las posiciones del array que guardan las muestras
				ImprimirMenu();                             
				break;
			case 0x0a: //Això és per enviar els 0x0a (line feed) que s'envia quan li donem al Enter
				break;
			default:
				printf("Opció incorrecta\n");	
				printf("He llegit %c \n",input);
				break;
			}
		scanf("%s", &input);
		}

	
	printf("Heu seleccionat la opció sortida\n");
	return 0;
}

void ImprimirMenu(void)
{
	printf("\n\nMenu:\n");
	printf("-------------------------------------------------------\n");
	printf("1.-Posar en marxa i parar adquisició\n");
	printf("2.-Demanar última mitjana (promig del n-mostres anteriors)\n");
	printf("3.-Demanar màxim\n");
	printf("4.-Demanar mínim\n");
	printf("5.-Reset màxim i mínim\n");
	printf("6.-Demanar comptador\n");
	printf("Premer 's' per sortir \n");
	printf("-------------------------------------------------------\n");
}

void E_R_Datos(char missatge[10],char buffer[10]){

	struct sockaddr_in	serverAddr;
	char	    serverName[] = "127.0.0.1"; //Adreça IP on est� el servidor
	int			sockAddrSize;
	int			sFd;
	int 		result;


	/*Crear el socket*/
	sFd=socket(AF_INET,SOCK_STREAM,0);

	/*Construir l'adreça*/
	sockAddrSize = sizeof(struct sockaddr_in);
	bzero ((char *)&serverAddr, sockAddrSize); //Posar l'estructura a zero
	serverAddr.sin_family=AF_INET;
	serverAddr.sin_port=htons (SERVER_PORT_NUM);
	serverAddr.sin_addr.s_addr = inet_addr(serverName);

	/*Conexió*/
	result = connect (sFd, (struct sockaddr *) &serverAddr, sockAddrSize);
	if (result < 0)
	{
		printf("Error en establir la connexió\n");
		exit(-1);
	}
	//printf("\nConnexió establerta amb el servidor: adreça %s, port %d\n",	inet_ntoa(serverAddr.sin_addr), ntohs(serverAddr.sin_port));

	/*Enviar*/
	strcpy(buffer,missatge); //Copiar missatge a buffer
	result = write(sFd, buffer, strlen(buffer));
	//printf("Missatge enviat a servidor(bytes %d): %s\n",	result, missatge);

	/*Rebre*/
	result = read(sFd, buffer, 10);
	//printf("Missatge rebut del servidor(bytes %d): %s\n",	result, buffer);
	
	/*Tancar el socket*/
	close(sFd);
}
