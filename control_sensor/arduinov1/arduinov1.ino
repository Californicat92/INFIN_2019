#define ledPin 13
#define DEBUG(a) Serial.println(a);
char data[255];

void setup()
{
pinMode(ledPin, OUTPUT);
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
digitalWrite(ledPin, digitalRead(ledPin) ^ 1);   // toggle LED pin 
}
void loop()
{
  
if (Serial.available()>0){
  //leemos la opcion enviada
    String data = Serial.readStringUntil('\n');
      
      //Serial.print(data[1]);
    
    if(data[0]== 'A'){
      
      switch (data[1]){
        case 'M':
            if (data[2]== '0' || data[2]== '1'){
                Serial.print("AM0Z\n");
            }
            else
            {
                Serial.print("AM1Z\n");
              
            }
         case 'S':
         if (data[5]== '0'){
                Serial.print("ASnn0Z\n");
            }
            else
            {
                Serial.print("ASnn1Z\n");
              
            }
         case 'E':
         case 'C':
          }
      

}
}
}
