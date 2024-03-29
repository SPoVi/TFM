/*
 * ACELERAR FRECUENCIA DE MUESTREO MEGA
 * 
 * https://forum.arduino.cc/t/acelerar-frecuencia-de-muestreo/379557/10
 */

unsigned long start_time;
unsigned long stop_time;
unsigned long values[1000];

void setup() {        
  Serial.begin(9600);  
  ADC->ADC_MR |= 0x80;  //set free running mode on ADC
  ADC->ADC_CHER = 0x80; //enable ADC on pin A0
}

void loop() {
  unsigned int i;
    
  start_time = micros();
  for(i=0;i<1000;i++){
    while((ADC->ADC_ISR & 0x80)==0); // wait for conversion
    values[i]=ADC->ADC_CDR[7]; //get values
  }
  stop_time = micros();

  Serial.print("Total time: ");
  Serial.println(stop_time-start_time); 
  Serial.print("Average time per conversion: ");
  Serial.println((float)(stop_time-start_time)/1000);

  Serial.println("Values: ");
  for(i=0;i<1000;i++) {
    Serial.println(values[i]);
  }  
  delay(2000);
}
