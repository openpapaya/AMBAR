/*
 * funcoes_ina219.h
 *
 *  Created on: Mar 1, 2023
 *      Author: guiwz
 */

#ifndef LIBS_FUNCOES_INA219_5529_H_
#define LIBS_FUNCOES_INA219_5529_H_
// INA219 variaveis
typedef enum {shuntvoltage, busvoltage, power, current} registerset;
typedef enum {powerdown, shunttrig, bustrig, shuntandbustrig, adcoff, shuntvolt, busvolt, shuntandbusvolt} comando;
    // 000: Power-Down      mode reduces the quiescent current and turns off current into INA219 inputs, avoiding any supply drain.
    // 001: Shunt voltage, triggers a single-shot conversion  (even if already programmed)
    // 010: Bus voltage, triggers a single-shot conversion  (even if already programmed)
    // 011: Shunt and bus, triggered
    // 100: ADC off. Stops all conversions.
    // 101: Convert current continuously
    // 110: Convert voltage continuously
    // Normal operatin mode (MODE bits of the Conf.reg. = 111). It continuously convertes the shunt voltage up to the number set in the
    // 111:shunt voltage averaging function( Conf.reg., SADC bits). The device then converts the bus voltage up to the number set in the bus
volatile float shunt_voltage;
volatile uint16_t shuntVolt_reg_word;
volatile uint8_t shuntVolt_reg_byte[2];     //

volatile uint16_t bus_voltage;
volatile uint16_t busVolt_reg_word;
volatile uint8_t busVolt_reg_byte[2];     //

volatile float power_value;
volatile uint16_t power_reg_word;     //
volatile uint8_t power_reg_bytes[2];

volatile float current_value;
volatile uint16_t current_reg_word;     //
volatile uint8_t current_reg_bytes[2];
// Funcoes
uint16_t ina219calibration(uint8_t addr);
//uint16_t ina219config( uint8_t addr, uint8_t bit15RST, uint8_t bit13brng, uint8_t bit12e11pg, uint8_t bit10e9e8e7badc, uint8_t bit6e5e4e3sadc, uint8_t bit2e1e0mode  );
uint16_t ina219config( uint8_t addr, uint16_t configRegister );
void ina219mode( uint8_t addr, comando mode);
uint16_t ina219readings( uint8_t addr,registerset readings);
void ina219Print(uint8_t addr, uint8_t d);





#endif /* LIBS_FUNCOES_INA219_5529_H_ */
