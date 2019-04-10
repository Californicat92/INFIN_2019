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
void ImprimirMenu(void)
{
	printf("\n\nMenu:\n");
	printf("--------------------\n");
	printf("1: Opció 1\n");
	printf("2: Opció 2\n");
	printf("3: Opció 3\n");
	printf("s: Sortir\n");
	printf("--------------------\n");
}


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
			case 0x0a: //Això és per enviar els 0x0a (line feed) que s'envia quan li donem al Enter
				break;
			default:
				printf("Opció incorrecta\n");	
				printf("He llegit 0x%hhx \n",input);
				break;
		}

		input = getchar();

	}
	
	return 0;
}


