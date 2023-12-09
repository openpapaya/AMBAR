/*
 * I2C.h
 *
 *  Created on: 5 de jul de 2021
 *      Author: guiwz
 */

#ifndef __I2C_H_
#define __I2C_H_

#include <stdint.h>

//                              *** ENDERECOS I2C ***
// PCF8574 - CONTROLE DAS VALVULAS  - Endereco: 0x20
// PCF8574 - SENSORES DE NIVEL      - Endereco: 0x21
// PCF8574 - LCD                    - Endereco: 0x27
// INA219  - Leitor Tensao/Corrente - Endereco: 0x40
// ESP32   - Leitor Tensao/Corrente - Endereco: 0x42

#define     SDA     4,1
#define     SCL     4,2


// Operações de linha
    uint8_t i2cWrite_UCB1(uint8_t addr, uint8_t data, uint8_t size);
    uint8_t i2cWrite_UCB0(uint8_t addr, uint8_t * data, uint8_t size);

    typedef enum {pcf8574_Snivel, lcd} disp;
    void i2cRead_UCB1 (uint8_t addr);
    uint8_t i2cWriteByte_UCB1(uint8_t addr, uint8_t byte);
    uint8_t i2cWriteByte_UCB0(uint8_t addr, uint8_t byte);
    void i2cConfig(void);
    char adr;                               //Endereço do escravo



#endif // __I2C_H_
