/*
 * UART_Alexis.c  MAIN PARA EL RECEPTOR
 *
 * Created: 22/02/2024 09:31:12 p. m.
 * Author : ShockWave
 */
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include "serial.h"
#include "timer.h"
#include "SSD1306/SSD1306.h"
#include "SSD1306/Font5x8.h"

int timer;   //Variable global
int timer2;  //Variable global
int bandera; //Variable global
int posY=8;
int i = 0;
int overflow = 0;

ISR(TIMER0_COMPA_vect) {
	timer++;
	timer2++;
	if (timer >= 5000) { // Si han pasado 3000 interrupciones (3 segundos) | Variable para el finalizado de lectura
		bandera= 1;
		timer=0;
	}
	
}

int main(void)
{	
	bandera = 0; //Variable global inicializada utilizada para saber cuando se excede el temporizador
	timer = 0; //Variable global inicializada
	
	DDRD |= (1<<DDD1); //Tx Establecido como salida.
	DDRC |= (1<<DDD4)|(1<<DDD5);
	char buffer[BUFFER_SIZE]; // Buffer para almacenar la cadena recibida
	
	// Inicialización del puerto serie
	vTimerInit();  //Inicializar el timer.
	vSerialInit(); //Inicializar la comunicación serial.
	GLCD_Setup();
	GLCD_SetFont(Font5x8,5,8,GLCD_Overwrite);
    
	
	while (1)
	{		
		
		vREADString(buffer); // Llamada a la función para leer una línea
		vWRITEString(buffer); // Envío de la línea recibida
		vWRITEChar('\r'); // Salto de línea
		if(i==4){
		posY = 8;
		GLCD_Clear();
		GLCD_Render();
		i=0;
		}
				GLCD_GotoXY(3,posY);
				GLCD_PrintString(buffer);
				GLCD_Render();
				posY = posY + 14;
				i++;
				
				bandera = 0;
				timer=0;
		
	
	}
}