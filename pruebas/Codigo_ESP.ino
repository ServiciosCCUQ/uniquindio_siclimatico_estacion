/*
   Ejemplo para ESP8266
*/

#include <ESP8266WiFi.h>

const char* ssid     = "GASO";
const char* password = "delphos*BI";

//char url[] = "www.aprendiendoarduino.com";
char url[] = "192.168.0.4";

WiFiClient client;

String webString = "";
String nombre = "";
String mensaje = "";

void setup()
{
  Serial.begin(115200);
  delay(10);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void loop()
{
  String webString = "";
  String nombre = "";
  String mensaje = "";


  while (Serial.available() == 0) {
  }
  do {
    char caracter_leido = Serial.read();
    if (caracter_leido == ' ') caracter_leido = '_';
    if (caracter_leido == '\r') continue;
    if (caracter_leido == '\n') continue;
    nombre += caracter_leido;   
    delay(5);
  }  while (Serial.available() > 0);
  Serial.println(nombre);


  grabaDatos(nombre);
  delay(200);

  while (client.available() == 0) {
  }

  if (client.available()) {
    Serial.println("Respuesta del Servidor---->");
    while (client.available()) {
      char c = client.read();
      webString += c;
    }
    Serial.println(webString);
    if (webString.indexOf("ok") >= 0) Serial.println("Datos guardados correctamente");
    else Serial.println("Error al guardar los datos");

    client.stop();

  }
}

void grabaDatos(String nombre) {
  Serial.println("enviando mensaje... ");

  Serial.println("connecting to server...");
  if (client.connect(url, 5050)) {
    Serial.println("connected");
    client.print("GET /recibirdatos?raw=");
    client.print(nombre);
    //client.print("&mensaje=");
    //client.print(mensaje);
    client.println(" HTTP/1.1");
    client.println("Host: 192.168.0.2");
    client.println("Connection: close");
    client.println();
    Serial.print("GET /recibirdatos?raw=");
    Serial.print(nombre);
   // Serial.print("&mensaje=");
   // Serial.println(mensaje);
  }
  else {
    Serial.println("connection failed");
  }
}
