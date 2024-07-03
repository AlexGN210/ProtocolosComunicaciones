#include <Wire.h>
#include <SoftwareSerial.h>

const int rxB = 2; // RX pin of SoftwareSerial (SerialB)
const int txB = 3; // TX pin of SoftwareSerial (SerialB)
const int ledA = 8; // LED for "Cake" or "Hello" on Serial
const int ledB = 9; // LED for "Cake" or "Hello" on SerialB

SoftwareSerial SerialB(rxB, txB);

String responseA = "";
String responseB = "";

void setup() {
  Serial.begin(9600);
  SerialB.begin(9600);
  Wire.begin();
  pinMode(ledA, OUTPUT); // Set LEDA pin as output
  pinMode(ledB, OUTPUT); // Set LEDB pin as output
  digitalWrite(ledA, LOW); // Ensure LEDA is initially off
  digitalWrite(ledB, LOW); // Ensure LEDB is initially off
}

void loop() {
  static unsigned long lastTimeSerialA = 0;
  static unsigned long lastTimeSerialB = 0;
  unsigned long currentTime = millis();

  // Enviar 'A>' por Serial cada 3 segundos
  if (currentTime - lastTimeSerialA >= 3000) {
    Serial.print("A>");
    lastTimeSerialA = currentTime;
  }

  // Enviar 'B>' por SerialB cada 5 segundos
  if (currentTime - lastTimeSerialB >= 5000) {
    SerialB.print("B>");
    lastTimeSerialB = currentTime;
  }

  // Leer respuesta del ATmega328p desde Serial
  if (Serial.available() > 0) {
    responseA = Serial.readStringUntil('>'); // Leer hasta el carácter '>'
    responseA += '>'; // Agregar el carácter '>' al final
    if (responseA == "Cake>" || responseA == "Hello>") {
      digitalWrite(ledA, HIGH); // Enciende el LED en D8
    } else {
      digitalWrite(ledA, LOW); // Apaga el LED en D8
    }

    // Enviar la cadena recibida por Serial a través de I2C
    Wire.beginTransmission(0x08);
    Wire.write(responseA.c_str());
    Wire.endTransmission();
  }

  // Leer respuesta del ATmega328p desde SerialB
  if (SerialB.available() > 0) {
    responseB = SerialB.readStringUntil('>'); // Leer hasta el carácter '>'
    responseB += '>'; // Agregar el carácter '>' al final
    if (responseB == "Cake>" || responseB == "Hello>") {
      digitalWrite(ledB, HIGH); // Enciende el LED en D9
    } else {
      digitalWrite(ledB, LOW); // Apaga el LED en D9
    }

    // Enviar la cadena recibida por SerialB a través de I2C
    delay(3000);
    Wire.beginTransmission(0x08);
    Wire.write(responseB.c_str());
    Wire.endTransmission();
  }
}
