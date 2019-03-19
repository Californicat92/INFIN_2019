/***************************************************************************
                          exemple_menu_consola.c
                             -------------------
    begin                : 2012
    copyright            : (C) 2010 by A. Moreno
    email                : amoreno@euss.cat
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/************************
*
*
* ImprimirMenu
*
*
*/
void ImprimirMenu(void);

/************************
*
*
* main
*
*
*/
int main(int argc, char **argv)                                                               
{                                                                          
	char input,cadena[10];

	ImprimirMenu();                             
	//input = getchar();
	scanf("%s", &input);
	while (input != 's')
	{
		switch (input)
		{
			// declaración de variables para bucles dentro del switch (escribir cadenas)
			int i;
			case '1':
				printf("Heu seleccionat l'opció 1\n");	
				int v;
				do
				{
					printf("Posar en marxa [1] o parar [0]:");
					scanf("%i", &v);
					if (v==1){
						printf("Es posa en marxa l'adquisicio.\n");
						printf("Temps de mostreig desitjat(1-20):");
						int t;
						char temps[3];
						scanf("%i", &t);
						sprintf(temps, "%.2d", t);
						printf("Numero de mostres per fer la mitjana(1-9):");
						int mostres;
						scanf("%i", &mostres);
						printf("Temps:%.2d\nMostres:%i\n", t, mostres);
						printf("{M%i%s%i}\n",v,temps,mostres);
						break;
					}
					else if (v==0){ 
						printf("Adquisicio aturada.\n");
						puts("{M0000}\n");
						break;
					}                      
				} while (v!=1 || v!=0);    
				ImprimirMenu();  
				break;
				
			case '2':
				printf("Heu seleccionat l'opció 2\n");
				puts("{U}\n");
				puts("Del servidor hem de rebre {Ucnnnnn}\nEscriu la demo a continuació: ");
				scanf("%s",cadena);
				printf("S'ha rebut el codi d'error %c\n",cadena[2]);
				printf("La temperatura mitja rebuda del servidor es: ");
				for (i = 3; i < 8; i++) printf("%c",cadena[i]);
				printf("ºC");
				ImprimirMenu();                             
				break;
			case '3':
				printf("Heu seleccionat l'opció 3\n");	
				puts("{X}\n");
				puts("Del servidor hem de rebre {Xcnnnnn}\nEscriu la demo a continuació: ");
				scanf("%s",cadena);
				printf("S'ha rebut el codi d'error %c\n",cadena[2]);
				printf("La temperatura maxima rebuda del servidor es: ");
				for (i = 3; i < 8; i++) printf("%c",cadena[i]);
				printf("ºC");
				ImprimirMenu();                             
				break;
			case '4':
				printf("Heu seleccionat l'opció 4\n");	
				ImprimirMenu();                             
				break;
			case '5':
				printf("Heu seleccionat l'opció 5\n");	
				ImprimirMenu();                             
				break;
			case '6':
				printf("Heu seleccionat l'opció 6\n");	
				ImprimirMenu();                             
				break;
			case 0x0a: //Això és per enviar els 0x0a (line feed) que s'envia quan li donem al Enter
				break;
			default:
				printf("Opció incorrecta\n");	
				printf("He llegit %c \n",input);
				break;
		}
		
		//input = getchar();
		scanf("%s", &input);	
	}
	printf("Heu seleccionat la opció sortida\n");
	return 0;
}
void ImprimirMenu(void)
{
	printf("\n\nMenu:\n");
	printf("-------------------------------------------------------\n");
	printf("1.-Demanar última mitjana (promig del n-mostres anteriors)\n");
	printf("2.-Demanar màxim\n");
	printf("3.-Demanar mínim\n");
	printf("4.-Reset màxim i mínim\n");
	printf("5.-Demanar comptador\n");
	printf("6.-Posar en marxa i parar adquisició\n");
	printf("Premer 's' per sortir \n");
	printf("-------------------------------------------------------\n");
}

