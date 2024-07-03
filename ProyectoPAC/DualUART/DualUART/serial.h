#ifndef SERIAL_H_
#define SERIAL_H_

#define BUFFER_SIZE 15 //Tamaño del buffer establecido por el usuario
char buffer[BUFFER_SIZE]; 

void vSerialInit();

void vWRITEString(const char *data);
void vWRITEChar(char data);
uint8_t vREADChar();
void vREADString(char* buffer);

#endif /* SERIAL_H_ */