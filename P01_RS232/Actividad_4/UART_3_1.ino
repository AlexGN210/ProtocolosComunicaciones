void setup() {
 // Inicia la comunicación serial con 115200 baudios, paridad PARITY_ODD (impar) y 1 bit de stop
  Serial.begin(115200, SERIAL_8O1);

}

void loop() {
  Serial.println('M');
  delay(500);

}