/*
 * lcd.h
 *
 *  Created on: 9 de jul de 2021
 *      Author: guiwz
 */

#define LCD 0x27                            // Endere�o do LCD

#ifndef LIBS_LCD_OperadorAmbar_H_
#define LIBS_LCD_OperadorAmbar_H_

#include <stdint.h>


typedef enum {defaut, errodecmd, confirmacaocmd, menucmd, senhacentral,senhaincorreta, menu,  opcaoinstrucomposttea, instrucomposttea, opcaocomposttea, composttea, iniciocomposttea,
             opcaofertirrigacao, instrufertirrigacao, iniciopferti, noactsensorestanque, sensorestanque, noactsensoresar, sensoresar,
             noactconsumodeagua, consumodeagua, erroT1, erroT2, erroT1T2, v1inabrindo, v1infechando, v1outabrindo, v1outfechando, v2inabrindo, v2infechando,
             v2outabrindo, v2outfechando, pumpNutOn, prepSend, prepOk, initPrep,tnutpumpabrindo, tnutpumpfechando} mostrarNoLCD;
typedef enum {T1, T2} tanque;
typedef enum {compostTea, EVAP, regandoTempo, estimativaTempo} processo;
typedef enum {temp, ina219rd} grandeza;

void lcdLayout(uint8_t addr, mostrarNoLCD layout);
void lcdTimer(uint8_t addr,tanque numero, processo tempo);
void lcdCmd(uint8_t addr,uint8_t byte, uint8_t  instr0char1);                 // Funï¿½ï¿½o que prepara os dois nibbles que serï¿½o os 8 bits de instruï¿½ï¿½o
void lcdCursor(uint8_t addr,uint8_t cPosit);                                 // Funï¿½ï¿½o que recebe do usuï¿½rio a posiï¿½ï¿½o desejada do cursor
void lcdClear(uint8_t addr,uint8_t cPosit);
void lcdNum(uint8_t addr,char x);
void lcdFloatNum(uint8_t addr,float v, grandeza tipo);
void lcdNumVir3(uint8_t addr,uint16_t v);
void lcdNumVir2(uint8_t addr,uint16_t b);
void lcdNumbers_Hour(uint8_t addr,char x);
void lcdNumbers_Minute(uint8_t addr,char x);
void lcdPrint(uint8_t addr,uint8_t *str);
void lcdPrintNumb(uint8_t addr,uint8_t *string);
void lcdWriteNibble(uint8_t addr, uint8_t nibble, uint8_t instr0char1);

uint8_t i2cWriteByte_UCB0(uint8_t addr, uint8_t byte);


#endif /* LIBS_LCD_OperadorAmbar_H_ */
