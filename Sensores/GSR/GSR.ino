const int GSR=A0;

int sensorValue=0;

int gsr_average=0;


int imedidas = 10;  // Numero de medidas que hace para hacer la media

void setup(){

  Serial.begin(9600);

}

 

void loop(){

  long sum=0;

  for(int i=0;i<imedidas;i++)           //Average the 10 measurements to remove the glitch

      {

      sensorValue=analogRead(GSR);

      sum += sensorValue;

      delay(5);

      }

   gsr_average = sum/imedidas;

   Serial.println(gsr_average);

}
