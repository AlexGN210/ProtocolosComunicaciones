//

#include <avr/io.h>
#include <string.h>
#include "serial.h"

void vSerialInit(void){
	CLKPR = 0x80;
	CLKPR = 0x00;
	UCSR0A = (0<<TXC0)|(1<<U2X0)|(0<<MPCM0);
	UCSR0B = (0<<RXCIE0)|(0<<TXCIE0)|(0<<UDRIE0)|(1<<RXEN0)|(1<<TXEN0)|(0<<UCSZ02)|(0<<TXB80);
	UCSR0C = (1 << UCSZ01) | (1 << UCSZ00) | (0 << USBS0);
	UBRR0 = 103; //(F_CPU/8/9600)-1; // Alexis.
}

	void vWRITEChar(char data){  //Escritura de un solo caracter.
		while ( !( UCSR0A & (1<<UDRE0)) );
		UDR0 = data;
	}


	void vWRITEString(const char *str) { //Escritura de una cadena que mientras exista espacio en la cadena se enviará el dato.
		while (*str) {
			vWRITEChar(*str);
			str++;
		}
	}

	uint8_t vREADChar() { //Lectura de un solo carácter cuando esté disponible el slot
		while (!(UCSR0A & (1 << RXC0)));
		return UDR0;
	}

	void vREADString(char* buffer) {
		int tambuf = -1; // Inicializado a -1 para que no se mande el salto de línea prueba "7"
		char char_recibido;  // Caracter recibido.
			
			for (int i = 0; i < BUFFER_SIZE; i++) {
				buffer[i] = '\0';
				char_recibido = '\0';
			}
	
		do {
			char_recibido = vREADChar();
			tambuf++; // Contador para recorrer el espacio de la variable.
			buffer[tambuf] = char_recibido; //Escritura de la cadena.

		} while (char_recibido != '>' && tambuf < BUFFER_SIZE && bandera==0); // Condición para poder salir con salto de línea, cuando se excede el buffer y cuando el timer se termina.
		
		 buffer[tambuf] = '\0';  // Convertir en string para imprimir el dato recibido. Esto es simplemente para demostración en simulada del carácter.
	}	