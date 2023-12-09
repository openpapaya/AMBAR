/*
 * funcoes_ina219.c
 *
 *  Created on: Mar 1, 2023
 *      Author: guiwz
 */

#include <msp430.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#include "libs/config_gpio_5529.h"
#include "libs/config_i2c_5529.h"
#include "libs/config_timers_5529.h"
#include "libs/ambar_ultrassom_5529.h"
#include "libs/ambar_lcd_5529.h"
#include "libs/ambar_uart_5529.h"
#include "libs/ambar_timers_5529.h"
#include "libs/ambar_valvulas_5529.h"
#include "libs/funcoes_ina219_5529.h"

#define     PCF8574_LCD1        0x27
#define     PCF8574_LCD2        0x26
// I2C Slave Address of the INA219 Current Meter Chip
#define INA219_ADDR 0x40
// Register Addresses of the INA219 Current Meter Chip
#define INA219_REG_CONFIG      0x00
#define INA219_REG_SHUNT_VOLT  0x01
#define INA219_REG_BUS_VOLT    0x02
#define INA219_REG_POWER       0x03
#define INA219_REG_CURRENT     0x04
#define INA219_REG_CALIBRATION 0x05

// Configuration Register Settings of the INA219 Current Meter Chip
#define INA219_CONFIG_MODE_SHUNT_BUS_CONT 0x0F08
#define INA219_CONFIG_MODE_SHUNT_CONT     0x0D08
#define INA219_CONFIG_MODE_BUS_CONT       0x0B08
#define INA219_CONFIG_MODE_POWER_DOWN     0x0100


uint16_t alarm_temp;
uint16_t x;
uint16_t tempSensor;
extern volatile int tempCelsius_int;
extern volatile char ina219 [5];

void ina219Print(uint8_t addr, uint8_t d)
{

    volatile static uint8_t current_value_old;
    volatile static uint8_t power_value_old;
    volatile static uint8_t bus_voltage_old;

    ina219readings(0x40, shuntvoltage );
    ina219readings(0x40, busvoltage );
    ina219readings(0x40, power);
    ina219readings(0x40, current);

    if( d == 2)     // Print somente no UART
    {
//        // Mostrar medicoes no terminal (UART)
//        // uartWrite("Tensão12- Corrente -12Potência\n\r");
//        //           v,vvv V123a,aaa A12345w,www W
//        uartWrite("Tensão  - Corrente -  Consumo \n\r");
//        inteiroParaString(bus_voltage, ina2190x40);            // Converte o inteiro "bus_voltage" para o formado string que do uart
//        uartWriteFails(ina219,5); uartWrite(" V   ");           // Transmite caracterer por caractere pelo uart
//        floatParaString(current_value, ina2190x40);          // P
//        uartWriteFails(ina219,5); uartWrite(" mA    ");
//        floatParaString(power_value, ina2190x40);          // P
//        uartWriteFails(ina219,5); uartWrite(" mW");
//        uartWrite("\n");
//        uartWrite("\r");
    }
    else if(d == 3) // Print no LCD e UART
    {
        //   Visor do LCD:
        //   0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F 0x10 0x11 0x12 0x13
        //   0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F 0x50 0x51 0x52 0x53
        //   0x14 0x15 0x16 0x17 0x18 0x19 0x1A 0x1B 0x1C 0x1D 0x1E 0x1F 0x20 0x21 0x22 0x23 0x24 0x25 0x26 0x27
        //   0x54 0x55 0x56 0x57 0x58 0x59 0x5A 0x5B 0x5C 0x5D 0x5E 0x5F 0x60 0x61 0x62 0x63 0x64 0x65 0x66 0x67
       // if(!((int)current_value_old == (int)current_value))
       // {   // Para o LCD so atualizar o valor de medicao se o mesmo se alterar
          //  lcdCursor(PCF8574_LCD1, 0x1E);
          //  wait(1,ms);
          //  lcdFloatNum(PCF8574_LCD1, current_value, ina219rd);                     // Imprimir no LCD o valor da medicao de corrente
          //  wait(1,ms);
            //XXXXXXXXXXXXXXXXXXXX
           // lcdPrint(PCF8574_LCD1," mA    ");
           // wait(1,ms);
           // current_value_old == current_value;               // Salva em "current_value_old" o valor da medicao atual
       // }
        if(!(power_value_old == power_value))
        {              // Para o LCD so atualizar o valor de medicao se o mesmo se alterar
            lcdCursor(PCF8574_LCD1,0x5E);                       // Pular para a posicao 0x5C do visor do LCD
            lcdFloatNum(PCF8574_LCD1, power_value, ina219rd);   // Imprima no LCD o valor da medicao de potencia
            lcdCursor(PCF8574_LCD1,0x63);                       //XXXXXXXXXXXXXXXXXXXX
            lcdPrint(PCF8574_LCD1,"mW");
            //"POWER:        mW   "
            power_value_old == power_value;                   // Salva em "power_value_old" o valor da medicao atual
        }
        // Mostrar medicoes no terminal (UART)
        // uartWrite("Tensão12- Corrente -12Potência\n\r");
        //           v,vvv V123a,aaa A12345w,www W
//        uartWrite("Tensão  - Corrente -  Consumo \n\r");
//        ina219_FloatParaString(bus_voltage);            // Converte o inteiro "bus_voltage" para o formado string que do uart
//        uartWriteFails(ina219,5); uartWrite(" V   ");           // Transmite caracterer por caractere pelo uart
//        ina219_FloatParaString(current_value);          // P
//        uartWriteFails(ina219,5); uartWrite(" A    ");
//        ina219_FloatParaString(power_value);          // P
//        uartWriteFails(ina219,5); uartWrite(" W");
//        uartWrite("\n");
//        uartWrite("\r");
    }
    else if(d == 4) // Corrente e consumo, print no LCD e UART
    {
        //  Visor do LCD:
        //  0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F 0x10 0x11 0x12 0x13
        //  0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F 0x50 0x51 0x52 0x53
        //  0x14 0x15 0x16 0x17 0x18 0x19 0x1A 0x1B 0x1C 0x1D 0x1E 0x1F 0x20 0x21 0x22 0x23 0x24 0x25 0x26 0x27
        //  0x54 0x55 0x56 0x57 0x58 0x59 0x5A 0x5B 0x5C 0x5D 0x5E 0x5F 0x60 0x61 0x62 0x63 0x64 0x65 0x66 0x67
        if(!(current_value_old == current_value))
        {   // Para o LCD so atualizar o valor de medicao se o mesmo se alterar
            lcdCursor(PCF8574_LCD1, 0x1E);
            lcdFloatNum(PCF8574_LCD1, current_value, ina219rd);                     // Imprimir no LCD o valor da medicao de corrente
           // XXXXXXXXXXXXXXXXXXXX
            lcdPrint(PCF8574_LCD1," mA  ");
            current_value_old == current_value;             // Salva em "current_value_old" o valor da medicao atual
        }
        if(!(power_value_old == power_value)){              // Para o LCD so atualizar o valor de medicao se o mesmo se alterar
            lcdCursor(PCF8574_LCD1,0x5E);                                // Pular para a posicao 0x5C do visor do LCD
            lcdFloatNum(PCF8574_LCD1, power_value, ina219rd);                       // Imprima no LCD o valor da medicao de potencia
            lcdPrint(PCF8574_LCD1," mW ");
            //"POWER:        mW   "
            power_value_old == power_value;               // Salva em "power_value_old" o valor da medicao atual
        }
//        if(!(bus_voltage_old == bus_voltage)){              // Para o LCD so atualizar o valor de medicao se o mesmo se alterar
//            lcdCursor(addr,0x5D);                                // Pular para a posicao 0x5C do visor do LCD
//            lcdFloatNum(power_value, ina219rd);                       // Imprima no LCD o valor da medicao de potencia
//            lcdPrint(addr," mW");
//            power_value_old == current_value;               // Salva em "power_value_old" o valor da medicao atual
    }
    ina219mode( 0x40, powerdown);    // 000: Power-Down mode reduces the quiescent current and turns off
}

// *********************************************************************************************
uint16_t ina219readings(uint8_t addr, registerset readings)                   // Essa funcao opera os quatro registradores que armazenam os resultados das medicoes
{                                           // Shunte Voltage, Bus Voltage, Power e Current

    // -----------------------------------------------------------------------------------------------------------------------------------
    //  POINTER |  REGISTER  |                               FUNCTION                                       |   POWER-ON-RESET   |  TYPE
    //    ADDR      NAME
    //    0x00
    //    0x01   Shunt Volt.                     Shut voltage measurement data.                                 Shunt voltage        R
    //    0x02    Bus Volt.                      Bus voltage measuremente data.                                  Bus voltage         R
    //    0x03     Power¹                         Power measuremente data.                                    00000000 00000000     R
    //    0x04    Current¹                   Contains the value of the current flowing through                00000000 00000000     R
    //    0x05
    //                           system calibration
    //   (1)    The Power register and Current register default to 0 because the Calibration register defaults to 0, yielding a zero
    //          current value until the Calibration register is programmed.
    // -----------------------------------------------------------------------------------------------------------------------------------
    // Current Register eh internamente calculado seguindo a equacao 4: Current Register = (Shunt Voltage Register x Calibration Register)/4096
    // Power Register eh internamente calculado seguindo a equacao 5:   Power Register = (Current Register x Bus Voltage Register)/5000
//    volatile uint16_t shuntVolt_reg_word;
//    volatile uint8_t shuntVolt_reg_byte[2];     //
//
//    volatile uint16_t busVolt_reg_word;
//    volatile uint8_t busVolt_reg_byte[2];     //
//
//    volatile uint16_t power_reg_word;     //
//    volatile uint8_t power_reg_bytes[2];
//
//    volatile uint16_t current_reg_word;     //
//    volatile uint8_t current_reg_bytes[2];
 //   __disable_interrupt();
    volatile static int i = 0;
    UCB1IFG = 0;                            // Boa pratica -  Zerar o registro de flags antes de comeCar
    UCB1I2CSA = addr;                       // Confingura o endereco do escravo
    UCB1CTL1 |= UCTR;                       // Modo transmissor
    if(UCB1STAT & UCBBUSY);                 // Espera a linha estar desocupada ( Aprimorar esse codigo para que o programa n fique parado esperando)
    {
        int i = 1000;
        while(i)
        {
            i--;
            if(!(UCB1STAT & UCBBUSY)){
                i = 0;
            }
        }
    }
    // -------------- Shunt Voltage ---------------------
    if( readings == shuntvoltage)
    {
        // ATENCAO: Eh configurada de acordo com o PGA do registrador de configuracao.
        // Atentar-se nos seus valores para cada configuracao de PGA
        // Se estiver ocupada retorna para outras operacoes e volta depoisde um tempo
        UCB1CTL1 |= UCTXSTT;                    // Requisita o inicio da comunicacao com TX. UCTXSTT gera o START
        while(!(UCB1IFG & UCTXIFG));            // UCTXIFG eh setado apos o START acontecer e o byte de dados puder ser escrito na UCB1TXBUF
        UCB1TXBUF = INA219_REG_SHUNT_VOLT;      //

        while( UCB1CTL1 & UCTXSTT );            // Espera o ciclo de ACK/NACK acontecer
        if(UCB1IFG & UCNACKIFG)                 // Se o escravo nao estiver presente
        {                                       // vou receber um NACK
            UCB1CTL1 |= UCTXSTP;                // Sou obrigado entao a transmitir um STOP
            while (UCB1CTL1 & UCTXSTP);         // Espero o stop ser de fato transmitido
            return 1;                           // e retorno um codigo de erro
            // COLOCAR aqui um LED de alerta para o erro
        }
        else if( UCB1IFG & UCTXIFG)             // Teve acknowledge
        {
            //wait(5, us);                        // When SCL using SCL freq. in excess of 1 MHz, register contents are updated 4 us after completion of the write command. Therefore,
            UCB1IFG = 0;                        // a 4-us delay is required between completion of a write to a given register and a subsequent read
            UCB1I2CSA = addr;                   // Confingura o endereco do escravo// Boa pratica -  Zerar o registro de flags antes de comeCar
            UCB1CTL1 &= ~UCTR;                  // Modo receptor
            UCB1CTL1 |= UCTXSTT;                // Requisita o inicio da comunicacao com TX. UCTXSTT gera o START
//            while(!(UCB1IFG & UCRXIFG));        // Quando UCBRXIFG flag levantar, o primeiro byte vindo do dispositivo foi recebido, lembrar da bandeira levantada na caixa de correio
            wait(1,ms);
            shuntVolt_reg_byte[1] = UCB1RXBUF; //
            // Ver o resultado de enviar um NACK ao inves de STOP
            UCB1CTL1 |= UCTXSTP;                    // Parar a comunicacao
//            while(!(UCB1IFG & UCRXIFG));
            wait(1,ms);
            shuntVolt_reg_byte[0] = UCB1RXBUF;     //
            shuntVolt_reg_word = (shuntVolt_reg_byte[1] << 8 | shuntVolt_reg_byte[0]);
            shunt_voltage = 10 * shuntVolt_reg_word;            // 10 uV/bit. Multiplico apenas por 10, mas na hora de mostrar no LCD e no terminal aplico o 10^-6
        }
  //      __enable_interrupt();
        return shunt_voltage;
    }
    // -------------- Bus Voltage ---------------------
    else if( readings == busvoltage)
    {
        // Se estiver ocupada retorna para outras operacoes e volta depoisde um tempo
        UCB1CTL1 |= UCTXSTT;                    // Requisita o inicio da comunicacao com TX. UCTXSTT gera o START

        while(!(UCB1IFG & UCTXIFG));            // UCTXIFG eh setado apos o START acontecer e o byte de dados puder ser escrito na UCB1TXBUF
        UCB1TXBUF = INA219_REG_BUS_VOLT;        //

        while( UCB1CTL1 & UCTXSTT );            // Espera o ciclo de ACK/NACK acontecer
        if(UCB1IFG & UCNACKIFG)                 // Se o escravo nao estiver presente
        {                                       // vou receber um NACK
            UCB1CTL1 |= UCTXSTP;                // Sou obrigado entao a transmitir um STOP
            while (UCB1CTL1 & UCTXSTP);         // Espero o stop ser de fato transmitido
            return 1;                           // e retorno um codigo de erro
            // COLOCAR aqui um LED de alerta para o erro
        }
        else if( UCB1IFG & UCTXIFG)             // Teve acknowledge
        {
            //wait(5, us);                        // When SCL using SCL freq. in excess of 1 MHz, register contents are updated 4 us after completion of the write command. Therefore,
            UCB1IFG = 0;                        // a 4-us delay is required between completion of a write to a given register and a subsequent read
            UCB1I2CSA = addr;                   // Confingura o endereco do escravo// Boa pratica -  Zerar o registro de flags antes de comeCar
            UCB1CTL1 &= ~UCTR;                  // Modo receptor
            UCB1CTL1 |= UCTXSTT;                // Requisita o inicio da comunicacao com TX. UCTXSTT gera o START
 //           while(!(UCB1IFG & UCRXIFG));        // Quando UCBRXIFG flag levantar, o primeiro byte vindo do dispositivo foi recebido, lembrar da bandeira levantada na caixa de correio
            wait(1,ms);
            busVolt_reg_byte[1] = UCB1RXBUF; //
            // Ver o resultado de enviar um NACK ao inves de STOP
            UCB1CTL1 |= UCTXSTP;                    // Parar a comunicacao
//            while(!(UCB1IFG & UCRXIFG));
            wait(1,ms);
            busVolt_reg_byte[0] = UCB1RXBUF;     //
            busVolt_reg_word = (busVolt_reg_byte[1] << 8 | busVolt_reg_byte[0]);
            busVolt_reg_word = busVolt_reg_word >> 3;     // The bus voltage is internally measured at the IN– pin to calculate the voltage level delivered to the load. The Bus
                                                          // Voltage register bits are not right-aligned; therefore, they must be shifted right by three bits. Multiply the shifted
                                                          // contents by the 4-mV LSB to compute the bus voltage measured by the device in volts
            bus_voltage = 4 * busVolt_reg_word;           // Bus voltage, 1 LSB step size for 12 bits ADC conversion = 4 mV
        }
      //  __enable_interrupt();
        return bus_voltage;
    }
    // -------------- Power ---------------------
    else if( readings == power)
    {
        // Se estiver ocupada retorna para outras operacoes e volta depoisde um tempo
        UCB1CTL1 |= UCTXSTT;                    // Requisita o inicio da comunicacao com TX. UCTXSTT gera o START

        while(!(UCB1IFG & UCTXIFG));            // UCTXIFG eh setado apos o START acontecer e o byte de dados puder ser escrito na UCB1TXBUF
        UCB1TXBUF = INA219_REG_POWER;           //

        while( UCB1CTL1 & UCTXSTT );            // Espera o ciclo de ACK/NACK acontecer
        if(UCB1IFG & UCNACKIFG)                 // Se o escravo nao estiver presente
        {                                       // vou receber um NACK
            UCB1CTL1 |= UCTXSTP;                // Sou obrigado entao a transmitir um STOP
            while (UCB1CTL1 & UCTXSTP);         // Espero o stop ser de fato transmitido
            return 1;                           // e retorno um codigo de erro
            // COLOCAR aqui um LED de alerta para o erro
        }
        else if( UCB1IFG & UCTXIFG)             // Teve acknowledge
        {
            //wait(5, us);                      // When SCL using SCL freq. in excess of 1 MHz, register contents are updated 4 us after completion of the write command. Therefore,
            UCB1IFG = 0;                        // a 4-us delay is required between completion of a write to a given register and a subsequent read
            UCB1I2CSA = addr;                   // Confingura o endereco do escravo// Boa pratica -  Zerar o registro de flags antes de comeCar
            UCB1CTL1 &= ~UCTR;                  // Modo receptor
            UCB1CTL1 |= UCTXSTT;                // Requisita o inicio da comunicacao com TX. UCTXSTT gera o START
            //while(!(UCB1IFG & UCRXIFG));      // Quando UCBRXIFG flag levantar, o primeiro byte vindo do dispositivo foi recebido, lembrar da bandeira levantada na caixa de correio
            wait(1,ms);
            power_reg_bytes[1] = UCB1RXBUF;     //
            // Ver o resultado de enviar um NACK ao inves de STOP
            UCB1CTL1 |= UCTXSTP;                // Parar a comunicacao
            //while(!(UCB1IFG & UCRXIFG));
            wait(1,ms);
            power_reg_bytes[0] = UCB1RXBUF;     //
            power_reg_word = (power_reg_bytes[1] << 8 | power_reg_bytes[0]);
            // "Power Register content is multiplied by Power LSB which is 20 times the Current_LSB for a power value in watts" . Current_LBS é 20 uA/bit
            // --> power_value = 20*20*10^-6 * power_reg_word = 0.4396 W ( Exemplo utilizando power_reg_word = 1099)
//            power_value    = 4*power_reg_word; // 20*20*10^6 = 4*10^-4. Por isso multiplico por 4, e os 10^4 deixo para trabalhar na hora de escrever no display
//            power_value    = power_value/10000;
            power_value = power_reg_word;
        }
     //   __enable_interrupt();
        return power_value;
    }
    // -------------- Current ---------------------
    else if( readings == current)
    {
        // Se estiver ocupada retorna para outras operacoes e volta depoisde um tempo
        UCB1CTL1 |= UCTXSTT;                    // Requisita o inicio da comunicacao com TX. UCTXSTT gera o START

        while(!(UCB1IFG & UCTXIFG));            // UCTXIFG eh setado apos o START acontecer e o byte de dados puder ser escrito na UCB1TXBUF
        UCB1TXBUF = INA219_REG_CURRENT;         //

        while( UCB1CTL1 & UCTXSTT );            // Espera o ciclo de ACK/NACK acontecer
        if(UCB1IFG & UCNACKIFG)                 // Se o escravo nao estiver presente
        {                                       // vou receber um NACK
            UCB1CTL1 |= UCTXSTP;                // Sou obrigado entao a transmitir um STOP
            while (UCB1CTL1 & UCTXSTP);         // Espero o stop ser de fato transmitido
            return 1;                           // e retorno um codigo de erro
            // COLOCAR aqui um LED de alerta para o erro
        }
        else if( UCB1IFG & UCTXIFG)             // Teve acknowledge
        {
            //wait(5, us);                        // When SCL using SCL freq. in excess of 1 MHz, register contents are updated 4 us after completion of the write command. Therefore,
            UCB1IFG = 0;                        // a 4-us delay is required between completion of a write to a given register and a subsequent read
            UCB1I2CSA = addr;                   // Confingura o endereco do escravo// Boa pratica -  Zerar o registro de flags antes de comeCar
            UCB1CTL1 &= ~UCTR;                  // Modo receptor
            UCB1CTL1 |= UCTXSTT;                // Requisita o inicio da comunicacao com TX. UCTXSTT gera o START
//            while(!(UCB1IFG & UCRXIFG));        // Quando UCBRXIFG flag levantar, o primeiro byte vindo do dispositivo foi recebido, lembrar da bandeira levantada na caixa de correio
            wait(1,ms);
            current_reg_bytes[1] = UCB1RXBUF; //
            // Ver o resultado de enviar um NACK ao inves de STOP
            UCB1CTL1 |= UCTXSTP;                    // Parar a comunicacao
            //while(!(UCB1IFG & UCRXIFG));
            wait(1,ms);
            current_reg_bytes[0] = UCB1RXBUF;     //
            current_reg_word = (current_reg_bytes[1] << 8 | current_reg_bytes[0]);
            // To obtain a value in amperes the Current register value is multiplied by the programmed Current_LSB.
            // current_value = Current_LSB * current_reg_word = 20*10^-6 * current_reg_word
            current_value = current_reg_word/20;
            //current_value = current_value/100000;
        }
     //   __enable_interrupt();
        return current_value;
    }
    return 1;
}
// ************************************************************************************************************************************
uint16_t ina219calibration(uint8_t addr)                 // Calibration Register (address = 05h)[reset = 00h]
{
    // uint16_t ina219calibration(uint8_t rshunt, uint8_t maxcurrent);
    // ina219calibration:  Sets full-scale range and LSB of current and power measurements. Overall system calibration
    // Current and power calibration are set by bits FS15 to FS1 of the Calibration register
    // 15     14      13      12     11    10    9      8     7      6     5      4      3      2      1    0
    // FS15  FS14    FS13    FS12   FS11  FS10  FS9    FS8   FS7    FS6   FS5    FS4    FS3    FS2    FS1  FS0
    // RW-0  RW-0    RW-0   RW-0    RW-0  RW-0  RW-0  RW-0   RW-0   RW-0  RW-0   RW-0   RW-0  RW-0   RW-0  R-0 ==0000: VALULE AFTER RESE
    __disable_interrupt();
    volatile float current_LSB;
    volatile uint16_t cal;
    volatile float rshunt;
    volatile uint16_t cal_value;
    volatile uint8_t cal_value_MSB;
    volatile uint8_t cal_value_LSB;

    current_LSB = 0.000020;
    rshunt = 0.1;                           // Resistor shunt de 100 mili Ohms, current_LSB ( Maxima corrente esperada = 0.4 A --> 0.4/2^15 = 12 uA/bit
                                            // Arredondado para o resultado de current_LSB para 50 uA/bit --> cal = 0.04096/(50x10^-6 . 0,1) = 8192
    // cal = trunc((0.04096)/(current_LSB*rshunt));
    //0.04096 is an internal fixed value used to ensure scaling is maintained properly
    cal = 8192;
    //cal_value = cal<<1;                    // Deslocar bits para a esquerda pois o FS0 eh um bit vazio, sempre permanece em 0
    cal_value_MSB = (cal >> 8) & 0xFF;// Separa o byte mais significativo de cal_value
    cal_value_LSB = cal & 0xFF;       // Separa o byte menos significativo de cal_value. ( O envio pelo i2c é em pacotes de 8 bits)


    UCB1IFG = 0;                            // Boa pratica -  Zerar o registro de flags antes de comeCar
    UCB1I2CSA = addr;                       // Confingura o endereco do escravo
    UCB1CTL1 |= UCTR;                       // Modo transmissor
    if(UCB1STAT & UCBBUSY);                 // Espera a linha estar desocupada ( Aprimorar esse codigo para que o programa n fique parado esperando)
    {
        int i = 1000;
        while(i)
        {
            i--;
            if(!(UCB1STAT & UCBBUSY)){
                i = 0;
            }
        }
    }
    UCB1CTL1 |= UCTXSTT;                    // Requisita o inicio da comunicacao com TX. UCTXSTT gera o START
    while(!(UCB1IFG & UCTXIFG));            // UCTXIFG eh setado apos o START acontecer e o byte de dados puder ser escrito na UCB1TXBUF
    UCB1TXBUF = INA219_REG_CALIBRATION;     // The value for the register pointer as shown in Figure 18 is the first byte transferred
    while( UCB1CTL1 & UCTXSTT );            // Espera o ciclo de ACK/NACK acontecer
    if(UCB1IFG & UCNACKIFG)                 // Se o escravo nao estiver presente
    {                                       // vou receber um NACK
        UCB1CTL1 |= UCTXSTP;                // Sou obrigado entao a transmitir um STOP
        while (UCB1CTL1 & UCTXSTP);         // Espero o stop ser de fato transmitido
        return 1;                           // e retorno um codigo de erro
        // COLOCAR aqui um LED de alerta para o erro
    }
    else if( UCB1IFG & UCTXIFG)             // Teve acknowledge
    {
//        wait(5, us);                        // When SCL using SCL freq. in excess of 1 MHz, register contents are updated 4 us after completion of the write command. Therefore,
//        UCB1IFG = 0;                        // a 4-us delay is required between completion of a write to a given register and a subsequent read
//        UCB1I2CSA = addr;                   // Confingura o endereco do escravo// Boa pratica -  Zerar o registro de flags antes de comeCar
//        UCB1CTL1 |= UCTR;                   // Modo transmissor
//        UCB1CTL1 |= UCTXSTT;                // Requisita o inicio da comunicacao com TX. UCTXSTT gera o START
        while(!(UCB1IFG & UCTXIFG));        // UCTXIFG eh setado apos o START acontecer e o byte de dados puder ser escrito na UCB1TXBUF
        UCB1TXBUF = cal_value_MSB;          // Manda o MSB para o Calibration Register
        while(!(UCB1IFG & UCTXIFG));        // Espera o acknowledge do dispositivo
        UCB1TXBUF = cal_value_LSB;          // Manda o LSB para o Calibration Register
        while(!(UCB1IFG & UCTXIFG));        // Espera o acknowledge do ultimo byte ser carregado no dispositivo
        UCB1CTL1 |= UCTXSTP;                //
        while (UCB1CTL1 & UCTXSTP);         // Espero o stop ser de fato transmitido
    }
    /* 0.04096 is an internal fixed value used to ensure scaling is maintained properly
     * current_LSB = Maximum Expected Current/2^15 = 1 A / 2^15 = 30.51757 uA/bit -> rounded up to 0.1 mA/bit
     * Setting the current LSB to this value allows for sufficient precission while serving to simplify the math as well
     * Power_LSB = 20 * current_LSB
     * */
    __enable_interrupt();
    return cal_value;
}
// ***********************************************************************************************************************

uint16_t ina219config( uint8_t addr, uint16_t configRegister  )
{
    // PARECE QUE A CONFIGURACAO DEFAULT COM DATA = 111 EH A MAIS UTILIZADA MESMO, POIS EH A QUE REALIZA CONTINUAMENTE MEDICOES DE SHUNT VOLT. E BUS VOLT.
    // ina219config:  All-register reset, settings for bus voltage range, PFA Gain, ADC resolution/averaging
    // 15     14      13      12     11    10    9      8     7      6     5      4      3      2      1    0
    // RST    -      BRNG    PG1    PG0  BADC4 BADC3  BADC2  BADC1  SAD4  SADC3  SADC2 SADC1  MODE   MODE   MODE
    // RW-0  RW-0    RW-1   RW-1    RW-1  RW-0  RW-0  RW-1   RW-1   RW-0  RW-1   RW-1   RW-1  RW-1   RW-1   RW-1 ==399F: VALULE AFTER RESE
   // volatile static uint16_t configRegister = 0x399F;      // Default
    // No momento esta recebendo a configuracao = 0x199F
    volatile static uint8_t config_reg_MSB;
    volatile static uint8_t config_reg_LSB;
 //   configRegister = (bit15RST<<15 | bit13BRNG<<13 | bit12e11PG <<11 | bit10e9e8e7BADC <<7 | bit6e5e4e3SADC <<3 | bit2e1e0MODE ); // Ex: Binary:0010111001101111b
    config_reg_MSB = configRegister>>8 & 0xFF;
    config_reg_LSB = configRegister & 0xFF;
    // Colocar aqui a funcao para enviar por i2c a configuracao. Essa funcao ira enviar o ponteiro de endereco 0x00 (configuration)
    // no modo de escrita
    __disable_interrupt();
    UCB1IFG = 0;                            // Boa pratica -  Zerar o registro de flags antes de comeCar
    UCB1I2CSA = addr;                       // Confingura o endereco do escravo
    UCB1CTL1 |= UCTR;                       // Modo transmissor

    UCB1CTL1 |= UCTXSTT;                    // Requisita o inicio da comunicacao com TX. UCTXSTT gera o START

    while(!(UCB1IFG & UCTXIFG));            // UCTXIFG eh setado apos o START acontecer e o byte de dados puder ser escrito na UCB1TXBUF
    UCB1TXBUF = 0X00;                       // = INA219_REG_CONFIG; Reset, settings for bus volt. range, PGA Gain, ADC resolutopm./averaging    while( UCB1CTL1 & UCTXSTT );            // Espera o ciclo de ACK/NACK acontecer
    UCB1TXBUF = config_reg_MSB;
    while( UCB1CTL1 & UCTXSTT );            // Espera o ciclo de ACK/NACK acontecer
    UCB1TXBUF = config_reg_LSB;
    UCB1CTL1 |= UCTXSTP;                //
    while (UCB1CTL1 & UCTXSTP);         // Espero o stop ser de fato transmitido
    __enable_interrupt();
    return 0;
}
void ina219mode(uint8_t addr, comando mode )
{
    // PARECE QUE A CONFIGURACAO DEFAULT COM DATA = 111 EH A MAIS UTILIZADA, POIS EH A QUE REALIZA CONTINUAMENTE MEDICOES DE SHUNT VOLT. E BUS VOLT.
    // ina219config:  All-register reset, settings for bus voltage range, PFA Gain, ADC resolution/averaging
    // 15     14      13      12     11    10    9      8     7      6     5      4      3      2      1    0
    // RST    -      BRNG    PG1    PG0  BADC4 BADC3  BADC2  BADC1  SAD4  SADC3  SADC2 SADC1  MODE   MODE   MODE
    // RW-0  RW-0    RW-1   RW-1    RW-1  RW-0  RW-0  RW-1   RW-1   RW-0  RW-1   RW-1   RW-1  RW-1   RW-1   RW-1 ==399F: VALULE AFTER RESE
    // ina219config( 0x40, 0b0, 0b0, 0b0, 0b11, 0b0011, 0b0011, 0b111);           // Endereco do dispositivo INA219 + 16 bits de config.
    // The Conditions for the example circuit is: Maximum expected load current = 0.5 A, Nominal load current = 0.1 A,
    // VCM = 5 V, RSHUNT = 0.1 Ω, VSHUNT FSR = 320 mV (PGA = /8), and BRNG = 0 (VBUS range = 16 V).
    // - RESET, BIT 15.
    // - Don't care, BIT 14.
    // - BRNG, bit 13. 0 = VBUS range = 16. 1 = VBUS range = 32
    // - PROGRAMMABLE GAIN AMPLIFIER (PGA), bits 12 e 11. The PGA function in the INA219 allows you to adjust
    // the gain of the amplifier to match the voltage range of the shunt resistor,  which is important for accurate current measurement
    // - BUS ADC (BACD), bits 10,9,8 e 7. The BADC (Bus Voltage ADC) bit in the INA219 configuration register is
    // a setting that controls the number of samples taken by the ADC (Analog-to-Digital Converter) for bus voltage measurement.
    // - SHUNT ADC (SADC), bits 6,5,4 e 3. The SADC (Shunt ADC) bit in the INA219 configuration register is
    // a setting that controls the number of samples taken by the ADC (Analog-to-Digital Converter) for shunt voltage measurement.
    // - MODE. Power-down
    // 000   Shunt voltage, triggered
    // 001   Bus voltage, triggered
    // 010   Shunt and bus, triggered
    // 011   ADC off (disabled)
    // 100   Shunt voltage, continuous
    // 101   Bus voltage, continuous
    // 111   Shunt and bus. continuous
    // volatile static uint16_t configRegister = 0x399F;      // Default
    // No modo de operacao 111 "ina219config" esta recebendo a configuracao = 0x199F
    // No modo Power-Down

    if(mode == powerdown){                  // 000: Power-Down mode reduces the quiescent current and turns off
        ina219config( addr, 0X1998  );      // current into INA219 inputs, avoiding any supply drain.
    }
    if(mode == shunttrig){                  // 001: Shunt voltage, triggers a single-shot conversion  (even if already programmed)
        ina219config( addr, 0X1999  );
    }
    if(mode == bustrig){                    // 010: Bus voltage, triggers a single-shot conversion  (even if already programmed)
        ina219config( addr, 0X199A  );
    }
    if(mode == shuntandbustrig){            // 011: Shunt and bus, triggered
        ina219config( addr, 0X199B  );
    }
    if(mode == adcoff){                     // 100: ADC off. Stops all conversions.
        ina219config( addr, 0X199C  );
    }
    if(mode == shuntvolt){                  // 101: Convert current continuously
        ina219config( addr, 0X199D  );
    }
    if(mode == busvolt){                    // 110: Convert voltage continuously
        ina219config( addr, 0X199E  );
    }
    if(mode == shuntandbusvolt){            // 111: Normal operatin mode (MODE bits of the Conf.reg. = 111).
                                            // It continuously convertes the shunt voltage up to the number set in the
                                            // shunt voltage averaging function( Conf.reg., SADC bits).
                                            // The device then converts the bus voltage up to the number set in the bus
        ina219config( addr, 0X199F  );
    }
}
