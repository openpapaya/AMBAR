/*
 * lcd.h
 *
 *  Created on: 9 de jul de 2021
 *      Author: guiwz
 */

#define LCD 0x27                            // Endere�o do LCD

#ifndef LIBS_LCDi2c5529_H_
#define LIBS_LCDi2c5529_H_

#include <stdint.h>

//                              *** ENDERECOS I2C ***
// PCF8574 - CONTROLE DAS VALVULAS  - Endereco: 0x20
// PCF8574 - SENSORES DE NIVEL      - Endereco: 0x21
// PCF8574 - LCD1                   - Endereco: 0x27
// PCF8574 - LCD2                   - Endereco: 0x26
// INA219  - Leitor Tensao/Corrente - Endereco: 0x40

#define CHAR    1
#define INSTR   0

#define BL  BIT3                            // Backlight
#define EN  BIT2                            // Enable
#define RW  BIT1                            // R/~W. Read and Write
#define RS  BIT0                            // Instru��o ou caractere


void lcdInit(uint8_t addr);
void lcdWriteByte(uint8_t addr, uint8_t byte, uint8_t instr0char1);   // fun��o para enviar instru��es ao LCD
//void lcdCmd(uint8_t byte, uint8_t  instr0char1);                 // Fun��o que prepara os dois nibbles que ser�o os 8 bits de instru��o
//void lcdCursor(uint8_t cPosit);                                 // Fun��o que recebe do usu�rio a posi��o desejada do cursor
//void lcdClear(uint8_t cPosit);
//void lcdNum(char x);
//void lcdNumbers_Hour(char x);
//void lcdNumbers_Minute(char x);
//void lcdPrint(uint8_t *str);
//void lcdPrintNumb(uint8_t *string);


#endif /* LIBS_LCDi2c5529_H_ */
