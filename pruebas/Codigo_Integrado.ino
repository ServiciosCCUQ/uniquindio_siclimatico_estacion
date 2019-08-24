// Incluimos librería
#include <DHT.h>
#include "RTClib.h"
#include <Wire.h>

//Pin de conexion del Pluviometro
#define RainPin 3 
// Definimos el pin digital donde se conecta el sensor
#define DHTPIN 2
// Dependiendo del tipo de sensor
#define DHTTYPE DHT11
// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

const int humedadSensor = A0;  // Soil moisture sensor analog pin output at pin A0 of Arduino
int valorHumedad;

// ------------------------------------------------ Pluviometro
bool bucketPositionA = false;             // one of the two positions of tipping-bucket               
const double bucketAmount = 5.73;
double lluviaDiaria = 0.0;                   // rain accumulated for the day
double lluviaXMin = 0.0;                  // rain accumulated for one hour
double lluviaUltimoMinuto = 0.0;     // rain accumulated for the day till the last hour          
bool primerMomento;                               // as we want readings of the (MHz) loops only at the 0th moment 

RTC_Millis rtc;                           // software RTC time

void setup() {
  // Inicializamos comunicación serie
  Serial.begin(115600);
  rtc.begin(DateTime(__DATE__, __TIME__));       // start the RTC
  // Comenzamos el sensor DHT
  dht.begin();
  pinMode(RainPin, INPUT);                       // set the Rain Pin as input.
  //Serial.println("Inicializando Sensores ...");
}

void loop() {
  DateTime now = rtc.now();
  pluviometro(now);
  
   if(now.second() == 0){
      //HumedadAmbiente|TemperaturaAmbiente
      String dht11 = temperaturaDHT11();  
      //Se expresa en %
      String humedadTierra = humedadYL69();
      String lluvia = String(lluviaXMin,2);
      String data = dht11 + "|" + humedadTierra + "|" + lluvia;
      
      //Serial.println(dht11);
      //Serial.println(humedadTierra);  
      //Serial.print("LLuvia! ");
      //Serial.println(lluviaXMin);
  
      //Serial.print("Data ");
      Serial.println(data);
      


      delay(2000); 
   }  
}

String temperaturaDHT11() {
  String res;
  // Esperamos 5 segundos entre medidas
  //delay(5000);

  // Leemos la humedad relativa
  float h = dht.readHumidity();
  // Leemos la temperatura en grados centígrados (por defecto)
  float t = dht.readTemperature();
  // Leemos la temperatura en grados Fahreheit
  float f = dht.readTemperature(true);

  // Comprobamos si ha habido algún error en la lectura
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Error obteniendo los datos del sensor DHT11");
    return "";
  }

  // Calcular el índice de calor en Fahreheit
  float hif = dht.computeHeatIndex(f, h);
  // Calcular el índice de calor en grados centígrados
  float hic = dht.computeHeatIndex(t, h, false);

/*
  Serial.print("Humedad***: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperatura: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Índice de calor: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");
*/

  
  res += String(h,2);
  res += "|";
  res += String(t);

  return res;
  
}

String humedadYL69() {
  // When the plant is watered well the sensor will read a value 380~400, I will keep the 400
  // value but if you want you can change it below.
  valorHumedad = analogRead(humedadSensor);  // Read analog value
  valorHumedad = constrain(valorHumedad, 400, 1023); // Keep the ranges!
  valorHumedad = map(valorHumedad, 400, 1023, 100, 0); // Map value : 400 will be 100 and 1023 will be 0
  //Serial.print("Soil humidity: ");
  //Serial.print(valorHumedad);
  //Serial.println("%");
  delay(1000);  // Read every 1 sec
  
  return String(valorHumedad);
}


void pluviometro(DateTime now){
   // ++++++++++++++++++++++++ Count the bucket tips ++++++++++++++++++++++++++++++++
  if ((bucketPositionA==false)&&(digitalRead(RainPin)==HIGH)){
    bucketPositionA=true;
    lluviaDiaria+=bucketAmount;                               // update the daily rain
  }
  
  if ((bucketPositionA==true)&&(digitalRead(RainPin)==LOW)){
    bucketPositionA=false;  
  } 
  // +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
  
  if(now.second() != 0) primerMomento = true;                     // after the primerMomento minute is over, be ready for next read
  
  if(now.second() == 0 && primerMomento == true){
 
    lluviaXMin = lluviaDiaria - lluviaUltimoMinuto;      // calculate the last hour's rain
    lluviaUltimoMinuto = lluviaDiaria;                   // update the rain till last hour for next calculation
    
    // facny display for humans to comprehend

   /*
    Serial.println();
    Serial.print(now.hour());
    Serial.print(":");
    Serial.print(now.minute());
    Serial.print(":  Total Rain for the day = ");
    Serial.print(lluviaDiaria,8);                            // the '8' ensures the required accuracy
    Serial.println(" inches");
    Serial.println();
    Serial.println(" cc");
    Serial.println();
    */

    
    primerMomento = false;                                        // execute calculations only once per hour

  }
  
  if(now.second()== 0) {
    lluviaDiaria = 0.0;                                      // clear daily-rain at midnight
    lluviaUltimoMinuto = 0.0;                        // we do not want negative rain at 01:00
  }
   
}
