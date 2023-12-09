/*
 * gpio.h
 *
 *  Created on: 30 de jun de 2021
 *      Author: guiwz
 */

#ifndef LIBS_IOCONFIG_H_
#define LIBS_IOCONFIG_H_

#include <stdint.h>


typedef enum {input, output, inPullUp, inPullDown, unusedPin} pinMode_t;

void gpioConfig();                       //Função que configura as entradas e saídas (GPIO)
void pinMode(uint8_t port, uint8_t bit, pinMode_t mode );   // função para enviar instruções ao LCD

void    pinWrite (uint8_t port, uint8_t bit, uint8_t value);
void    pinMode (uint8_t port, uint8_t bit, pinMode_t mode);
uint8_t pinRead (uint8_t port, uint8_t bit);



#endif /* LIBS_IOCONFIG_H_ */

