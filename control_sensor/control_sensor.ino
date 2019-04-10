#define DEBUG(a, b) for (int index = 0; index < b; index++) Serial.print(a[index]); Serial.println();

#define MIDA 100

const int AnalogIn = A0;


char mensaje[MIDA];
 
float mostres[10]; //Es guarden totes les mostres de la entrada analogica per a fer la mitja de 5 mostres anteriors  
int tmostreig;  //Temps de mostratge
float temperatura;
int MarxaParo;
int i=0;
int j=0;
int auxiliar=0; 




void setup()
{
    analogReference(INTERNAL);
    //pinMode(AnalogIn,INPUT);

    // initialize timer1 
    noInterrupts();           // disable all interrupts
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1  = 0;
    OCR1A = 31250;            // compare match register 16MHz/256/2Hz
    TCCR1B |= (1 << WGM12);   // CTC mode
    TCCR1B |= (1 << CS12);    // 256 prescaler 
    TIMSK1 |= (1 << OCIE1A);  // enable timer compare interrupt
    interrupts();             // enable all interrupts
    Serial.begin(9600);    // abre el Puerto serie
}

ISR(TIMER1_COMPA_vect)          // timer compare interrupt service 
{

int len;
int cont=0;

   if (j>MIDA) j=0;
   if (MarxaParo==1) i=i+1;
 
                               auxiliar=630;   //canviar aquesta per AnalogIn-----------------------------------------------analogRead(AnalogIn)
  
   if ((i==tmostreig) && (MarxaParo==1)) {
     
      mostres[j] = map(analogRead(AnalogIn), 0, 1023, 0.0, 110.0);
      j=j+1;
      temperatura = mostres[j-1];
      for (len=j-1,cont=1;len>j-6,len>=0;len--,cont++){
        temperatura = temperatura + mostres[len];
      }
      
      temperatura=temperatura/cont;
    
      i=0;
    }          
    
  }

void loop(){
  
int BusError;
bool Error=0;
int sortida=0;
bool ValorSortida=0;
int entrada=0;
bool ValorEntrada=0;
int tempC=0;
  
  if (Serial.available()>0){

      size_t count = Serial.readBytesUntil('\n', mensaje, MIDA);
      DEBUG(mensaje, count);

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
