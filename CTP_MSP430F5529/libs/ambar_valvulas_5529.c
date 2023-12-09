/*
 * timer.c
 *
 *  Created on: 5 de jul de 2021
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

//                              *** ENDERECOS I2C ***
// PCF8574_valves - CONTROLE DAS VALVULAS  - Endereco: 0x20
// PCF8574_sNivel - SENSORES DE NIVEL      - Endereco: 0x21
// PCF8574_LCD - LCD                    - Endereco: 0x27
// INA219  - Leitor Tensao/Corrente - Endereco: 0x40
// ESP32  - cliente MQTT - Endereco: 0x42

#define      PCF8574_valves  0x20
#define      PCF8574_sNivel  0x21
#define      ESP32           0x42
#define      INA219          0x40
#define      PCF8574_LCD1    0x27
#define      PCF8574_LCD2    0x26

#define      CLOSE_T1_In     0b11111110      //  QN9 (TIP127)
#define      OPEN_T1_In      0b11111101     //  QN10 (TIP127)
#define      CLOSE_T1_Out    0b11111011     //  QN13 (TIP127)
#define      OPEN_T1_Out     0b11110111     //  QN14 (TIP127)
#define      CLOSE_T2_In     0b11101111     //  QN11 (TIP127)
#define      OPEN_T2_In      0b11011111     //  QN12 (TIP127)
#define      CLOSE_T2_Out    0b10111111     //  QN15 (TIP127)
#define      OPEN_T2_Out     0b01111111     //  QN16 (TIP127)

volatile uint8_t inputs74HC139;
volatile uint8_t PCF8574_TIP127;
volatile unsigned int flag_Operador_T1_T2 = 0;                // Flag para o operador dos tanques
volatile unsigned int evap_T1 = 0;              //Vari�vel "evap_T1" e "evap_T2", permite a entrada na fun��o timerevap_T1(2)
volatile unsigned int evap_T2 = 0;
volatile unsigned int regT1ok, regT2ok;              // Indicacoes para tanques prontos para regar
volatile unsigned int flag_regT1ok, flag_regT2ok;    // Indicacoes para tanques prontos para regar
extern int rotina_Inicial;
extern int flagNivelA_1, flagNivelB_1, flagNivelA_2, flagNivelB_2;
extern volatile unsigned int lcd_T1Tool;
volatile unsigned int t1_closeRegIN, t2_closeRegIN, t1_closeRegOUT, t2_closeRegOUT;         // Solicitacao de fechamento e abertura de v�lvulas
volatile unsigned int t1_openRegIN, t2_openRegIN, t1_openRegOUT, t2_openRegOUT;             // Solicitacao de fechamento e abertura de v�lvulas
volatile unsigned int t1_regar, t2_regar;            // Solicita��o de rega
volatile unsigned int t1_encher, t2_encher;          // Solicita��o para encher os tanques
extern int atualizarUart;
extern volatile char ina219 [5];
// Variaveis vindas do config_timers_5529
extern volatile uint16_t count;
extern volatile uint8_t flagTvalve;
// Variaveis vindas do Ambar1.c
extern volatile uint8_t regT1_OkfromNut;
extern volatile uint8_t regT2_OkfromNut;

volatile uint8_t mem_byte_interrupt = 0;
volatile uint8_t t1Past;
volatile uint8_t t1Now;
volatile uint8_t t2Past;
volatile uint8_t t2Now;
//volatile unsigned int tseg_valve;
//void i2cReleControl( componente rele, tarefa ON_OFF )
//{
//     CBA = P.76543210  da PCF8574
//     000 = 0bRRXX1XX1
//     001 = 0b00X11X00  CLOSE T1 In    QN9 (TIP127)
//     000 = 0b00X11X10   OPEN T1 In    QN10 (TIP127)
//     101 = 0b00X01X10  CLOSE T1 Out   QN11 (TIP127)
//     100 = 0b00X01X00   OPEN T1 Out   QN12 (TIP127)
//     010 = 0b001X01X1  CLOSE T2 In    QN13 (TIP127)
//     011 = 0b001X00X1   OPEN T2 In    QN14 (TIP127)
//     111 = 0b000X01X1  CLOSE T2 Out   QN15 (TIP127)
//     110 = 0b000X00X1   OPEN T2 Out   QN16 (TIP127)
//    uint8_t addr = PCF8574_LCD2;
//
//    if ( rele == RELE1)                  // Registro de entrada T1
//    {
//        if ( acao == ON)                 // QN10 (TIP127)
//        {//                 0b00X11X10
//         //   inputs74HC139 = 0b00011010;     // OPEN RegIn T1. Abrir registro de entrada do tanque 1
//            lcdLayout(PCF8574_LCD2,initPrep);         // Volta para o layout inicial do LCD
//        }
//        if ( acao == OFF)                // QN9 (TIP127)
//        {//                 0b00X11X00
//         //   inputs74HC139 = 0b00011000;     // CLOSE RegIn T1. Fechar registro de entrada do tanque 1
//            lcdLayout(PCF8574_LCD2,prepOk);        // Volta para o layout inicial do LCD
//        }
//    }
//    if ( rele == RELE2)                  // Registro de entrada T1
//    {
//        if ( acao == ON)                 // QN10 (TIP127)
//        {//                 0b00X11X10
//         //   inputs74HC139 = 0b00011010;     // OPEN RegIn T1. Abrir registro de entrada do tanque 1
//            lcdLayout(PCF8574_LCD2, sendPrep);         // Volta para o layout inicial do LCD
//        }
//        if ( acao == OFF)                // QN9 (TIP127)
//        {//                 0b00X11X00
//         //   inputs74HC139 = 0b00011000;     // CLOSE RegIn T1. Fechar registro de entrada do tanque 1
//            lcdLayout(PCF8574_LCD2, prepSend);        // Volta para o layout inicial do LCD
//        }
//    }

void i2cValveControl( componente valvula, tarefa acao )
{
    //       P.76543210  da PCF8574
    // 001 = 0b11111110  CLOSE T1 In    QN9 (TIP127)
    // 000 = 0b11111101   OPEN T1 In    QN10 (TIP127)
    // 101 = 0b11111011  CLOSE T1 Out   QN13 (TIP127)
    // 100 = 0b11110111   OPEN T1 Out   QN14 (TIP127)
    // 010 = 0b11101111  CLOSE T2 In    QN11 (TIP127)
    // 011 = 0b11011111   OPEN T2 In    QN12 (TIP127)
    // 111 = 0b10111111  CLOSE T2 Out   QN15 (TIP127)
    // 110 = 0b01111111   OPEN T2 Out   QN16 (TIP127)
    if ( valvula == T1INV)                  // Registro de entrada T1
    {
        if ( acao == fechar)                // QN9 (TIP127)
        {
            // CLOSE RegIn T1. Fechar registro de entrada do tanque 1
            i2cWriteByte_UCB1( PCF8574_valves , CLOSE_T1_In);
            lcdLayout(PCF8574_LCD1,v1infechando);        // Volta para o layout inicial do LCD
        }
        if ( acao == abrir)                 // QN10 (TIP127)
        {
            // OPEN RegIn T1. Abrir registro de entrada do tanque 1
            i2cWriteByte_UCB1( PCF8574_valves , OPEN_T1_In);
            lcdLayout(PCF8574_LCD1,v1inabrindo);         // Volta para o layout inicial do LCD
            lcd_T1Tool = 1;                 // Quando T1 esta enchendo, serve para periodicamente informar o nivel no visor LCD
        }
    }
    else if ( valvula == T1OUTV)            // Registro de saida do T1
    {
        if ( acao == fechar)                // QN13 (TIP127)
        {//                 0b00X01X10
            // Fechar registro de saida do tanque 1
            i2cWriteByte_UCB1( PCF8574_valves , CLOSE_T1_Out);
            lcdLayout(PCF8574_LCD1,v1outfechando);       // Volta para o layout inicial do LCD
        }
        if ( acao == abrir)                 // QN14 (TIP127)
        {//                 0b00X01X00
            // OPEN RegOutT1 . Abrir registro de saida do tanque 1
            i2cWriteByte_UCB1( PCF8574_valves , OPEN_T1_Out);
            lcdLayout(PCF8574_LCD1,v1outabrindo);        // Volta para o layout inicial do LCD
            lcd_T1Tool = 4;                 // Para escrever no LCD que esta regando pelo tanque 1
        }
    }
    else if ( valvula == T2INV)             //RegIn T2
    {
        if ( acao == fechar)                // QN11 (TIP127). RegIn T2 CLOSE.
        {
            // Fechar registro de entrada do tanque 2
            i2cWriteByte_UCB1( PCF8574_valves , CLOSE_T2_In);
            lcdLayout(PCF8574_LCD1,v2infechando);        // Volta para o layout inicial do LCD
        }
        if ( acao == abrir)                 // QN12 (TIP127). RegIn T2 OPEN
        {
            // Abrir registro de entrada do tanque 2
            i2cWriteByte_UCB1( PCF8574_valves , OPEN_T2_In);
            lcdLayout(PCF8574_LCD1,v2inabrindo);         // Volta para o layout inicial do LCD
            lcd_T2Tool = 1;                 // Quando T2 esta enchendo, serve para periodicamente informar o nivel no visor LCD
        }
    }
    else if ( valvula == T2OUTV)
    {
        if ( acao == fechar)                // QN15 (TIP127). RegOutT2 CLOSE.
        {
            // Fechar registro de saida do tanque 2
            i2cWriteByte_UCB1( PCF8574_valves , CLOSE_T2_Out);
            lcdLayout(PCF8574_LCD1,v2outfechando);       // Volta para o layout inicial do LCD
        }
        if ( acao == abrir)                 // QN16 (TIP127). RegOutT2.
        {
            // Abrir registro de saida do tanque 2
            i2cWriteByte_UCB1( PCF8574_valves , OPEN_T2_Out);
            lcdLayout(PCF8574_LCD1,v2outabrindo);        // Volta para o layout inicial do LCD
            lcd_T2Tool = 4;                 // Para escrever no LCD que esta regando pelo tanque 1
        }
    }
    flagTvalve = 1;

    // Ligando as conversoes continuas no ina219 ( para medir corrente e tensao quando as valvulas forem acionadas)
   // ina219mode( 0x40, shuntandbusvolt);// 111: Normal operatin mode (MODE bits of the Conf.reg. = 111).
    // It continuously convertes the shunt voltage up to the number set in the
    // shunt voltage averaging function( Conf.reg., SADC bits).
    // The device then converts the bus voltage up to the number set in the bus
    // Apresentar medicoes de corrente e potencia
    lcdCursor(PCF8574_LCD1,0x14);
    lcdPrint(PCF8574_LCD1," Corrente:          ");   // Funcao para escrever caracteres no LCD
    lcdCursor(PCF8574_LCD1,0x54);
    lcdPrint(PCF8574_LCD1," Consumo:           ");   // Funcao para escrever caracteres no LCD

    // ------------------------------------------------------------------
    // "i2cwriteByte (uint8_t addr, uint8_t byte)" eh o "portao de embarque" para os bytes de comunicacao
    // com o decoder, por meio do protocolo i2c. Tres bytes sao enviados, com os tres bits "inputs74HC139" sendo enviados nos tres
    // bytes, mas somente no segundo byte eh ativado o enable, dessa forma os bits de "inputs74HC139" desejados ja estao carregados, evitando assim
    // acionamentos momentaneos das outras valvulas.
    // Temos como caracteristica o envio de dois "pacotes", o primeiro com EN=0, o segundo EN=1
    // e o ultimo com EN=0, 7 segundos depois, depois que a valvula abriu/fechou completamente.
    //                                       __
    //                             EN --> __|  |__
    // POR QUE eh assim? ï¿½ dessa forma porque a leitura dos dados acontece no flanco de subida do enable,
    // entao primeiro enviamos os bits de "inputs74HC139" do decoder, depois fazemos um novo envio, agora ativando o enable
    // e por fim, enviamos o mesmo byte so que com o enable em LOW, para desativar o decoder.

}
void testesAmbar5529()
{
    // Tanque 1
    if(t1_openRegIN)
    {
        uartWrite("Abrindo registro de entrada do tanque 1...\n\r");
        i2cValveControl(T1INV, abrir);
        t1_openRegIN = 0;
    }
    if(t1_closeRegIN)
    {
        uartWrite("Fechando registro de entrada do tanque 1...\n\r");
        i2cValveControl(T1INV, fechar);
        t1_closeRegIN = 0;
    }
    if(t1_openRegOUT)
    {
        uartWrite("Abrindo registro de saida do tanque 1...\n\r");
        i2cValveControl(T1OUTV, abrir);
        t1_openRegOUT = 0;
    }
    if(t1_closeRegOUT)
    {
        uartWrite("Fechando registro de saida do tanque 1...\n\r");
        i2cValveControl(T1OUTV, fechar);
        t1_closeRegOUT = 0;
    }
    // Tanque 2
    if(t2_openRegIN)
    {
        uartWrite("Abrindo registro de entrada do tanque 2...\n\r");
        i2cValveControl(T2INV, abrir);
        t2_openRegIN = 0;
    }
    if(t2_closeRegIN)
    {
        uartWrite("Fechando registro de entrada do tanque 2...\n\r");
        i2cValveControl(T2INV, fechar);
        t2_closeRegIN = 0;
    }
    if(t2_openRegOUT)
    {
        uartWrite("Abrindo registro de saida do tanque 2...\n\r");
        i2cValveControl(T2OUTV, abrir);
        t2_openRegOUT = 0;
    }
    if(t2_closeRegOUT)
    {
        uartWrite("Fechando registro de saida do tanque 2...\n\r");
        i2cValveControl(T2OUTV, fechar);
        t2_closeRegOUT = 0;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++
//ESTADO DE INICIALIZACAO - Monitorar niveis A e B dos tanques 1 e 2
int monNivelAB_1_2(void)
{
    // Inicializando
    uartWrite("\n");                        // Pular para linha de baixo no terminal
    uartWrite("\r");                        // Retorno do carro no terminal
    uartWrite("\n");                        // Pular para linha de baixo no terminal
    uartWrite("\r");                        // Retorno do carro no terminal
    uartWrite("Inicializando central...\n\r");
    // Fechar todas as valvulas apos um reset
    t1_closeRegIN   = 1;
    t1_closeRegOUT  = 1;
    t2_closeRegIN   = 1;
    t2_closeRegOUT  = 1;
    //    lcdCursor(addr,0x00);                     // Pular para linha de baixo
    //    lcdPrint(addr,"   Mesmo que tudo   ");
    //    lcdCursor(addr,0x40);                     // Pular para linha de baixo
    //    lcdPrint(addr,"   pareca dificil   ");
    //    wait(1, sec);
    //    lcdCursor(addr,0x14);                     // Pular para linha de cima
    //    lcdPrint(addr," Deus esta com voce ");
    //    wait(2, sec);
    //    lcdCursor(addr,0x00);
    //    lcdPrint(addr,"      Sistema       ");
    //    lcdCursor(addr,0x40);                     // Pular para linha de baixo
    //    lcdPrint(addr,"         de         ");
    //    lcdCursor(addr,0x14);                     // Pular para linha de baixo
    //    lcdPrint(addr,"   Fertirrigacao    ");
    //    lcdCursor(addr,0x54);                     // Pular para linha de baixo
    //    lcdPrint(addr,"        Ambar        ");
    //    wait(2, sec);
    // ************************************************************************************
    // Rotina de inicializacao apos um reset
    // NivelB2|NivelA2|NivelB1|NivelA1|| readPCF8574  ||       T1       |      T2
    //    0       0       0       0   || 0bXXXX0000   ||  T2 VAZIO      |   T1 VAZIO       x    |   x    |
    //    0       0       1       0   || 0bXXXX0010   ||  T2 VAZIO      |   T1 1/2 CHEIO
    //    0       0       1       1   || 0bXXXX0011   ||  T2 VAZIO      |   T1 CHEIO
    //    0       1       0       0   || 0bXXXX0100   ||  ERRO T2       |   T1 VAZIO
    //    0       1       0       1   || 0bXXXX0101   ||  ERRO T2       |   ERRO T1
    //    0       1       1       0   || 0bXXXX0110   ||  ERRO T2       |   T1 1/2 CHEIO
    //    0       1       1       1   || 0bXXXX0111   ||  ERRO T2       |   T1 CHEIO
    //    1       0       0       0   || 0bXXXX1000   ||  T2 1/2 CHEIO  |   T1 VAZIO
    //    1       0       0       1   || 0bXXXX1001   ||  T2 1/2 CHEIO  |   ERRO T1
    //    1       0       1       0   || 0bXXXX1010   ||  T2 1/2 CHEIO  |   T1 1/2 CHEIO
    //    1       0       1       1   || 0bXXXX1011   ||  T2 1/2 CHEIO  |   T1 CHEIO
    //    1       1       0       0   || 0bXXXX1100   ||  T2 CHEIO      |   T1 VAZIO
    //    1       1       0       1   || 0bXXXX1101   ||  T2 CHEIO      |   ERRO T1
    //    1       1       1       0   || 0bXXXX1110   ||  T2 CHEIO      |   T1 1/2 CHEIO
    //    1       1       1       1   || 0bXXXX1111   ||  T2 CHEIO      |   T1 CHEIO
    //               - Fecha todas as valvulas ao ocorrer um reset -

    mem_byte_interrupt = readPCF8574(ADDR_SNIVEL);      // Leitura dos estados dos sensores de nivel
    mem_byte_interrupt = mem_byte_interrupt & 0x0F;
    if((mem_byte_interrupt) == 0b0000) //  1010  T2 Vazio e T1 Vazio
    {
        t1_openRegIN = 1;
        regT1ok  = 0;
        regT2ok  = 0;
        t1_regar = 0;
        t2_regar = 0;
        lcd_T1Tool = 1;                     // Status enchendo tanque 1
        lcd_T2Tool = 5;                     // T2 vazio
        rotina_Inicial = 1;
    }
    else if((mem_byte_interrupt) == 0b0001)  // 0001  T2 Vazio e ERRO T1
    {
        t2_openRegIN = 1;
        lcdLayout(PCF8574_LCD1,erroT1);
        lcd_T1Tool = 6;             // Erro T1
        regT2ok  = 0;
        t2_regar = 0;
        lcd_T2Tool = 1;             // T2 enchendo
    }
    else if((mem_byte_interrupt) == 0b0010)  // 0010  T2 Vazio  e T1 1/2 Cheio
    {
    t1_openRegOUT = 1;
    t2_openRegIN = 1;
    regT1ok  = 0;
    regT2ok  = 0;
    t1_regar = 1;
    t2_regar = 0;
    lcd_T1Tool = 4;             // Regando pelo tanque 1
    lcd_T2Tool = 1;             // Tanque 2 enchendo
    }
    else if((mem_byte_interrupt) == 0b0011)  // 0011  T2 Vazio  e T1 Cheio
    {
    t1_openRegOUT = 1;
    t2_openRegIN = 1;
    regT1ok  = 0;
    regT2ok  = 0;
    t1_regar = 1;
    t2_regar = 0;
    lcd_T1Tool = 4;             // Regando pelo tanque 1
    lcd_T2Tool = 1;             // Tanque 2 enchendo
    }
    else if((mem_byte_interrupt) == 0b0100)  // 0100  Erro T2  e T1 Vazio
    {
        t1_openRegIN = 1;
        lcdLayout(PCF8574_LCD1,erroT2);
        lcd_T2Tool = 6;             // T2 com erro
        // lcdLayout(PCF8574_LCD1,v1inabrindo);
        regT1ok  = 1;
        regT2ok  = 1;
        t1_regar = 1;
        t2_regar = 0;
        lcd_T1Tool = 1;             // Enchendo tanque 1
    }
    else if((mem_byte_interrupt) == 0b0101)  // 0101  Erro T2  e Erro T1
    {
        lcdLayout(PCF8574_LCD1,erroT1T2);
        lcd_T1Tool = 6;             // Erro T1
        lcd_T2Tool = 6;             // Erro T2
        //rotina_Inicial == 1;
    }
    else if((mem_byte_interrupt) == 0b0110)  // 0110  Erro T2  e T1 1/2 Cheio
    {
        t1_openRegOUT = 1;
        lcdLayout(PCF8574_LCD1,erroT2);
        lcd_T2Tool = 6;             // Erro T1
        regT1ok  = 1;
        t1_regar = 1;
        lcd_T1Tool = 4;             // T1 regando
    }
    else if((mem_byte_interrupt) == 0b0111)  // 0111  Erro T2  e T1 Cheio
    {
        t1_openRegOUT = 1;
        lcdLayout(PCF8574_LCD1,erroT2);
        lcd_T2Tool = 6;             // Erro T1
        regT1ok  = 1;
        t1_regar = 1;
        lcd_T1Tool = 4;             // T1 regando
    }
    else if((mem_byte_interrupt) == 0b1000)  // 1000  T2 1/2 Cheio  e T1 Vazio
    {
        t1_openRegIN = 1;
        t2_openRegOUT = 1;
        regT1ok  = 0;
        t1_regar = 0;
        regT2ok  = 0;
        t2_regar = 1;
        lcd_T1Tool = 1;                     // Enchendo tanque 1
        lcd_T2Tool = 4;                     // Status regando pelo tanque 2
    }
    else if((mem_byte_interrupt) == 0b1001)  // 1001  T2 1/2 Cheio  e Erro T1
    {
        t2_openRegOUT = 1;
        lcdLayout(PCF8574_LCD1,erroT1);
        lcd_T1Tool = 6;             // Erro T1
        regT2ok  = 0;
        t2_regar = 1;
        lcd_T2Tool = 4;             // T2 REGANDO
    }
    else if((mem_byte_interrupt) == 0b1010)  // 1010  T2 1/2 Cheio  e T1 1/2 Cheio
    {
        t1_openRegOUT = 1;
        regT1ok  = 0;
        regT2ok  = 1;
        t1_regar = 1;
        t2_regar = 0;
        lcd_T1Tool = 4;             // T1 Regando
        lcd_T2Tool = 3;             // T2 ok
    }
    else if((mem_byte_interrupt) == 0b1011)  // 1011  T2 1/2 Cheio  e T1 Cheio
    {
        t1_openRegOUT = 1;
        regT1ok  = 0;
        regT2ok  = 1;
        t1_regar = 1;
        t2_regar = 0;
        lcd_T1Tool = 4;             // T1 Regando
        lcd_T2Tool = 3;             // T2 ok
    }
    else if((mem_byte_interrupt) == 0b1100)  // 1100  T2 Cheio  e T1 Vazio
    {
        t1_openRegIN = 1;
        t2_openRegOUT = 1;
        regT1ok  = 0;
        t1_regar = 0;
        regT2ok  = 0;
        t2_regar = 1;
        lcd_T1Tool = 1;                     // Enchendo tanque 1
        lcd_T2Tool = 4;                     // Status regando pelo tanque 2
    }
    else if((mem_byte_interrupt) == 0b1101)  // 1101  T2 Cheio  e Erro T1
    {
        t2_openRegOUT = 1;
        lcdLayout(PCF8574_LCD1,erroT1);
        lcd_T1Tool = 6;             // Erro T1
        regT2ok  = 0;
        t2_regar = 1;
        lcd_T2Tool = 4;             // T2 REGANDO
    }
    else if((mem_byte_interrupt) == 0b1110)  // 1110  T2 Cheio  e T1 1/2 Cheio
    {
        t1_openRegOUT = 1;
        regT1ok  = 0;
        regT2ok  = 1;
        t1_regar = 1;
        t2_regar = 0;
        lcd_T1Tool = 4;             // T1 Regando
        lcd_T2Tool = 3;             // T2 ok
    }
    else if((mem_byte_interrupt) == 0b1111)  // 1111  T2 Cheio  e T1 Cheio
    {
        t1_openRegOUT = 1;
        regT1ok  = 0;
        regT2ok  = 1;
        t1_regar = 1;
        t2_regar = 0;
        lcd_T1Tool = 4;             // T1 Regando
        lcd_T2Tool = 3;             // T2 ok
    }
    t2Past  = (mem_byte_interrupt >> 2);
    t2Past &= 0x03;         // Exemplo (0b11111011) &= 0x03 = 0b00000011 &= 0x03 = 0b00000011
    t2Now = t2Past;         // Salva em ambos o estado inicial, para fazer sentido na funcao Rebotes, que salva Now em Past
    t1Past  = mem_byte_interrupt;
    t1Past &= 0x03;         // Exemplo (0b11111011 >> 2) &= 0x03 = 0b00000010 &= 0x03 = 0b00000010
    t1Now = t1Past;         // Salva em ambos o estado inicial, para fazer sentido na funcao Rebotes, que salva Now em Past
    lcdLayout(PCF8574_LCD1,defaut);                // Volta para o layout inicial do LCD
    return 0;
}
//---------------------------------------------
//++++++++++++++++++++++++++++++++++++++++++++++
void operador_T1_T2()
{
    uint8_t i = 0;
        if( t1_closeRegIN )                      // Fechando registro de entrada do tanque 1
        {
            uartWrite("Fechando registro de entrada do tanque 1...\n\r");
            i2cValveControl(T1INV, fechar);
            i = 1;
            t1_closeRegIN = 0;
        }
        else if( t1_closeRegOUT )                    // Fechando registro de saida do tanque 1
        {
            uartWrite("Fechando registro de saida do tanque 1...\n\r");
            i2cValveControl(T1OUTV, fechar);
            i = 1;
            t1_closeRegOUT = 0;
        }
        else if( t2_closeRegIN )                    // Fechando registro de entrada do tanque 2
        {
            uartWrite("Fechando registro de entrada do tanque 2...\n\r");
            i2cValveControl(T2INV, fechar);
            i = 1;
            t2_closeRegIN = 0;
        }
        else if( t2_closeRegOUT )                    // Fechando registro de saida do tanque 2
        {
            uartWrite("Fechando registro de saida do tanque 2...\n\r");
            i2cValveControl(T2OUTV, fechar);
            i = 1;
            t2_closeRegOUT  = 0;
        }
        else if( t1_openRegIN )                      // Abrindo registro de entrada do tanque 1
        {
            uartWrite("Abrindo registro de entrada do tanque 1...\n\r");
            i2cValveControl(T1INV, abrir);
            i = 1;
            t1_openRegIN = 0;
        }
        else if( t1_openRegOUT )                     // Abrindo registro de saida do tanque 1
        {
            uartWrite("Abrindo registro de saida do tanque 1...\n\r");
            i2cValveControl(T1OUTV, abrir);
            i = 1;
            t1_openRegOUT = 0;
        }
        else if( t2_openRegIN )                      // Abrindo registro de entrada do tanque 2
        {
            uartWrite("Abrindo registro de entrada do tanque 2...\n\r");
            i2cValveControl(T2INV, abrir);
            i = 1;
            t2_openRegIN = 0;
        }
        else if( t2_openRegOUT )                     // Abrindo registro de saida do tanque 2
        {
            uartWrite("Abrindo registro de saida do tanque 2...\n\r");
            i2cValveControl(T2OUTV, abrir);
            i = 1;
            t2_openRegOUT = 0;
        }
        else if(regT1ok & t1_regar)              //  Abrindo registro de saida do tanque 1
        {                                   //  t1_regar eh uma solicitacao vinda de t2
         //   if(regT1_OkfromNut == 1)        //  Preparador de nutrientes envia o regT1_OkfromNut
          //  {
                uartWrite("Abrindo registro de saida do tanque 1...\n\r");
                i2cValveControl(T1OUTV, abrir);
                i = 1;
                regT1ok = 0;
         //   }
        }
        else if(regT2ok & t2_regar)             // Abrindo registro de saida do tanque 2
        {                                  // t2_regar eh uma solicitacao vinda de t1
         //   if(regT2_OkfromNut == 1)       // Preparador de nutrientes envia o regT1_OkfromNut
         //   {
                uartWrite("Abrindo registro de saida do tanque 2...\n\r");
                i2cValveControl(T2OUTV, abrir);
                i = 1;
                regT2ok = 0;
         //   }
        }
        else if( i == 1)
        {
            //ina219Print(ina219, 4);
            i = 0;
        }
//        if( rele1_ON1_OFF0)
//        {
//            i2cReleControl(RELE1, on);
//        }
//        else
//        {
//            if( rele1_ON1_OFF0)
//            {
//                i2cReleControl(RELE2, off);
//            }
//        }
}

