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
int suma;                 /* Per a fer la mitja de les 5 ultimes adquisicions*/


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
 
// PROGRAMA -------------------------------------------------------------------------------------------------------- 
 
auxiliar=auxiliar+5;          //PER A FER PROVES canviar aquesta per AnalogIn-----------------------analogRead(AnalogIn)
 
   if (j>MIDA) j=0;           /* El punter de l'array de mostres <<j>> passa a valdre 0 sempre que sigui superior a la mida del array.*/
 
   if (MarxaParo==1) i=i+1;   /* Només incrementem la <<i>> si la adquisició de lectures es troba en Marxa ja que aquesta és la condició 
                                 per a contar temps. Si no incrementessim la <<i>> sota aquesta condició el temps aniria contant de tal 
                                 forma que si passem de <<MarxaParo=0>> a <<MarxaParo=1>> la <<i>> en aquest moment podria valdre qualsevol
                                 valor diferent de 0 i per tant no estariem contant el temps de mostreig que s'ens solicita en la primera
                                 mostra de la adquisisció. */  
 
 
 // Adquisisció de mostres i mitjana de les 5 ùltimes.
  
   if ((i==tmostreig) && (MarxaParo==1)) {                          /* Es realitza la adquisició si i==tmostreig i seleccionem marxa d'adquisisció.*/
     
      mostres[j] = map(analogRead(AnalogIn), 0, 1023, 0.0, 110.0);  /* Es van guardan els valors de la entrada en format temperatura */
     
     switch (j){

      case 0:
      
        suma=mostres[j];
        temperatura=suma; 

      break;

      case 1:
     
        suma=mostres[j]+mostres[j-1];
        temperatura=suma/(j+1); 

      break;

      case 2:
     
        suma=mostres[j]+mostres[j-1]+mostres[j-2];
        temperatura=suma/(j+1); 

      break;

      case 3:
      
        suma=mostres[j]+mostres[j-1]+mostres[j-2]+mostres[j-3];
        temperatura=suma/(j+1); 

      break;

      case 4:
      
        suma=mostres[j]+mostres[j-1]+mostres[j-2]+mostres[j-3]+mostres[j-4];
        temperatura=suma/(j+1); 
      
      break;

      default:
      
        suma=mostres[j]+mostres[j-1]+mostres[j-2]+mostres[j-3]+mostres[j-4];
        temperatura=suma/5; 
         
      break;

      }
      j=j+1; 

      i=0;
   }      
}



//        ****************************************** ERROR DE PROTOCOL ***********************************************

int ProtocolError (int count, int NumeroChar){
  
/* És comú per a tots els tipus de missatges.
   Es comprova que longitud del missatge sigui la correcte, que la "A" es trobi en la primera posició, la "Z" en la última i que la "A" 
   no es repeteixi en cap altre posició del missatge.
    mensaje [0] == A
    mensaje [count] == Z
    mensaje [1...count] != A 
  */  

int BusError;           /* Apunta a cada posició del array del missatge en el bucle For per a comprobar si hi ha error de protocol.*/
bool Error=0;           /* Si hi ha error de protocol dins del bucle For <<Error=1>>. */  
  
   
   if ((count== NumeroChar ) && (mensaje[0]=='A') && (mensaje[count-1]=='Z')){               /*--> En aquest primer if es comproba que
                                                                                                   el mistage tingui el numero de bytes
                                                                                                   esperats, que la "A" es trobi en la primera 
                                                                                                   posició i que la "Z" es trobi en la última.*/
           
          for (BusError=1;BusError<=count;BusError++)           /* --> En aquest "for" es comproba que la "A" no es repeteixi en cap pasició del missatge*/      
          {         
           if (mensaje[BusError]=='A') Error=1;                 /* --> Si alguna posició del array és igual a "A" la variable <<Error=1>>.*/
           }
    
           if ( Error==1) 
            return 1;                                           /* --> ERROR DE PROTOCOL en el cas de que es repeteixi la "A" en alguna posició del missatge.*/
                                                                       
                                           
                 else                                              
                   return 0;                                   /* --> MISSATGE OKEY. No hi ha error de protocol. */     
                  
                   
                 
         } else if ( Error==0) 
             return 1;                                         /* --> ERROR DE PROTOCOL. Si no es compleixen les condicions del primer if i no s'ha enviat*/
                                                               /*      un missatge d'error de protocol perque s'hagi repetit la "A" en alguna posició del*/
                                                               /*     missatge (Error==0). */                                                                                                                                 
        
        Error=0;                                               /* --> Es posa <<Error=0>> per a evitar que al entrar a un altre cas o cuan es rebi un altre*/
                                                               /*      missatge salti un altre cop l'error de protocol.*/
   
}




//           **************************************** ERROR DE PARAMETRES *************************************

int ParametresError (char TipoMensaje){
  
  switch (TipoMensaje){

    
    
    // ERROR DE PARAMENTRES CAS M:
    /*   
         mensaje [2] == MarxaParo             (Si el missatge solicita aturar la adquisició de mostres i ja esta aturada o viceversa)
         mensaje [2] > 1                      (Si el digit és superior a 1 ja que s'espera una entrada booleana 1-Marxa 0-Aturar) 
         mensaje [3] > 2                      (Si el digit és superior a 2 ja que s'espera un temps de mostreig de 1 a 20 s)
         mensaje [3] <=1 i mensaje [4] > 9    (És una condició redundant ja que un digit no podra ser superior a 9)
         mensaje [3] >=2 i mensaje [4] > 0    (Sempre que el primer digit valgui 2 el segon digit te que valdre 0)
   */  
     case 'M':

        if (((mensaje[2]== '0') && (MarxaParo==0)) || ((mensaje[2]== '1') && (MarxaParo==1)) || ((mensaje[2]-'0') > 1) || ((mensaje[3]-'0') > 2) || (((mensaje[3]-'0') <= 1) && ((mensaje[4]-'0') > 9)) || (((mensaje[3]-'0') >= 2) && ((mensaje[4]-'0') > 0 )))

          return 1;

        else return 0;

     break;


    
   // ERROR DE PARAMENTRES CAS S:
   /*    
         mensaje [2] > 1                      (Si el digit de decenes és superior a 1 ja que l'arduino nomes disposa de 13 sorties possibles)
         mensaje [4] > 1                      (Si el digit és superior a 1 ja que s'espera una entrada booleana 1-Activar 0-Desactivar)
         mensaje [2] == 0 i mensaje [3] > 9   (És una condició redundant ja que un digit no podra ser superior a 9) 
         mensaje [2] >= 1 i mensaje [3] > 3   (Si el primer digit és 1 el segon digit te que ser inferior a 4, el nombre max de sorties és 13) 
   */       
     case 'S': 

         if (((mensaje[2]-'0') > 1) || ((mensaje[4]-'0') > 1) || (((mensaje[2]-'0') ==0) && ((mensaje[3]-'0') > 9)) || (((mensaje[2]-'0') >= 1) && ((mensaje[3]-'0') > 3 )))     

           return 1;

         else return 0;

     break;
     

   
   // ERROR DE PARAMENTRES CAS E:
   /*    
         mensaje [2] > 1                      (Si el digit de decenes és superior a 1 ja que l'arduino nomes disposa de 13 entrades possibles)
         mensaje [2] == 0 i mensaje [3] > 9   (És una condició redundant ja que un digit no podra ser superior a 9) 
         mensaje [2] >= 1 i mensaje [3] > 3   (Si el primer digit és 1 el segon digit te que ser inferior a 4, el nombre max de entrades és 13) 
   */       
     case 'E':  

        if (((mensaje[2]-'0') > 1) || (((mensaje[2]-'0') ==0) && ((mensaje[3]-'0') > 9)) || (((mensaje[2]-'0') >= 1) && ((mensaje[3]-'0') > 3 )))
          
          return 1;

        else return 0;
          
     break;
     

   
   // ERROR DE PARAMENTRES CAS C:
   /*      
          MarxaParo == 0                       (Si la adquisició de mostres de temperatura no es troba en marxa no es pot donar valor de temperatura)    
   */       
     case 'C':
         
        if (MarxaParo==0)

         return 1;

        else return 0;

        break;
   }
}



//           *********************************** LECTURA DE MISSATGES DEL PORT SERIE *********************************

void loop(){
  
 
// Variables Locals ------------------------------------------------------------------------------------------------- 

int sortida=0;          /* EN EL CAS S       -> Es guarda el numero de sortida que es vol activar o desactivar llegit del port serie. */
bool ValorSortida=0;    /* EN EL CAS S       -> Es guarda el valor digital que te que agafar la sortida en funció del missatge rebut del port serie. */
int entrada=0;          /* EN EL CAS E       -> Es guarda el numero de entrada que el port serie solicita llegir del arduino.  */
bool ValorEntrada=0;    /* EN EL CAS E       -> Es guarda el valor digital que te la entrada per a enviar-lo al port serie. */
int tempC=0;            /* EN EL CAS C       -> Es guarda el valor de la variable temperatura en valor digital (rang 0 a 1023) per a enviar-lo pel port serie */ 


 
// PROGRAMA ---------------------------------------------------------------------------------------------------------
 
  if (Serial.available()>0){  /*Si el valor de retorn de la funció Serial.availables és > 0 vol dir que s'esta rebent un misatge.
                                Per tant només es realitzara la lectura del port serie quan es detecti que hi ha un missatge nou.*/

      size_t count = Serial.readBytesUntil('\n', mensaje, MIDA); /*LECTURA DEL MISSATGE. Es guarda el misatge en el array mensaje i 
                                                                   la cuantitat de caracter que te aquest en la variable count */                                                           
      //DEBUG(mensaje, count);
   
    
   switch (mensaje[1]){

// TIPUS DE MISSATGE
    /*  En el byte 1 del array de mensaje (mensaje [1]) es llegeix quin tipus de missatge es rep i en funció d'aixó es selecciona un cas o un altre
        mensaje [1] == M -> Marxa o aturada d'adquisisció de mostres i temps d'adquisició.
         mensaje [1] == S -> Seleccionar i activar sortida. 
         mensaje [1] == E -> Seleccionar i llegir entrada.
         mensaje [1] == C -> Convertidor analogic digital.
    */
        
   
     
// CAS M: Marxa o aturada d'adquisisció de mostres i temps d'adquisició.
  /* 
     mensaje [2]: Marxa "==1" o atura "==0" d'enregistrament de temperatures.
     mensaje [3]: digit de desenes de temps de mostreig.
     mensaje [4]: digit de unitats de temps de mostreig.
    
     MarxaParo = mensaje [2]
     tmostreig = (mensaje [3] i mensaje [4]) * 2   -> Es multiplica per dos perque la "i" de la interrupció del temporitzador 1 conta cada sego/2.
   */  
       case 'M':  

            if  (ProtocolError(count,6) == 1)             Serial.print("AM1Z\n");                                 /* Error protocol */                    
           
            else if (ParametresError(mensaje[1]) == 1)      Serial.print("AM2Z\n");                               /* Error de paramentres */
              
                                 
                 else  
                 {                          
                   MarxaParo=mensaje[2]-'0';                             /* SALVAR VALOR DE parada o marxa EN VARIABLE GLOVAL PER A TRACTAR EN LA INTERRUPCIÓ        (Es converteix un Char en Int restant el valor ASCII de 0) */
                   tmostreig = ((mensaje[3]-'0')*10+(mensaje[4]-'0'))*2; /* CONCATENEM DOS VALORS DEL MISATGE PER A LLEGIR-LOS COM UN ENTER I ES MULTIMPLICA PER DOS (El Timer funciona a 500ms, el doble de velociat que 1 segon) */
                   Serial.print("AM0Z\n");                               /* Missatge port serie tot Okey */
                 }

       
         break;


     
// CAS S: Seleccionar i activar sortida.
  /* 
     mensaje [2]: digit de desenes de la sortida.
     mensaje [3]: digit de unitats de la sortida.
     mensaje [4]: Activar "==1" o desactivar "==0" sortida.
    
     sortida = mensaje [2] i mensaje [3]
     ValorSortida = mensaje [4]
  */
        case 'S':    

            if  (ProtocolError(count,6) == 1)           Serial.print("AS1Z\n");                           /* Error protocol */
                                                                        
            else if (ParametresError(mensaje[1])==1)    Serial.print("AS2Z\n");                           /* Error de paramentres */
                
                                 
                 else  
                 {                          
                 
                   sortida = (mensaje[2]-'0')*10+(mensaje[3]-'0');        /* CONCATENEM DOS VALORS DEL MISATGE PER A LLEGIR-LOS COM UN ENTER */
                   ValorSortida=(mensaje[4]-'0');                         /* ES GUARDA EL VALOR DE LA SORTIDA COM UN ENTER */
                   pinMode(sortida, OUTPUT);                              /* ES CONFIGURA EL PIN SELECCIONAT COM A SORTIDA */
                   digitalWrite(sortida, ValorSortida);                   /* S'ESCRIU EL VALOR A LA SORTIDA  */
                   Serial.print("AS0Z\n");                                /* Missatge port serie tot Okey */
                 
                   
                 }
       
        break;
     
     
// CAS E: Seleccionar i llegir entrada.
  /* 
     mensaje [2]: digit de desenes de la entrada.
     mensaje [3]: digit de unitats de la entrada.
    
     entrada = mensaje [2] i mensaje [3]
     ValorEntrada: Es guarda el valor digital que te la entrada per a enviar-lo al port serie.
  */           
        case 'E':
         
            if  (ProtocolError(count,5) == 1)          Serial.print("AE1Z\n");                           /* Error protocol */
                                                                        
            else if (ParametresError(mensaje[1])==1)   Serial.print("AE2Z\n");                           /* Error de paramentres */
                
                                 
                 else  
                 {                          
                 
                   entrada = (mensaje[2]-'0')*10+(mensaje[3]-'0');        /* CONCATENEM DOS VALORS DEL MISATGE PER A LLEGIR-LOS COM UN ENTER */                 
                   pinMode(entrada, INPUT);                               /* ES CONFIGURA EL PIN SELECCIONAT COM A ENTRADA */
                   ValorEntrada=digitalRead(entrada);                     /* ES LLEGEIX EL VALOR DE LA ENTRADA I S'ESCRIU EN LA VARIABLE "ValoEntrada" */
                   
                   Serial.print("AE0");                                   /* Missatge de tot okey (el 4 byte del missatge conte el valor de la entrada) */
                   Serial.print(ValorEntrada);
                   Serial.print("Z\n");
                 
                   
                 }
        
        break;
        

// CAS C: Convertidor analogic digital.
  /* 
     temperatura: Valor de la ùltima mostra de temperatura amb la mitja de les 5 anteriors en [ºC]    
     tempC:       Valor de la ùltima mostra de temperatura amb la mitja de les 5 anteriors en format [0..1023].
     MarxaParo:   DEL CAS M -> Marxa "==1" o atura "==0" d'enregistrament de temperatures.
  */           
        case 'C':
         

            if  (ProtocolError(count,3) == 1)          Serial.print("AC1Z\n");                          /* Error de protocol */
                                                                        
            else if (ParametresError(mensaje[1])==1)   Serial.print("AC2Z\n");                          /* Error de paramentres */
                                                                                  
                 else  
                 { 
                       tempC = map(temperatura, 0.0, 110.0, 0, 1023);      /* ES GUARDA EL VALOR DE LA TEMPERATURA EN "tempC" en format [0..1023] */
                      
                       Serial.print("AC0");                                /* Missatge de tot okey (el 4 byte del missatge conte el valor de la tempC) */
                       Serial.print(tempC);  
                       Serial.print("Z\n");              
                              
                 }
       
       
       break;
     
    }
  }  
}
