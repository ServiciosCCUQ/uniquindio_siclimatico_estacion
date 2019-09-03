/*
   Codigo Integrado Estacion Meteorologica SEN186
   Sensores y variables:
    Pluviómetro.
  -          Precipitación en 1 hora (mm)
  -          Precipitación en 24 horas (mm)

  Anemómetro.
  -          Velocidad media del viento por minuto (m/s)
  -          Velocidad media del viento en 5 minutos (m/s)

  DTH22.
  -          Temperatura (C)
  -          Humedad (%)

  Veleta.
  -          Dirección del viento

  Sensor BME/P 280

  -          Presión barométrica (hPa)

  Sensor CC811

  -          Sensor de Aire, VCO y CO2


   @see https://github.com/tinkerall/Tutorial/blob/master/EP16SensorMovimientoPIR/EP16SensorMovimientoPIR.ino - Configuracion PIR
   @see http://www.rei-labs.net/another-arduino-nikon-ir-remote/ - Configuracion IR Camara Nikon

*/

#include <ArduinoJson.h>
#include <Wire.h>
#include <DHT.h>
#include "RTClib.h" // Libreria Reloj
//#include <RTCLib.h>
#include <time.h>

#include "SparkFunCCS811.h" //Click here to get the library: http://librarymanager/All#SparkFun_CCS811

#define CCS811_ADDR 0x5B //Default I2C Address
//#define CCS811_ADDR 0x5A //Alternate I2C Address

// Definimos el pin digital donde se conecta el sensor
#define DHTPIN 13
// Dependiendo del tipo de sensor
#define DHTTYPE DHT11

// Inicializamos el sensor DHT11
DHT dht(DHTPIN, DHTTYPE);

CCS811 mySensor(CCS811_ADDR);

// Declaramos un RTC DS3231 (Reloj)
RTC_DS3231 rtc;
//RTC_DS1307 rtc;

char                 databuffer[35];
double               temp;

DynamicJsonDocument doc(512);

//Led Indicador de Encendido
const int ledPIN = 12;

// Informacion para la camara
//const int PIR1 = 2;  // pin 2 va a la señal del sensor de movimiento
//const int PIR2 = 4;  // pin 3 va a la señal del sensor de movimiento
const int INFRAROJO = 3;
int pir_lectura1 = 0;
int pir_lectura2 = 0;

void getBuffer()                                                                    //Get weather status data
{
  int index;
  for (index = 0; index < 35; index ++)
  {
    if (Serial.available())
    {
      databuffer[index] = Serial.read();
      if (databuffer[0] != 'c')
      {
        index = -1;
      }
    }
    else
    {
      index --;
    }
  }
}

int transCharToInt(char *_buffer, int _start, int _stop)                             //char to int）
{
  int _index;
  int result = 0;
  int num = _stop - _start + 1;
  int _temp[num];
  for (_index = _start; _index <= _stop; _index ++)
  {
    _temp[_index - _start] = _buffer[_index] - '0';
    result = 10 * result + _temp[_index - _start];
  }
  return result;
}

int WindDirection()                                                                  //Wind Direction
{
  return transCharToInt(databuffer, 1, 3);
}

float WindSpeedAverage()                                                             //air Speed (1 minute)
{
  temp = 0.44704 * transCharToInt(databuffer, 5, 7);
  return temp;
}

float WindSpeedMax()                                                                 //Max air speed (5 minutes)
{
  temp = 0.44704 * transCharToInt(databuffer, 9, 11);
  return temp;
}

float Temperature()                                                                  //Temperature ("C")
{
  temp = (transCharToInt(databuffer, 13, 15) - 32.00) * 5.00 / 9.00;
  return temp;
}

float RainfallOneHour()                                                              //Rainfall (1 hour)
{
  temp = transCharToInt(databuffer, 17, 19) * 25.40 * 0.01;
  return temp;
}

float RainfallOneDay()                                                               //Rainfall (24 hours)
{
  temp = transCharToInt(databuffer, 21, 23) * 25.40 * 0.01;
  return temp;
}

int Humidity()                                                                       //Humidity
{
  return transCharToInt(databuffer, 25, 26);
}

float BarPressure()                                                                  //Barometric Pressure
{
  temp = transCharToInt(databuffer, 28, 32);
  return temp / 10.00;
}
void ccs811() {
  //Serial.println("CCS811 Basic Example");

  Wire.begin(); //Inialize I2C Harware

  //It is recommended to check return status on .begin(), but it is not
  //required.
  CCS811Core::status returnCode = mySensor.begin();
  if (returnCode != CCS811Core::SENSOR_SUCCESS)
  {
    //Serial.println(".begin() returned with an error.");
    while (1); //Hang if there was a problem.
  }

}
void css8112() {
  //Check to see if data is ready with .dataAvailable()
  if (mySensor.dataAvailable())
  {
    //If so, have the sensor read and calculate the results.
    //Get them later
    mySensor.readAlgorithmResults();


    doc["co2"] = mySensor.getCO2();
    doc["voc"] = mySensor.getTVOC();
  }

  delay(10); //Don't spam the I2C bus
}
void setup()
{
  pinMode(ledPIN , OUTPUT);
  setupInfrarojo();

  //inicializar css8112
  ccs811();

  //inicializar reloj
  rtc.begin();

  // Comenzamos el sensor DHT
  dht.begin();

  Serial.begin(9600);


}

void setupInfrarojo() {
  //  pinMode(PIR1, INPUT);  // Configurar pir como entrada o INPUT
  //  pinMode(PIR2, INPUT);  // Configurar pir como entrada o INPUT
  pinMode(INFRAROJO, OUTPUT);
  TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2B = _BV(WGM22) | _BV(CS21);
  OCR2A = 52;
  OCR2B = 20;
}

float co2() {

  if (mySensor.dataAvailable())
  {
    mySensor.readAlgorithmResults();
    return mySensor.getCO2();
  }

  return -1;
}

float voc() {

  if (mySensor.dataAvailable())
  {
    mySensor.readAlgorithmResults();
    return mySensor.getTVOC();
  }

  return -1;
}


void data_dth11() {
  doc["dth11_humedad"] = dht.readHumidity();
  doc["dth11_temp"] = dht.readTemperature();
}


void loop()
{

  detectarLibacion();

  // Obtener fecha Actual
  DateTime now = rtc.now();

  if ((now.second() % 6) == 0 ) {
    //Manejo de Luz Indicadora de encendido
    manejoFaro();
  }
  if ( (now.minute() % 10) == 0 && now.second() == 0) {
    //if ( now.second() % 10 ==0){
    delay(5000);
    getBuffer();


    //Restar a la fecha la epoca ( 1970)
    time_t t = now.unixtime() - UNIX_OFFSET;
    const char *fecha = ctime(&t);
    doc["fecha"] = fecha;

    //JsonObject wind = doc.createNestedObject("wind");
    doc["dir"] = WindDirection();
    doc["speed1"] = WindSpeedAverage();
    doc["speed5"] = WindSpeedMax();


    //JsonObject rain = doc.createNestedObject("rain");
    doc["hour1"] = RainfallOneHour();
    doc["hour24"] = RainfallOneDay();

    doc["temp"] = Temperature();
    doc["hum"] = Humidity();
    doc["bp"] = BarPressure();

    css8112();
    data_dth11();

    serializeJson(doc, Serial);
    Serial.println("");
  }
}

void manejoFaro() {
  digitalWrite(ledPIN , HIGH);
  delay(200);
  digitalWrite(ledPIN , LOW);
}


void detectarLibacion() {
  if (Serial.available() > 0 && Serial.read() == 70) {
    //Ejecutar Orden66 (Si el raspberry le envia 'F')
    dispararCamara();
  }
}

void dispararCamara() {
  delay(60);
  pulsoInfra();
  pulsoInfra();
  delay(1000);
}

void pulsoInfra() {
  TCCR2A |= _BV(COM2B1);
  delayMicroseconds(2000);
  TCCR2A &= ~_BV(COM2B1);
  delay(28);
  TCCR2A |= _BV(COM2B1);
  delayMicroseconds(390);
  TCCR2A &= ~_BV(COM2B1);
  delayMicroseconds(1580);
  TCCR2A |= _BV(COM2B1);
  delayMicroseconds(410);
  TCCR2A &= ~_BV(COM2B1);
  delayMicroseconds(3580);
  TCCR2A |= _BV(COM2B1);
  delayMicroseconds(400);
  TCCR2A &= ~_BV(COM2B1);
  delay(63);
}





/*
  FUNCION PARA OBTENER LA FECHA EN MODO TEXTO
  Devuelve: DD-MM-AAAA HH:MM:SS
  @see http://aitormartin-apuntes.blogspot.com/2014/01/arduino-funcion-para-obtener-la-fecha.html
*/
String getFecha()
{
  char fecha[20];
  DateTime now = rtc.now();

  int dia = now.day();
  int mes = now.month();
  int anio = now.year();
  int hora = now.hour();
  int minuto = now.minute();
  int segundo = now.second();

  sprintf( fecha, "%.4d-%.2d-%.2d %.2d:%.2d:%.2d", anio, mes, dia, hora, minuto, segundo);
  return String( fecha );
}

String getFecha2(const DateTime& f)
{
  char fecha[20] = "";

  int dia = f.day();
  int mes = f.month();
  int anio = f.year();
  int hora = f.hour();
  int minuto = f.minute();
  int segundo = f.second();


  //sprintf( fecha, "%04d-%02d-%02d %02d:%02d:%02d", anio, mes, dia, hora, minuto, segundo);
  return  String(f.minute()) ;
}
