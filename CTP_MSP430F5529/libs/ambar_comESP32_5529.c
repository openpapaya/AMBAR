/*
 * ambar_ComEsp32.c
 *
 *  Created on: Jul 7, 2023
 *      Author: guiwz
 */

#include <msp430.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "libs/config_gpio_5529.h"
#include "libs/config_lcd_5529.h"
#include "libs/config_timers_5529.h"
#include "libs/ambar_uart_5529.h"
#include "libs/ambar_lcd_5529.h"
#include "libs/ambar_valvulas_5529.h"
#include "libs/config_i2c_5529.h"
#include "libs/funcoes_ina219_5529.h"
#include "libs/funcoes_PCF8574_5529.h"

extern volatile unsigned int  s1_Ultra;
extern volatile unsigned int  s2_Ultra;
extern volatile char stringToEsp32[10];
extern volatile float      tempCelsius;

serial_esp32()
{

    volatile unsigned int x = 0;
    volatile char y = 0;
    volatile unsigned int i = 0;
    volatile float p = 0;
    volatile float v;
    //uint8_t tempT1int = 0;
   // tempT1int = (int)tempCelsius;
    inteiroParaString(s1_Ultra, tanque1L);
    uartWrite_UCA0(stringT1L);
    inteiroParaString(s2_Ultra, tanque2L);
    uartWrite_UCA0(stringT2L);
   // inteiroParaString(tempT1int, tempint);
    //uartWrite_UCA0(stringT1temp);



    // TEMPERATURA
    x = 0;
    i = 0;
    v = tempCelsius;
    // Separando dezena
    p = v/10;                        // f = 9.5731
    y = (int)p;                      // separa o inteiro da parte fracionária
    stringT1temp [0] = 0x30 + y;      // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
    // Separando unidade
    v = v - 10*y;                    // v = 95,731 - 10*9 = 5,731
    y = (int)v;
    stringT1temp [1] = 0x30 + y;       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
    // Separando primeira casa decimal
    v = v - y;                       // v = 5,731 - 5 = 0,731
    v = 10*v;                        // v = 7,31
    y = (int)v;
    stringT1temp[2] = 0x30 + y;       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
    stringT1temp[3] = 'T';
    uartWrite_UCA0(stringT1temp);

    if(lcd_T1Tool == 1)                 // lcd_T1Tool =  flag para o tanque 1 enchendo
                {
                    stringToEsp32[0] = '1';
                    stringToEsp32[1] = '1';
                    stringToEsp32[2] = ';';
                    //        i2cWriteByte_UCB0(0x42, 3);
                    //        i2cWriteByte_UCB0(0x42, 1);
                    uartWrite_UCA0(stringToEsp32);
                }
                //-------------------------------------------------
                //Informar da evaporacao do tanque 1
                //-------------------------------------------------
                else if(lcd_T1Tool == 2 )                        // Se a flag "evap_T1" esta levantada, entra na contagem para atualizaï¿½ï¿½o do estado
                {
                    stringToEsp32[0] = '1';
                    stringToEsp32[1] = '2';
                    stringToEsp32[2] = ';';
                    //           i2cWriteByte_UCB0(0x42, 3);
                    //          i2cWriteByte_UCB0(0x42, 2);
                    uartWrite_UCA0(stringToEsp32);
                }
                //--------------------------------------------------------------------------------------------------
                // Contador ativado para indicar no LCD que evap 1 ok ( lcd_T1Tool = 3;)
                //--------------------------------------------------------------------------------------------------
                else if(lcd_T1Tool == 3)
                {
                    stringToEsp32[0] = '1';
                    stringToEsp32[1] = '3';
                    stringToEsp32[2] = ';';
                    //     i2cWriteByte_UCB0(0x42, 3);
                    //         i2cWriteByte_UCB0(0x42, 3);
                    uartWrite_UCA0(stringToEsp32);                       // Comunicacao com o ESP32
                }
                //-------------------------------------------------
                //Contador da rega do tanque 1
                //-------------------------------------------------
                else if(lcd_T1Tool == 4)
                {
                    stringToEsp32[0] = '1';
                    stringToEsp32[1] = '4';
                    stringToEsp32[2] = ';';
                    //      i2cWriteByte_UCB0(0x42, 3);
                    //      i2cWriteByte_UCB0(0x42, 4);
                    uartWrite_UCA0(stringToEsp32);                       // Comunicacao com o ESP32
                }
                else if(lcd_T1Tool == 5)
                {
                    stringToEsp32[0] = '1';
                    stringToEsp32[1] = '5';
                    stringToEsp32[2] = ';';
                    //     i2cWriteByte_UCB0(0x42, 3);
                    ///     i2cWriteByte_UCB0(0x42, 5);
                    uartWrite_UCA0(stringToEsp32);                       // Comunicacao com o ESP32
                }
                else if(lcd_T1Tool == 6)
                {
                    stringToEsp32[0] = '1';
                    stringToEsp32[1] = '6';
                    stringToEsp32[2] = ';';
                    //     i2cWriteByte_UCB0(0x42, 3);
                    //     i2cWriteByte_UCB0(0x42, 6);
                    uartWrite_UCA0(stringToEsp32);                       // Comunicacao com o ESP32
                }
                //  uartWrite_UCA0(stringToEsp32);
                //--------------------------------------------------------------------------------------------------
                // Contador ativado quando T2 esta enchendo, serve para periodicamente informar o nivel no visor LCD
                //--------------------------------------------------------------------------------------------------
                if(lcd_T2Tool == 1)
                {
                    stringToEsp32[0] = '2';
                    stringToEsp32[1] = '1';
                    stringToEsp32[2] = ';';
                    //     i2cWriteByte_UCB0(0x42, 4);
                    //     i2cWriteByte_UCB0(0x42, 1);
                    uartWrite_UCA0(stringToEsp32);                       // Comunicacao com o ESP32
                }

                //-------------------------------------------------
                //Contador da evaporacao do tanque 2
                //-------------------------------------------------
                else if(lcd_T2Tool == 2)
                {
                    stringToEsp32[0] = '2';
                    stringToEsp32[1] = '2';
                    stringToEsp32[2] = ';';
                    //     i2cWriteByte_UCB0(0x42, 4);
                    //     i2cWriteByte_UCB0(0x42, 2);
                    uartWrite_UCA0(stringToEsp32);                       // Comunicacao com o ESP32
                }
                //--------------------------------------------------------------------------------------------------
                // Contador ativado para indicar no LCD que evap 2 ok ( lcd_T2Tool = 3;)
                //--------------------------------------------------------------------------------------------------
                else if(lcd_T2Tool == 3)
                {
                    stringToEsp32[0] = '2';
                    stringToEsp32[1] = '3';
                    stringToEsp32[2] = ';';
                    //      i2cWriteByte_UCB0(0x42, 4);
                    //        i2cWriteByte_UCB0(0x42, 3);

                    uartWrite_UCA0(stringToEsp32);                       // Comunicacao com o ESP32
                }
                //-------------------------------------------------
                //Contador da rega do tanque 2
                //-------------------------------------------------
                else if(lcd_T2Tool == 4)                     //
                {
                    stringToEsp32[0] = '2';
                    stringToEsp32[1] = '4';
                    stringToEsp32[2] = ';';
                    //       i2cWriteByte_UCB0(0x42, 4);
                    //       i2cWriteByte_UCB0(0x42, 4);
                    uartWrite_UCA0(stringToEsp32);                       // Comunicacao com o ESP32
                }
                else if(lcd_T2Tool == 5)                     //
                {
                    stringToEsp32[0] = '2';
                    stringToEsp32[1] = '5';
                    stringToEsp32[2] = ';';
                    //       i2cWriteByte_UCB0(0x42, 4);
                    //       i2cWriteByte_UCB0(0x42, 5);
                    uartWrite_UCA0(stringToEsp32);                       // Comunicacao com o ESP32
                }
                else if(lcd_T2Tool == 6)                     //
                {
                    stringToEsp32[0] = '2';
                    stringToEsp32[1] = '6';
                    stringToEsp32[2] = ';';
                    //        i2cWriteByte_UCB0(0x42, 4);
                    //        i2cWriteByte_UCB0(0x42, 6);
                    uartWrite_UCA0(stringToEsp32);                       // Comunicacao com o ESP32
                }
}
