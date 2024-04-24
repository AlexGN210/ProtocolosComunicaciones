

#ifndef SERIAL_H_
#define SERIAL_H_

extern int bandera;
extern int timer;
extern int overflow;  

#define BUFFER_SIZE 15 //Tamaño del buffer establecido por el usuario

void vSerialInit();

void vWRITEString(const char *data);
void vWRITEChar(char data);
uint8_t vREADChar();
void vREADString(char* buffer);

#endif /* SERIAL_H_ */