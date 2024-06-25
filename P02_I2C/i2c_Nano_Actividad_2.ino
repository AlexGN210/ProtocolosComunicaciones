#include <Wire.h>

void setup() {
  Wire.begin(0x52); // Inicializa el bus I2C con dirección de esclavo 0x50 o 0x52
  Wire.onReceive(datoRecibido);
  pinMode(13,OUTPUT);
}

void loop() {
  datoRecibido(18);
}

void datoRecibido(int byteCount) {
  while (Wire.available()) {
    char dato = Wire.read(); // Lee el dato enviado por el maestro
    if (dato == 'A') {
      digitalWrite(13,1);
    } else if (dato == 'B') {
      digitalWrite(13,0);
    }
  }
}
