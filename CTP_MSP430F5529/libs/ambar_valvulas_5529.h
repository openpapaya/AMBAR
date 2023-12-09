/*
 * timer.h
 *
 *  Created on: 5 de jul de 2021
 *      Author: guiwz
 */

#ifndef __Valvulas_Operador_H
#define __Valvulas_Operador_H

typedef enum {abrir, fechar} tarefa;
typedef enum {T1INV, T2INV, T1OUTV, T2OUTV} componente;

void valveControl( componente valvula, tarefa acao );
void i2cValveControl(  componente valvula, tarefa acao );
void i2cReleControl(  componente valvula, tarefa acao );
void PCF8574_Valves(  uint8_t inputs74HC139);
//void i2c3to8Decoder( uint8_t addr, uint8_t select);
void testesAmbar5529();

#endif

