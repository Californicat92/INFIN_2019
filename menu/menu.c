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
	char input;

	ImprimirMenu();                             
	input = getchar();

	while (input != 's')
	{
		if (input > '~'){input=' ';}
		{	
			switch (input)
			{
				case '1':
					printf("Heu seleccionat l'opció 1\n");	
					ImprimirMenu();                             
					break;
				case '2':
					printf("Heu seleccionat l'opció 2\n");	
					ImprimirMenu();                             
					break;
				case '3':
					printf("Heu seleccionat l'opció 3\n");	
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
		}
			input = getchar();

	}
	
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
	printf("-------------------------------------------------------\n");
}

