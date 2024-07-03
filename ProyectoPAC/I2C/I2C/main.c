/*
 * I2C.c
 *
 * Created: 02/07/2024 05:20:32 p. m.
 * Author : ShockWave
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/twi.h>
#include <string.h>

#define SLAVE_ADDRESS 0x29

volatile char receivedData[6]; // Max word length + null terminator

void initI2C_slave(uint8_t address) {
	TWAR = (address << 1);
	TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWIE);
}

void i2c_receive_event() {
	static uint8_t index = 0;

	if ((TWSR & 0xF8) == TW_SR_DATA_ACK) {
		receivedData[index] = TWDR;
		index++;

		if (index >= sizeof(receivedData)) {
			index = 0;

			if (strcmp(receivedData, "Cake") == 0) {
				TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWEA);
				TWDR = 0x48; // Send address 0x48 (ASCII 'H')
				} else if (strcmp(receivedData, "Hello") == 0) {
				TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWEA);
				TWDR = 0x50; // Send address 0x50 (ASCII 'P')
				} else {
				TWCR = (1<<TWEN) | (1<<TWINT) | (1<<TWEA);
				TWDR = '\0'; // Send null terminator (no response)
			}
		}
	}
}

ISR(TWI_vect) {
	switch (TW_STATUS) {
		case TW_SR_SLA_ACK:
		case TW_SR_DATA_ACK:
		i2c_receive_event();
		break;
		default:
		TWCR = (1<<TWEN) | (1<<TWEA) | (1<<TWINT);
		break;
	}
}

int main(void) {
	initI2C_slave(SLAVE_ADDRESS);
	sei();

	while (1) {
		// Application code can be added here if needed
	}
	
	return 0;
}
