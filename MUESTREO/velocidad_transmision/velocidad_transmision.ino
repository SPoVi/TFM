// Codigo de pruebas de velocidades de transmision
// http://panamahitek.com/la-velocidad-de-la-comunicacion-serial-en-arduino/

  
int amplitud = 1000; //Amplitud de la onda senoidal
double w = 2 * 0.2 * 3.141593; //Frecuencia angular
void setup() {
  Serial.begin(9600); //Se inicia comunicacion serial
}
void loop() {
  //Mientras millis() sea mayor a 5000 milisegundos
  while (millis() >= 5000) {
    //Mientras millis() sea menor a 10000 segundos
    while (millis() <= 10000) {
      //Funcion millis transformada a tiempo
      double t = millis() * 0.001;
      //Calculo del valor puntual de la onda senoidal
      double num = amplitud * sin(w * t);
      //Se imprime el valor calculado
      Serial.println(num);
    }
  }
}
