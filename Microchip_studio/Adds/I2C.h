/*
 * I2C.h
 *
 * Sergio Isidoro - 1221709
 * Andre Nogueira - 1221709
 *
 * CHIP: Atmega328p 8MHz
*/

#ifndef I2C_H
#define I2C_H
#define F_CPU 8000000UL

#include <avr/io.h>

void I2C_init(void);			// Inicializa o m�dulo I2C (TWI)
void I2C_start(void);			// Envia condi��o de start no barramento I2C
void I2C_stop(void);			// Envia condi��o de stop no barramento I2C
void I2C_write(uint8_t data);	// Escreve um byte de dados no barramento I2C
uint8_t I2C_read_ack(void);		// L� um byte do barramento I2C e envia ACK
uint8_t I2C_read_nack(void);	// L� um byte do barramento I2C e envia NACK

#endif
