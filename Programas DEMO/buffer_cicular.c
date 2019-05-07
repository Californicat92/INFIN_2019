/***************************************************************************
                          buffer_cicular.c
                             -------------------
     copyright            : (C) 2012 by A. Moreno
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#define	MIDA_BUFFER 5

typedef struct _TipusMostra{
	long int segons;
	float	temperatura;
}TipusMostra;

struct{
	TipusMostra *dades;
	int	index_entrada; //Apunta al lloc on es posarà la sagüent mostra
	int nombre_mostres; //Nombre de mostres que hi ha el el buffer circular
}buffer_circular;

void	buffer_cicular_inici(void){
	buffer_circular.dades = malloc(sizeof(TipusMostra)*MIDA_BUFFER);
	buffer_circular.index_entrada = 0;
	buffer_circular.nombre_mostres = 0;
}

void	buffer_cicular_final(void){
	free(buffer_circular.dades);
}

void	buffer_cicular_introduir(TipusMostra dada){
	buffer_circular.dades[buffer_circular.index_entrada] = dada;

	buffer_circular.index_entrada++;
	if (buffer_circular.index_entrada == MIDA_BUFFER){
		buffer_circular.index_entrada = 0; //Continuem pel principi: circular
	}

	if (buffer_circular.nombre_mostres < MIDA_BUFFER){ //Agumentar fins que estigui ple
		buffer_circular.nombre_mostres++;
	}

}

void	buffer_cicular_borrar_tot(void){
	buffer_circular.index_entrada = 0;
	buffer_circular.nombre_mostres = 0;
}

void	buffer_cicular_bolcat_dades(void){
	int i;
	TipusMostra dada;

	if (buffer_circular.nombre_mostres < MIDA_BUFFER){

		for (i=0;i<buffer_circular.nombre_mostres;i++){
			dada = buffer_circular.dades[i];
			printf("Temps: %ld Temperatura: %f\n", dada.segons, dada.temperatura);
		}
	}
	else{
		for (i=buffer_circular.index_entrada;i<MIDA_BUFFER;i++){
			dada = buffer_circular.dades[i];
			printf("Temps: %ld Temperatura: %f\n", dada.segons, dada.temperatura);
		}
		for (i=0;i<buffer_circular.index_entrada;i++){
			dada = buffer_circular.dades[i];
			printf("Temps: %ld Temperatura: %f\n", dada.segons, dada.temperatura);
		}
	}
}





int main(int argc, char *argv[]){

	TipusMostra dada;
	
	buffer_cicular_inici();

	printf("Introduim 5 dades al buffer circular\n");

	dada.segons = 1;
	dada.temperatura = 11.3;
	buffer_cicular_introduir(dada);

	dada.segons = 2;
	dada.temperatura = 12.3;
	buffer_cicular_introduir(dada);

	dada.segons = 3;
	dada.temperatura = 13.3;
	buffer_cicular_introduir(dada);

	dada.segons = 4;
	dada.temperatura = 14.3;
	buffer_cicular_introduir(dada);

	dada.segons = 5;
	dada.temperatura = 15.3;
	buffer_cicular_introduir(dada);

	printf("Fem un bocat del contingut del buffer circular\n");
	buffer_cicular_bolcat_dades();

	printf("Introduim 2 dades més al buffer circular\n");
	dada.segons = 6;
	dada.temperatura = 16.3;
	buffer_cicular_introduir(dada);

	dada.segons = 7;
	dada.temperatura = 17.3;
	buffer_cicular_introduir(dada);

	printf("Fem un bocat del contingut del buffer circular\n");
	buffer_cicular_bolcat_dades();


	printf("Borrem el buffer circular\n");
	buffer_cicular_borrar_tot();
	
	printf("Fem un bocat del contingut del buffer circular\n");
	buffer_cicular_bolcat_dades();

	buffer_cicular_final();

	return 0;
}

/* Exemple d'execució
$ gcc buffer_circular.c -o buffer_circular
$ ./buffer_circular 
Introduim 5 dades al buffer circular
Fem un bocat del contingut del buffer circular
Temps: 1 Temperatura: 11.300000
Temps: 2 Temperatura: 12.300000
Temps: 3 Temperatura: 13.300000
Temps: 4 Temperatura: 14.300000
Temps: 5 Temperatura: 15.300000
Introduim 2 dades més al buffer circular
Fem un bocat del contingut del buffer circular
Temps: 3 Temperatura: 13.300000
Temps: 4 Temperatura: 14.300000
Temps: 5 Temperatura: 15.300000
Temps: 6 Temperatura: 16.299999
Temps: 7 Temperatura: 17.299999
Borrem el buffer circular
Fem un bocat del contingut del buffer circular
*/


