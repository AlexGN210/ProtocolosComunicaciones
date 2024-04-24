/*
 * UART_Alexis.c
 *
 * Created: 22/02/2024 09:31:12 p. m.
 * Author : ShockWave
 */
#define F_CPU 8000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "serial.h"
#include "timer.h"

int timer;   //Variable global
int timer2;  //Variable global
int bandera; //Variable global


ISR(TIMER0_COMPA_vect) {
	timer++;
	timer2++;
	if (timer >= 3000) { // Si han pasado 3000 interrupciones (3 segundos) | Variable para el finalizado de lectura
		bandera= 1;
		timer=0;
	}
	if (timer2 >= 1000) { // Si han pasado 1000 interrupciones (1 segundo) | Variable para envío constante de cadena
		vWRITEString("123");
		vWRITEChar('\r');
		timer2=0;
	}	
	
}

int main(void)
{	
	bandera = 0; //Variable global inicializada utilizada para saber cuando se excede el temporizador
	timer = 0; //Variable global inicializada
	
	DDRD |= (1<<DDD1); //Tx Establecido como salida.
	char buffer[BUFFER_SIZE]; // Buffer para almacenar la cadena recibida
	
	// Inicialización del puerto serie
	vTimerInit();  //Inicializar el timer.
	vSerialInit(); //Inicializar la comunicación serial.
    
	while (1)
	{		
		
		vREADString(buffer); // Llamada a la función para leer una línea
		vWRITEString(buffer); // Envío de la línea recibida
			
		vWRITEChar('\r'); // Salto de línea
				bandera = 0;
				timer=0;
	}
}
