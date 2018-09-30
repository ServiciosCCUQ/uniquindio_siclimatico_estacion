
int sensor1 = 9; // sensor balancin

void setup() {
  // start serial port at 115200 bps:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

   pinMode(sensor1,INPUT); 
       digitalWrite(sensor1,LOW);  // digital sensor is on digital pin 2
  establishContact();  // send a byte to establish contact until receiver responds
}

void loop() {
  
}

void establishContact() {
  while (Serial.available() <= 0) {
    int Sensor1 = digitalRead(sensor1);     // leer el dato del puerto 9
Serial.print(Sensor1); // send a capital A
    delay(6000);
  }
}
