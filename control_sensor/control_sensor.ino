#define DEBUG(a, b) for (int index = 0; index < b; index++) Serial.print(a[index]); Serial.println();

#define MIDA 100          /*Es deifineix la mida de tots els Arrays del programa*/



//           **************************************** VARIABLES GLOBALS **********************************************

const int AnalogIn = A0;  /* Es declara el nom de la entrada analògica A0                                           */


// Variable que es troba en el EL VOID LOOP --------------------------------------------------------------------------

char mensaje[MIDA];       /* Es declara l'array de caracters en el que es preten guardar el misstage del port serie */


// Variables que es troben en el EL VOID LOOP i en ISR(TIMER1_COMPA_vect) --------------------------------------------

int tmostreig;            /* Es guarda l'interval de temps entre mostres que es solicita pel port serie. [Unitats: Seg * 2]  */ 
float temperatura;        /* Es guarda la mitja de les 5 últimes mostres de temperatura. [Unitats: T]                        */ 
int MarxaParo;            /* Es guarda la solicitud de marxa o aturada d'adquisició de mostres.                              */



// Variables que es troben en el ISR(TIMER1_COMPA_vect) --------------------------------------------------------------

float mostres[MIDA];      /* Es guarden totes les mostres de la entrada analogica per a fer la mitja de 5 mostres anteriors. [Unitats: T]                       */
int i=0;                  /* Incrementa cada cop que entra en una interrupció i inicialitza cada cop que s'adquireix una mostra.                                */
int j=0;                  /* Te la funció de punter en l'array de mostres (incrementa una posició cada cop que es guarda una mostra i s'inicialitza si j>MIDA). */
int auxiliar=0;           /* CAP UTILITAT només realització de probes en substitució de la entrada analògica.                                                   */



//           *********************************************** SETUP ***************************************************

void setup()
{ 
 
 /*Funció que permet utilitzar la tensió interna de l'arduino com a tenció de referencia per a entrada analogica 
  (per tant el rang de la entrada analogica és de 0 a 1,1V) */
   
    analogReference(INTERNAL);
                              

 // S'INICIALITZA EL TIMER 1
    noInterrupts();           /* disable all interrupts                                                        */
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    OCR1A = 31250;            /* compare match register 16MHz/256/2Hz (SALTA LA INTERRUPCIÓ CADA 500ms)        */
    TCCR1B |= (1 << WGM12);   /* CTC mode                                                                      */
    TCCR1B |= (1 << CS12);    /* 256 prescaler                                                                 */
    TIMSK1 |= (1 << OCIE1A);  /* enable timer compare interrupt                                                */
    interrupts();             /* enable all interrupts                                                         */
    Serial.begin(9600);       /* abre el Puerto serie                                                          */
}



//           ************************************** INTERRUPCIÓ DEL TIMER 1 *****************************************

ISR(TIMER1_COMPA_vect)          
{
 
// Variables Locals ------------------------------------------------------------------------------------------------- 

int suma=0;                   /* Per a fer la mitja de les 5 ultimes adquisicións*/

int len;                      /* S'utilitza en el bucle For. Per a realitzar la suma de les 5 ùltimes posicions del vecteor mostres */
 
int cont=0;                   /* S'utilitza en el bucle For. Per a contar el numero de iteracions i després dividir la suma de les
                                 ùltimes mostres */


// PROGRAMA -------------------------------------------------------------------------------------------------------- 
auxiliar=630;                 //PER A FER PROVES canviar aquesta per AnalogIn-----------------------analogRead(AnalogIn)
 
   if (j>MIDA) j=0;           /* El punter de l'array de mostres <<j>> passa a valdre 0 sempre que sigui superior a la mida del array.
 
   if (MarxaParo==1) i=i+1;   /* Només incrementem la <<i>> si la adquisició de lectures es troba en Marxa ja que aquesta és la condició 
                                 per a contar temps. Si no incrementessim la <<i>> sota aquesta condició el temps aniria contant de tal 
                                 forma que si passem de <<MarxaParo=0>> a <<MarxaParo=1>> la <<i>> en aquest moment podria valdre qualsevol
                                 valor diferent de 0 i per tant no estariem contant el temps de mostreig que s'ens solicita en la primera
                                 mostra de la adquisisció. */  
 
 
 // Adquisisció de mostres i mitjana de les 5 ùltimes.
  
   if ((i==tmostreig) && (MarxaParo==1)) {                          /* Es realitza la adquisició si i==tmostreig i seleccionem marxa d'adquisisció.*/
     
      mostres[j] = map(analogRead(AnalogIn), 0, 1023, 0.0, 110.0);  /* Es van guardan els valors de la entrada en format temperatura */
      j=j+1;                                                        /* S'incrementa el valor del punter <<j>>, la propera adquisició es guarada en j+1 */
     
      for (len=j-1,cont=1;len>j-6,len>=0;len--,cont++){             /* Bucle que realitza la suma de les 5 últimes adquisicions */
        suma = suma + mostres[len];
      }
      
      temperatura=suma/cont;                                        /* Es guarda la mitja de les mostres a la variable <<temperatura>> */
    
      i=0;                                                          /* Cada cop que es realitza una adquisició es posa el contador de temps <<i>> a 0. */
    }          
    
  }



//           *********************************** LECTURA DE MISSATGES DEL PORT SERIE *********************************

void loop(){
 
// Variables Locals ------------------------------------------------------------------------------------------------- 

int BusError;
bool Error=0;
int sortida=0;
bool ValorSortida=0;
int entrada=0;
bool ValorEntrada=0;
int tempC=0;
 
 
// PROGRAMA ---------------------------------------------------------------------------------------------------------
 
  if (Serial.available()>0){  /*Si el valor de retorn de la funció Serial.availables és > 0 vol dir que s'esta rebent un misatge.
                                Per tant només es realitzara la lectura del port serie quan es detecti que hi ha un missatge nou.*/

      size_t count = Serial.readBytesUntil('\n', mensaje, MIDA); /*LECTURA DEL MISSATGE. Es guarda el misatge en el array mensaje i 
                                                                   la cuantitat de caracter que te aquest en la variable count */                                                           
      DEBUG(mensaje, count);
   
   
// CONDICIONS PER A CADA TIPUS DE MISSATGE
      switch (mensaje[1]){
      
        case 'M':
         if ((count==6) && (mensaje[0]=='A') && (mensaje[count-1]=='Z'))
         {
          for (BusError=1;BusError<=count;BusError++) 
          {  
            if (mensaje[BusError]=='A') Error=1;      
          }
    
            if ( Error==1) Serial.print("AM1Z\n");
                                                                        
            else if (((mensaje[2]== '0') && (MarxaParo==0)) || ((mensaje[2]== '1') && (MarxaParo==1)) || ((mensaje[2]-'0') > 1) || ((mensaje[3]-'0') > 2) || (((mensaje[3]-'0') <= 1) && ((mensaje[4]-'0') > 9)) || (((mensaje[3]-'0') >= 2) && ((mensaje[4]-'0') > 0 )))
                Serial.print("AM2Z\n");                                    //Missatge d'error en el cas de no cumplir les condicions d'error de paramentres
                                 
                 else  
                 {                          
                   MarxaParo=mensaje[2]-'0';                             //SALVAR VALOR DE parada o marxa EN VARIABLE GLOVAL PER A TRACTAR EN LA INTERRUPCIÓ        (Es converteix un Char en Int restant el valor ASCII de 0)
                   tmostreig = ((mensaje[3]-'0')*10+(mensaje[4]-'0'))*2; //CONCATENEM DOS VALORS DEL MISATGE PER A LLEGIR-LOS COM UN ENTER I ES MULTIMPLICA PER DOS (El Timer funciona a 500ms, el doble de velociat que 1 segon) 
                   Serial.print("AM0Z\n");
                   
                 }
         } else if ( Error==0) Serial.print("AM1Z\n");                    //Missatge d'error en el cas de no cumplir les condicions d'error de protocol
        
         Error=0;
       
         break;

      

        case 'S':
         if ((count==6) && (mensaje[0]=='A') && (mensaje[count-1]=='Z'))
         {
          for (BusError=1;BusError<=count;BusError++) 
          {  
            if (mensaje[BusError]=='A') Error=1;      
          }
    
            if ( Error==1) Serial.print("AS1Z\n");
                                                                        
            else if (((mensaje[2]-'0') > 1) || ((mensaje[4]-'0') > 1) || (((mensaje[2]-'0') ==0) && ((mensaje[3]-'0') > 9)) || (((mensaje[2]-'0') >= 1) && ((mensaje[3]-'0') > 3 )))
                Serial.print("AS2Z\n");                                    //Missatge d'error en el cas de no cumplir les condicions d'error de paramentres
                                 
                 else  
                 {                          
                 
                   sortida = (mensaje[2]-'0')*10+(mensaje[3]-'0');
                   ValorSortida=(mensaje[4]-'0');
                   pinMode(sortida, OUTPUT);
                   digitalWrite(sortida, ValorSortida);
                   Serial.print("AS0Z\n");
                 
                   
                 }
        } else if ( Error==0) Serial.print("AS1Z\n");                    //Missatge d'error en el cas de no cumplir les condicions d'error de protocol
        
        Error=0;
        break;

        case 'E':
         if ((count==5) && (mensaje[0]=='A') && (mensaje[count-1]=='Z'))
         {
          for (BusError=1;BusError<=count;BusError++) 
          {  
            if (mensaje[BusError]=='A') Error=1;      
          }
    
            if ( Error==1) Serial.print("AE1Z\n");
                                                                        
            else if (((mensaje[2]-'0') > 1) || (((mensaje[2]-'0') ==0) && ((mensaje[3]-'0') > 9)) || (((mensaje[2]-'0') >= 1) && ((mensaje[3]-'0') > 3 )))
                Serial.print("AE2Z\n");                                    //Missatge d'error en el cas de no cumplir les condicions d'error de paramentres
                                 
                 else  
                 {                          
                 
                   entrada = (mensaje[2]-'0')*10+(mensaje[3]-'0');
                   
                   pinMode(entrada, INPUT);
                   ValorEntrada=digitalRead(entrada);
                   Serial.print("AE0");
                   Serial.print(ValorEntrada);
                   Serial.print("Z\n");
                 
                   
                 }
         } else if ( Error==0) Serial.print("AS1Z\n");                    //Missatge d'error en el cas de no cumplir les condicions d'error de protocol
        
        Error=0;
       
        break;
        


        case 'C':
         if ((count==3) && (mensaje[0]=='A') && (mensaje[2]=='Z'))
         {
          for (BusError=1;BusError<=count;BusError++) 
          {  
            if (mensaje[BusError]=='A') Error=1;      
          }
    
            if ( Error==1) Serial.print("AC1Z\n");
                                                                        
            else if (MarxaParo==0)
                Serial.print("AC2Z\n");                                    //Missatge d'error en el cas de no cumplir les condicions d'error de paramentres
                                 
                 else  
                 { 
                       tempC = map(temperatura, 0.0, 110.0, 0, 1023); 
                       Serial.print("AC0");       
                       Serial.print(tempC);  
                       Serial.print("Z\n");              
                              
                 }
        }else if( Error==0) Serial.print("AC1Z\n");                    //Missatge d'error en el cas de no cumplir les condicions d'error de protocol
        
        Error=0;
       
       break;
     
    }
  }
}
