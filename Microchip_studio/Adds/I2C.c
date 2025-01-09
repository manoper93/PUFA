/*
 * I2C.c
 *
 * Sergio Isidoro - 1221709
 * Andre Nogueira - 1221709
 *
 * CHIP: Atmega328p 8MHz
 */

#include "I2C.h"

void I2C_init(void) {									// Inicializa o barramento I2C/TWI
	TWSR = 0x00;										// Define o prescaler para 1
	TWBR = ((F_CPU / 400000UL) - 16) / 2;				// Configura o registrador TWBR para obter SCL = 400kHz
	TWCR = (1 << TWEN);									// Habilita o m�dulo TWI/I2C
}

void I2C_start(void) {									// Gera uma condi��o de START no barramento I2C
	TWCR = (1 << TWSTA) | (1 << TWEN) | (1 << TWINT);	// Gera START, habilita TWI e limpa a flag TWINT
	while (!(TWCR & (1 << TWINT)));						// Aguarda a conclus�o do START
}

void I2C_stop(void) {									// Gera uma condi��o de STOP no barramento I2C
	TWCR = (1 << TWSTO) | (1 << TWEN) | (1 << TWINT);   // Gera STOP, habilita TWI e limpa a flag TWINT
}

void I2C_write(uint8_t data) {							// Escreve um byte no barramento I2C
	TWDR = data;										// Carrega o dado no registrador de dados TWI
	TWCR = (1 << TWINT) | (1 << TWEN);					// Inicia a transmiss�o e limpa a flag TWINT
	while (!(TWCR & (1 << TWINT)));						// Aguarda a conclus�o da transmiss�o
}

uint8_t I2C_read_ack(void) {							// L� um byte do barramento I2C e envia um ACK (confirma��o)
	TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWEA);	// Habilita a leitura, limpa a flag TWINT e configura para enviar ACK
	while (!(TWCR & (1 << TWINT)));						// Aguarda a conclus�o da leitura
	return TWDR;										// Retorna o dado recebido
}

uint8_t I2C_read_nack(void) {							// L� um byte do barramento I2C e envia um NACK (n�o confirma��o)
	TWCR = (1 << TWINT) | (1 << TWEN);					// Habilita a leitura, limpa a flag TWINT e configura para enviar NACK
	while (!(TWCR & (1 << TWINT)));						// Aguarda a conclus�o da leitura
	return TWDR;										// Retorna o dado recebido
}