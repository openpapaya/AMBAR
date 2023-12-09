/*
 * uartAmbar5529.h
 *
 *  Created on: 6 de fev de 2023
 *      Author: user
 */

#ifndef LIBS_UARTAMBAR5529_H_
#define LIBS_UARTAMBAR5529_H_

#include    <msp430.h>
#include    <stdint.h>
#include    <stdio.h>
#include    <math.h>
#include    <stdlib.h>
#include    <string.h>


// ----------------- UART --------------------------------

// Ver Tabela ASCII
// Retorno do carro: 0x0D
//           Espaco: 0x20
// Variaveis globais
//uint8_t comando   = 0;
//uint8_t flagEnter = 0;
extern volatile unsigned int lcd_T1Tool;
extern volatile unsigned int lcd_T2Tool;
extern volatile unsigned int flagTestes;
//volatile unsigned int vMed1Int = 0;
//volatile unsigned int vMed2Int = 0;
//volatile unsigned int v1Mn = 9999;
//volatile unsigned int v2Mn = 9999;
//volatile unsigned int v1Mx = 0;
//volatile unsigned int v2Mx = 0;
//volatile unsigned int cccc = 0;
volatile char stringDateYr  [4];
volatile char stringDateMnt [2];
volatile char stringDateDay [2];
volatile char stringHour  [8];
volatile char stringT1L   [10];
volatile char stringT2L   [10];
volatile char stringT1pH  [3];
volatile char stringT2pH  [3];
volatile char stringT1DO  [3];
volatile char stringT2DO  [3];
volatile char stringT1EC [5];
volatile char stringT2EC [5];
volatile char stringT1TDS [4];
volatile char stringT2TDS [4];
volatile char stringT1temp [10];
volatile char stringT2temp [10];
volatile char strTest [20];

void uartOpen(uint8_t interface);           // Solititar comunicacao serial pelo. P4.5 = UCA1RXD. P4.4 = UCA1TXD
void uartRead();
void uartWrite(volatile char * str);                 // Escreve string no registrador UCA1TXBUF
void uartWrite_UCA0(volatile char * str);                 // Escreve string no registrador UCA0TXBUF
void erroDeCmdUart();
void uartMonitorAmbar();            // Escreve string no registrador UCA1TXBUF
void uartWriteFails(volatile char * str, uint8_t strlen);
void uartWriteFails_UCA0(volatile char * str, uint8_t strlen);
void uart_operador_ambar5529();
// Escreve string no registrador UCA1TXBUF
typedef enum { ina2190x40, contadorCCCC,data, dia, mes, ano, tanque1L, tempint, phT1, doT1, ecT1, tdsT1, tanque2L, phT2, doT2, ecT2, tdsT2} dataType;
void inteiroParaString(uint16_t v, dataType varName);
void floatParaString(float v, dataType varName);
void ina219_FloatParaString(float v);

#define BUFFER_SIZE 16
struct
{
    uint8_t buffer[BUFFER_SIZE];
    uint8_t read, write, size,i,refresh;
}  rx;
//#define ENTER_SIZE 4                        // Reservando

//    uint8_t buffer[ENTER_SIZE];
    uint8_t modes;



#endif /* LIBS_UARTAMBAR5529_H_ */
