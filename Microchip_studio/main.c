/*
 *	MAIN.c
 *
 * Sergio Isidoro - 1221709
 * Andre Nogueira - 1221709
 *
 * CHIP: Atmega328p 8MHz
 * BAUD 76800	
*/

#define F_CPU 8000000UL

#include <inttypes.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <math.h>

#include "Adds/ADC.h"
#include "Adds/I2C.h"
#include "Adds/IO.h"
#include "Adds/OLED.h"
#include "Adds/TIMER0.h"
#include "Adds/TIMER1.h"
#include "Adds/TIMER2.h"
#include "Adds/UART.h"

static char buf_oled[50];
static char buf_UART[120];
volatile uint8_t start_OLED = 0;
float battery_perc = 0.0, voltage = 0.0;

#define MPU6050_ADDR 0x68														// Endere�o I2C do MPU6050 (7 bits)
#define ACCEL_XOUT_H 0x3B														// Registro de in�cio para leitura do aceler�metro
#define PWR_MGMT_1 0x6B															// Registro para configura��o do power management

#define ACCEL_CHANGE_THRESHOLD 1.2												// Limiar de mudan�a r�pida (em g)

void MPU6050_init(void) {
	I2C_start();																// Inicia envio de comandos para configurar o MPU6050 no modo normal
	I2C_write(MPU6050_ADDR << 1);												// Envia o endere�o do sensor no barramento I2C
	I2C_write(PWR_MGMT_1);														// Endere�o do registrador de gerenciamento de energia
	I2C_write(0x00);															// Configura o sensor para o modo ativo (normal)
	I2C_stop();
}

void detect_fall(void) {
	static float last_a_result = 1.0;											// �ltimo valor suavizado do vetor resultante

	uint8_t high, low;
	int16_t raw_ax, raw_ay, raw_az;
	float ax, ay, az, a_result;

	// L� os valores brutos de acelera��o
	I2C_start();
	I2C_write(MPU6050_ADDR << 1);
	I2C_write(ACCEL_XOUT_H);
	I2C_stop();

	I2C_start();
	I2C_write((MPU6050_ADDR << 1) | 0x01);
	high = I2C_read_ack();													// L� byte alto do eixo X
	low = I2C_read_ack();													// L� byte baixo do eixo X
	raw_ax = (high << 8) | low;

	high = I2C_read_ack();													// L� byte alto do eixo Y
	low = I2C_read_ack();													// L� byte baixo do eixo Y
	raw_ay = (high << 8) | low;

	high = I2C_read_ack();													// L� byte alto do eixo Z
	low = I2C_read_nack();													// L� byte baixo do eixo Z (�ltima leitura)
	raw_az = (high << 8) | low;
	I2C_stop();

	// Converte os valores para unidades de gravidade (g)
	ax = raw_ax / 16384.0;
	ay = raw_ay / 16384.0;
	az = raw_az / 16384.0;

	// Calcula o vetor resultante da acelera��o
	a_result = sqrt(ax * ax + ay * ay + az * az);

	// Se mudan�as brutas indicando poss�vel queda e define o sinalizador de queda
	if (fabs(a_result - last_a_result) > ACCEL_CHANGE_THRESHOLD) fall = 1;

	// Atualiza o �ltimo valor resultante
	last_a_result = a_result;
}

float battery_percentage() {
	uint16_t adc_value = ADC_read(3);											// L� o valor ADC do pino 3
	voltage = (adc_value * 3.3) / 1023.0;									// Converte o valor ADC para tens�o

	if (voltage > 3.0) voltage = 3.0;											// Limita o valor da tens�o para a faixa operacional da bateria
	if (voltage < 2.4) voltage = 2.4;											// Limita o valor da tens�o para a faixa operacional da bateria

	return ((voltage - 2.4) / (3.0 - 2.4)) * 100.0;								// Calcula a porcentagem da bateria com base na tens�o
}

//-------------------------------------------------------------------------------------------------- MAIN
int main() {
	ADC_init();
	I2C_init();
	IO_init();
	oled_init();
	TIMER0_init();
	TIMER1_init();
	TIMER2_init();
	UART_init();
	MPU6050_init();
	sei();
	
	PWM2_DDR &= ~(1 << PWM2_PIN);												// Configura o pino do PWM como entrada inicialmente (desabilita o PWM)
	
	OCR1A = ICR1 / 2;															// Define o duty cycle inicial para 50% do per�odo (ICR1 / 2)

	while (1) {
				
		//------------------------------------------------------------------------------------------------------------------ FALL
		
		detect_fall();															// Detecta quedas ou eventos espec�ficos usando a fun��o detect_fall()
		
				
		//------------------------------------------------------------------------------------------------------------------ Battery
		
		if (update_batt == 1) {													// Atualiza o estado da bateria, se necess�rio
			battery_perc = battery_percentage();								// Calcula a porcentagem da bateria
			update_batt = 0;
		}
		
		//------------------------------------------------------------------------------------------------------------------ UART
		
		if (update_UART == 1 && uart_status > 0) {																				// Envia dados via UART para comunica��o com o servidor ThingSpeak
			if (uart_status == 1) {																								// Inicia conex�o SSL com o servidor
				sprintf(buf_UART, "AT+CIPSTART=\"SSL\",\"api.thingspeak.com\",443\r\n");
				UART_send_string(buf_UART);
			} else if (uart_status == 2 && BPM < 100.0) {																		// Envia comando para transmitir dado com comprimento 74
				sprintf(buf_UART, "AT+CIPSEND=74\r\n");
				UART_send_string(buf_UART);
			} else if (uart_status == 2 && BPM >= 100.0) {																		// Envia comando para transmitir dado com comprimento 75
				sprintf(buf_UART, "AT+CIPSEND=75\r\n");
				UART_send_string(buf_UART);
			} else if (uart_status == 3) {																						// Envia a requisi��o GET para atualizar o campo no ThingSpeak com o valor de BPM
				sprintf(buf_UART, "GET https://api.thingspeak.com/update?api_key=N0GS7H87SQD4IBUS&field1=%d\r\n", (int)BPM);
				UART_send_string(buf_UART);
			}
			update_UART = 0;
		}
		
		//------------------------------------------------------------------------------------------------------------------ OLED

		if (update_OLED == 1) {													// Verifica flag de atualiza��o de 1ms
			if (start_OLED == 0) {												// Inicializa a tela OLED com mensagens padr�o
				oled_set_cursor(1, 2); oled_print("Sergio Isidoro");
				oled_set_cursor(2, 2); oled_print("1221709");
				oled_set_cursor(3, 2); oled_print("Andre Nogueira");
				oled_set_cursor(4, 2); oled_print("1212048");
				oled_set_cursor(6, 2); oled_print("LABSI Project");
				oled_set_cursor(7, 2); oled_print("LED RED 1Hz");
				_delay_ms(1000);
				oled_clear();
				oled_set_cursor(1, 2); oled_print("BPM inst: ---");
				oled_set_cursor(2, 2); oled_print("BPM:      ---");
				oled_set_cursor(3, 2); oled_print("Battery:  ---%");
				start_OLED = 1;													// Marca como inicializado
			}
			
			if (BPM >= 30.0 && BPM <= 240.0) {									// Mostra o valor instant�neo de BPM se estiver no intervalo v�lido
				sprintf(buf_oled, "BPM inst: %d", (int)BPM);
				oled_set_cursor(1, 2);
				oled_print(buf_oled);
			}
			
			if (fall == 1) {													// Mostra alertas na tela e habilita/desabilita o PWM baseado em condi��es
				PWM2_DDR |= (1 << PWM2_PIN);									// Ativa o PWM
				oled_set_cursor(0, 2); oled_print("FALL");
			} else if (BPM >= 180.0) {
				PWM2_DDR |= (1 << PWM2_PIN);									// Ativa o PWM
				oled_set_cursor(0, 2); oled_print("HIGH FREQ");
			} else {
				PWM2_DDR &= ~(1 << PWM2_PIN);									// Desativa o PWM
				oled_set_cursor(0, 2); oled_print(" ");
			}
			
			if (average_min > 0) {												// Mostra a m�dia de BPM por minuto
				sprintf(buf_oled, "BPM:      %d ", (int)average_min);
				oled_set_cursor(2, 2); oled_print(buf_oled);
			}
			
			if ((int)battery_perc > 0) {										// Mostra a m�dia de BPM por minuto
				sprintf(buf_oled, "Battery:  %d%%", (int)battery_perc);			// Atualiza a percentagem de bateria e outras informa��es
				oled_set_cursor(3, 2); oled_print(buf_oled);
			}
			
			sprintf(buf_oled, "T (ms):   %u", (int)time_count);					// Mostra o tempo atual em ms
			oled_set_cursor(5, 2); oled_print(buf_oled);
			
			sprintf(buf_oled, "Thresh:   %d", thresh);							// Mostra o valor atual da mediana calculada
			oled_set_cursor(6, 2); oled_print(buf_oled);
			
			sprintf(buf_oled, "Analog:   %d", adc_value);				// Mostra o valor atual lido do ADC
			oled_set_cursor(7, 2); oled_print(buf_oled);
			
			update_OLED = 0;													// Reseta a flag de atualiza��o do OLED
		}
		
	}
}