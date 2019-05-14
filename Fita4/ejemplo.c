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

int i;
int w=0;
void* codi_fill(void* parametre){ // codi thread fill
   	while(i>0){
  		printf("Thread fill PID(%d) i=%d\n",getpid(), i);
  		i--;
  		printf("\n %d", w);
  		sleep(2); // Retard de 2 segons

  	}
    pthread_exit(NULL);
    return NULL;
}


int main(int argc, char *argv[])
{
	pthread_t thread;
	
	i=5;

  printf("Proces pare 1 PID(%d) \n",getpid() );

  pthread_create(&thread, NULL, codi_fill, 0); //Es crea el thread fill

  printf("Proces pare 2 PID(%d) \n",getpid() );// Proces Pare
  	
 	while(i<10){
  		printf("Proces pare PID(%d) i=%d\n",getpid(), i);
  		i++;
  		if (i==7){
		w=9;
		}
  		sleep(1); // Retard de 1 segons
  	}

  pthread_join(thread, NULL);

 	printf("Proces pare PID(%d)\n",getpid());

  return EXIT_SUCCESS;
}
