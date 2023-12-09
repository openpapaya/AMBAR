/*
 * timer.h
 *
 *  Created on: 5 de jul de 2021
 *      Author: guiwz
 */
//#include <msp430.h>

#ifndef __TIMER_H
#define __TIMER_H

#include <stdint.h>
#include "libs/funcoes_PCF8574_5529.h"

//#define MAX_TIMERS 10

//#define         regIn_1      0x0010         //Referenciando 0x0010 como "regIn_1", equivalente ao PIN4 para a porta P1,  ou 0b00010000 (76543210)
//#define         regIn_2      0x0020         //Referenciando 0x0002 como "regIn_2", equivalente ao PIN5 para a porta P1,  ou 0b00100000 (76543210)

typedef enum {us, ms, sec, min} timeunit_t;


void ta0Config(void);                      //Fun��o que configura o Timer A0.
//uint16_t timerNew(uint32_t time, timeunit_t unit);
//uint16_t timerIsRunning(uint16_t timer);

void wait( uint16_t time, timeunit_t unit);
//void TA1_buzzers();

#endif

