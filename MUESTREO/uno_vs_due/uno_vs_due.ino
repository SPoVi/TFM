// VELOCIDAD DE MUESTREO UNO VS DUE
// https://forum.arduino.cc/t/velocidad-de-muestreo-uno-vs-due/250605

/* He creado otro sketch, en este, recojo en una array varias lecturas y mando la array por comunicación serial. 
 * Antes lo que hacia era mandar cada medida. 
 */

#define mysize 360

unsigned long tStart;
unsigned long tEnd;
byte data[mysize];
//byte time[mysize];
const int Pin = A0;



void setup() {
Serial.begin(115200);
} 

void loop() {
  tStart=micros();             // Inicio del tiempo para coger muestras. OJO MICROSEGUNDOS 10^-6
  for (int i=0; i<mysize;i++){
     data[i]=analogRead(Pin);  // recogo 'mysize' numero muestras
     //time[i]=micros();       
  tEnd=micros();              // Fin del tiempo para coger muestras
  }
  
  // Mostrar por pantalla
  Serial.println();
  Serial.println("NEW ACQUISITION");
  Serial.print("Inicio del tiempo (tStart) = ");
  Serial.println(tStart);  // Incio del tiempo
  Serial.print("Fin del tiempo (tEnd) = ");
  Serial.println(tEnd);    // Fin del tiempo
  Serial.print("Tarda (tEnd-tStart) = ");
  Serial.println(tEnd-tStart); // Lo que ha tardado en recoger la informacion 
  Serial.print("Tamaño del vector de datos = ");
  Serial.println(mysize);     // Tamaño de la muestra
  Serial.println("Datos enviados: ");

  // Mostrar datos del vector enviados
  for (int i=0; i<mysize;i++){
    Serial.println(data[i],DEC); 
  }
  delayMicroseconds(10);
}

/*
 * COMENTARIOS
 * 
 * Tarda unos 300975 microsegundos es decir unos 0.3 s en capturar 360 muestras
 * 
 * 1 microsegundo / muestra = 1 Mhz
 */
