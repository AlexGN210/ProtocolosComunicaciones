#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "serial.h"

extern char buffer[BUFFER_SIZE]; //Buffer para la recepción de la comunicación serial.


int main() {
	vSerialInit(); // Inicializar la comunicación Serial.
	
	DDRD |= (1<<DDD2); // Habilitar PIND2 como salida
	
	sei(); // Habilita las interrupciones.


	while(1) {
		// Sección para la lectura y confirmación del dato recibido para tomar acción
		_delay_ms(100); // Delay para la recepción de los datos del UART
		vREADString(buffer); // Lectura del carácter recibido.
		
		if (strcmp(buffer, "A") == 0){
			PORTD ^= (1<<PORTD2);
		vWRITEString("Cake>"); // Envío de la línea recibida
		vWRITEChar('\r'); // Salto de línea
	}
	
		if (strcmp(buffer, "B") == 0){
			PORTD ^= (1<<PORTD2);

		vWRITEString("Hello>"); // Envío de la línea recibida
		vWRITEChar('\r'); // Salto de línea
	}


	}

	return 0;
}