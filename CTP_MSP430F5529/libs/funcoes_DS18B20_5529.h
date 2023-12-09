/*
 * sensor_DS18B20.h
 *
 *  Created on: 28 de nov de 2021
 *      Author: guiwz
 */

#ifndef LIBS_FUNCOES_DS18B20_5529_H_
#define LIBS_FUNCOES_DS18B20_5529_H_


#define DS_CONVERT_T                    0x44
#define DS_READ_SCRATCH                 0xBE
#define WRITE_SCRATCHPAD                0x4E
#define COPY_SCRATCHPAD                 0x48
#define RECALL_E2                       0xB8
#define READ_POWER_SUPPLY               0xB4
#define DS_SKIP_ROM                     0xCC

// 1-wire por bit-banging
// ------------ 70 us é o intervalo de tempo de cada transmissão de bits ----------------------------
// EXEMPLOS:                        Enviar 0                        Enviar 1
//                  DQ low por 60 us + delay de 10 us       DQ low por 6 us + delay de 64 us

// -------------------- Enviar um RESET para a linha -------------------------------
//  DQ low + 480 us delay + Soltar DQ + 70 us delay + Sample bus ( 0 = device present, 1 = no part) + 410 us delay

//
// declarar uma funcao do tipo "inline"
//Em C existe o recurso de se declarar uma função do tipo inline. Uma função deste tipo,
//na verdade, não é usada como uma função. Ela se parece mais com uma Macro e ao
//invés de colocar uma chamada, o compilador substitui a linha que a chama pela função
//completa. Ao rodarmos o programa listado abaixo, vamos obter exatamente o resultado
//do Experimento 2, mas com a vantagem de que o programa ficou mais legível.


void oneWireConfig();

// Funções para 1-Wire
char ow_rst(void);
inline char ow_pin_rd (void) {return (P8IN & BIT1) >> 1; }  //Ler P8.1
inline void ow_pin_low(void) {P8DIR |= BIT1; }              //P8.1 = Zero
inline void ow_pin_hiz(void) {P8DIR &= ~BIT1; }             //P8.1 = Hi-Z

// Operações de 1 bit
void    writeBit_1W( uint8_t bit);
uint8_t    reset_1W();
uint8_t    readBit_1W();

// Operações de vários bits
void    writeByte_1W( uint8_t value);
uint8_t readByte_1W();
float readTemp_1W();
uint8_t read_temp();

#endif /* LIBS_FUNCOES_DS18B20_5529_H_ */
