#include "SoftwareSerial.h"

// Configura los pines de ESPSoftwareSerial
const int rxPin = 22; // Pin virtual RX
const int txPin = 23; // Pin virtual TX

// Configura la comunicación serial virtual
SoftwareSerial mySerial(rxPin, txPin);

void setup() {

// Inicia la comunicación serial estándar y virtual
    Serial.begin(115200);
    mySerial.begin(9600);
    pinMode(2,OUTPUT);

}

void loop() {
 char datos = mySerial.read();
 delay(100);
 Serial.print(datos);
 Serial.print(">");
  delay(3000);

}

