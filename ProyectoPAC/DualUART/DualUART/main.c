#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "serial.h"

extern char buffer[BUFFER_SIZE]; //Buffer para la recepci�n de la comunicaci�n serial.


int main() {
	vSerialInit(); // Inicializar la comunicaci�n Serial.
	
	DDRD |= (1<<DDD2); // Habilitar PIND2 como salida
	
	sei(); // Habilita las interrupciones.


	while(1) {
		// Secci�n para la lectura y confirmaci�n del dato recibido para tomar acci�n
		_delay_ms(100); // Delay para la recepci�n de los datos del UART
		vREADString(buffer); // Lectura del car�cter recibido.
		
		if (strcmp(buffer, "A") == 0){
			PORTD ^= (1<<PORTD2);
		vWRITEString("Cake>"); // Env�o de la l�nea recibida
		vWRITEChar('\r'); // Salto de l�nea
	}
	
		if (strcmp(buffer, "B") == 0){
			PORTD ^= (1<<PORTD2);

		vWRITEString("Hello>"); // Env�o de la l�nea recibida
		vWRITEChar('\r'); // Salto de l�nea
	}


	}

	return 0;
}