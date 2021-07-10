/*
 * Nano IOT 33 - analogRead() is Slower than expected
 * 
 * https://forum.arduino.cc/t/nano-iot-33-analogread-is-slower-than-expected/635209
 */

# define _SAMPLE_COUNT 360

void setup()
{
pinMode(A7, INPUT); //I’ve tried all of the audio input pins, btw
}

void loop()
{
float t0, t;

t0 = micros() ;
for( int i =0; i <_SAMPLE_COUNT; i++)
{
analogRead (A7);
}

t = micros() - t0;

Serial.print("Tiempo por muestra: ");
Serial.print((float)t/_SAMPLE_COUNT);
Serial.println(" us"); // microsegundos
Serial.print("Frequencia: ");
Serial.print((float)_SAMPLE_COUNT*1000000/t); // Multiplica por 10^6 
Serial.print(" Hz");
Serial.println();

delay( 2000 ); // 2 segundos

}
