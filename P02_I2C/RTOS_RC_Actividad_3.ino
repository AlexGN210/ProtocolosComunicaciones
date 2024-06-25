//Implementado y programado por Alexis García Nieves, Ingeniería Mecatrónica. 

#include <Arduino_FreeRTOS.h>
#include <VL53L0X.h>
#include <VL6180X.h>
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>

bool leerSensores = true;

// Pines para SPI del MRC522
#define RST_PIN         9  
#define SS_PIN          10 

MFRC522 mfrc522(SS_PIN, RST_PIN); 

MFRC522::MIFARE_Key key;

// Pines para Controlador L298N
const int pinMotorFrontal1 = 3;
const int pinMotorFrontal2 = 4; 
const int pinMotorTrasero1 = 7; 
const int pinMotorTrasero2 = 8; 

class MotorGroup {
private:
    int pin1;
    int pin2;

public:
    // Constructor
    MotorGroup(int pin1, int pin2) : pin1(pin1), pin2(pin2) {
        pinMode(pin1, OUTPUT);
        pinMode(pin2, OUTPUT);
    }

    // Método para avanzar con una velocidad dada
    void avanzar(int speed) {
        // Movimiento hacia adelante
        digitalWrite(pin1, HIGH);
        digitalWrite(pin2, LOW);
        analogWrite(pin1, abs(speed));
    }

    // Método para retroceder con una velocidad dada
    void retro(int speed) {
        // Movimiento hacia atrás
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, HIGH);
        analogWrite(pin1, abs(speed));
    }

    // Método para detener el movimiento
    void detener() {
        // Detener el motor
        digitalWrite(pin1, LOW);
        digitalWrite(pin2, LOW);
        analogWrite(pin1, 0);
    }
};

// SDA (Data): A4
// SCL (Clock): A5

const int SHT_S1 = 5;
const int SHT_S2 = 6;
const int SHT_S3 = A0;
const int SHT_S4 = 2; 

// Direcciones de los sensores
#define VL53L0X_1_ADDRESS 0x30
#define VL6180X_2_ADDRESS 0x31
#define VL6180X_3_ADDRESS 0x32
#define VL6180X_4_ADDRESS 0x33

// Objetos para los sensores
VL53L0X sensor1;
VL6180X sensor2;
VL6180X sensor3;
VL6180X sensor4;

MotorGroup motorFrontal(pinMotorFrontal1, pinMotorFrontal2);
MotorGroup motorTrasero(pinMotorTrasero1, pinMotorTrasero2);

void VLsetup(int num) {
  if (num == 4) {
    sensor1.init();
    sensor1.setAddress(VL53L0X_1_ADDRESS);
    sensor1.setTimeout(200);

    // stop continuous mode if already active
    sensor1.stopContinuous();
    // in case stopContinuous() triggered a single-shot
    // measurement, wait for it to complete
    delay(200);
    // start continuous mode with period of 100 ms
    sensor1.startContinuous(100);
    delay(10);
  } else if (num == 2) {
    sensor2.init();
    sensor2.setAddress(VL6180X_2_ADDRESS);
    sensor2.configureDefault();
    sensor2.writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);
    sensor2.writeReg16Bit(VL6180X::SYSALS__INTEGRATION_PERIOD, 50);
    sensor2.setTimeout(200);
  } else if (num == 3) {
    sensor3.init();
    sensor3.setAddress(VL6180X_3_ADDRESS);
    sensor3.configureDefault();
    sensor3.writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);
    sensor3.writeReg16Bit(VL6180X::SYSALS__INTEGRATION_PERIOD, 50);
    sensor3.setTimeout(200);
  }
  else if (num == 1) {
    sensor4.init();
    sensor4.setAddress(VL6180X_4_ADDRESS);
    sensor4.configureDefault();
    sensor4.writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);
    sensor4.writeReg16Bit(VL6180X::SYSALS__INTEGRATION_PERIOD, 50);
    sensor4.setTimeout(200);
  }

  //setup RFID
  SPI.begin();
  mfrc522.PCD_Init();

}

void setID(void *pvParameters) {
  // Reset a todos
  digitalWrite(SHT_S1, LOW);
  digitalWrite(SHT_S2, LOW);
  digitalWrite(SHT_S3, LOW);
  digitalWrite(SHT_S4, LOW);
  delay(10);

  // Encender sensores
  digitalWrite(SHT_S1, HIGH);
  digitalWrite(SHT_S2, HIGH);
  digitalWrite(SHT_S3, HIGH);
  digitalWrite(SHT_S4, HIGH);
  delay(10);

  // Activar Sensor 1 y apagar Sensores 2 y 3
  digitalWrite(SHT_S4, HIGH);
  digitalWrite(SHT_S2, LOW);
  digitalWrite(SHT_S3, LOW);
  digitalWrite(SHT_S1, LOW);
  delay(10);

  // Inicializar Sensor 1
  VLsetup(1);
   delay(100);
   
  // Activar Sensor 2
  digitalWrite(SHT_S2, HIGH);
  delay(10);

  // Inicializar Sensor 2
  VLsetup(2);
 delay(100);

  // Activar Sensor 3
  digitalWrite(SHT_S3, HIGH);
  delay(10);

  // Inicializar Sensor 3
  VLsetup(3);
  delay(100);

    // Activar Sensor 4
  digitalWrite(SHT_S1, HIGH);
  delay(10);

  // Inicializar Sensor 4
  VLsetup(4);
  delay(100);
  vTaskDelay(200 / portTICK_PERIOD_MS); // Esperar 200ms
  vTaskDelete(NULL); // Terminar esta tarea
}


void VLread(void *pvParameters) {
  while (1) {
    if (leerSensores) {
      Serial.print("\tS1 Range: ");
      Serial.print(sensor1.readRangeContinuousMillimeters());
      if (sensor1.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

      Serial.print(" | ");

      Serial.print("\tS2 Range: ");
      Serial.print(sensor2.readRangeSingleMillimeters());
      if (sensor2.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

      Serial.print(" | ");

      Serial.print("\tS3 Range: ");
      Serial.print(sensor3.readRangeSingleMillimeters());
      if (sensor3.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

     Serial.print(" | ");

      Serial.print("\tS4 Range: ");
      Serial.print(sensor4.readRangeSingleMillimeters());
      if (sensor4.timeoutOccurred()) { Serial.print(" TIMEOUT"); }

      Serial.println();
    }
    vTaskDelay(200 / portTICK_PERIOD_MS); // Esperar 200ms
  }
}

void BTread(void *pvParameters) {
  while (1) {
    // Verifica si hay datos disponibles para leer:
    if (Serial.available() > 0) {
      // Lee los datos entrantes hasta el carácter de nueva línea:
      String incomingData = Serial.readStringUntil('\n');
      incomingData.trim(); // Eliminar caracteres en blanco al principio y al final

      // Imprime los datos recibidos en el monitor serial:
      Serial.print("Recibido: ");
      Serial.println(incomingData);

      if (incomingData.equals("A")) {
       

      }
      if (incomingData.equals("B")) {
        
        
      }
    }
    vTaskDelay(10 / portTICK_PERIOD_MS); // Agregar un pequeño delay para evitar un bucle apretado
  }
}

void RFIDread(void *pvParameters) {
  while (1) {
    // Mira si hay una nueva tarjeta presente
    if (mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial()) {

      for (byte i = 0; i < 6; i++) key.keyByte[i] = 0xFF;

      byte block;
      byte len;
      MFRC522::StatusCode status;
      byte buffer1[18];

      block = 4;
      len = 18;
      
      status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, 4, &key, &(mfrc522.uid)); //line 834 of MFRC522.cpp file
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("Authentication failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        vTaskDelay(10 / portTICK_PERIOD_MS); // Espera pequeño retardo antes de continuar
        continue; // Volver a intentar
      }

      status = mfrc522.MIFARE_Read(block, buffer1, &len);
      if (status != MFRC522::STATUS_OK) {
        Serial.print(F("Reading failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        vTaskDelay(10 / portTICK_PERIOD_MS); // Espera pequeño retardo antes de continuar
        continue; // Volver a intentar
      }

      //IMPRIMIR INFORMACIÓN ESCRITA
      for (uint8_t i = 0; i < 16; i++) {
        if (buffer1[i] != 32) {
          Serial.write(buffer1[i]);
        }
      }
      Serial.print(" ");

      vTaskDelay(1000 / portTICK_PERIOD_MS); // Esperar 100ms

      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
    }
    vTaskDelay(100 / portTICK_PERIOD_MS); // Esperar 100ms
  }
}

void setup() {
  Serial.begin(9600);
  Wire.begin();

  // Configurar pines de control de los sensores
  pinMode(SHT_S1, OUTPUT);
  pinMode(SHT_S2, OUTPUT);
  pinMode(SHT_S3, OUTPUT);

  xTaskCreate(setID, "setID", 128, NULL, 2, NULL);
  xTaskCreate(VLread, "VLread", 128, NULL, 1, NULL);
  xTaskCreate(BTread, "BTread", 64, NULL, 1, NULL);
  xTaskCreate(RFIDread, "RFIDread", 128, NULL, 1, NULL);
}

void loop() {
  // No es necesario añadir código aquí ya que las tareas están siendo manejadas por FreeRTOS
}
