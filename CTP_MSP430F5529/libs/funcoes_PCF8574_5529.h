/*
 * funcoes_PCF8574_5529.h
 *
 *  Created on: Mar 5, 2023
 *      Author: guiwz
 */

#ifndef LIBS_FUNCOES_PCF8574_5529_H_
#define LIBS_FUNCOES_PCF8574_5529_H_

//                              *** ENDERECOS I2C ***
// PCF8574 - CONTROLE DAS VALVULAS  - Endereco: 0x20
// PCF8574 - SENSORES DE NIVEL      - Endereco: 0x21
// PCF8574 - LCD                    - Endereco: 0x27
// INA219  - Leitor Tensao/Corrente - Endereco: 0x40

#define ADDR_SNIVEL  0X21

uint8_t read_PCF8574;     //
uint8_t read_PCF8574_byte[2];

uint8_t readPCF8574(uint8_t addr);
uint8_t writePCF8574(uint8_t addr, uint8_t byte );



#endif /* LIBS_FUNCOES_PCF8574_5529_H_ */
