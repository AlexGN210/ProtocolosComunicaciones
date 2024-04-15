/*
 * timer.c
 *
 * Created: 27/03/2024 04:00:32 p. m.
 *  Author: ShockWave
 */ 
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "timer.h"

void vTimerInit(void){
	CLKPR = 0x80;
	CLKPR = 0x00;

	TCCR0A = (1 << WGM01);
	TCCR0B = (1 << CS02) | (1 << CS00);
	TIMSK0 = (1 << OCIE0A);
	OCR0A = 7;
	
	sei();
	
}