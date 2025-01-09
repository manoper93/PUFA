/*
 * ADC.h
 *
 * Sergio Isidoro - 1221709
 * Andre Nogueira - 1221709
 *
 * CHIP: Atmega328p 8MHz
 */


#ifndef ADC_H
#define ADC_H

#include <avr/io.h>

void ADC_init(void);					// Inicializa o m�dulo ADC
uint16_t ADC_read(uint8_t canal);		// L� o valor do ADC de um canal especificado

#endif
