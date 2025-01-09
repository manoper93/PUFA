/*
 * OLED.h
 *
 * Sergio Isidoro - 1221709
 * Andre Nogueira - 1221709
 *
 * CHIP: Atmega328p 8MHz
 */



#ifndef OLED_H
#define OLED_H

#include <avr/io.h>
#include <avr/pgmspace.h>
#include "I2C.h"								// Necess�rio para usar fun��es de I2C

#define OLED_ADDRESS 0x3C						// Endere�o padr�o do OLED

void oled_command(uint8_t cmd);					// Fun��o para enviar um comando ao OLED
void oled_data(uint8_t data);					// Fun��o para enviar um dado ao OLED
void oled_clear(void);							// Limpa a tela do OLED
void oled_init(void);							// Inicializa o OLED
void oled_set_cursor(uint8_t row, uint8_t col);	// Define o cursor (linha e coluna)
extern const uint8_t font5x7[][5];				// Fonte de 5x7 pixels (declarada como constante em PROGMEM)
void oled_print(char* str);						// Fun��o para enviar string

#endif