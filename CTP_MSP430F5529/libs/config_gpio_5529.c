/*
 * gpio.c
 *
 *  Created on: 30 de jun de 2021
 *      Author: guiwz
 */
//Configurar GPIO
//P1.0 - LED vermelho da placa
//P1.0 -
/*
 * gpio.c
 *
 *  Created on: 30 de jun de 2021
 *      Author: guiwz
 */
//Configurar GPIO

//*************************************************************
//      - Configuration After Reset -
// 1. Initialize Ports: PxDIR, PxREN, PxOUT, and PxIES
// 2. Clear LOCKLPM5
// 3. If not wake-up from LPMx.5: clear all PxIFGs to avoid erroneous port interrupts
// 4. Enable port interrupts in PxIE
//**************************************************************
#include <msp430.h>
#include <stdint.h>
#include "config_gpio_5529.h"


#define BIT(n) (1 << n)

volatile uint8_t * PIN[]  = {  &P1IN,   &P1IN,   &P2IN,   &P3IN,   &P4IN,   &P5IN,   &P6IN,  &P7IN };
volatile uint8_t * PDIR[] = { &P1DIR,  &P1DIR,  &P2DIR,  &P3DIR,  &P4DIR,  &P5DIR,  &P6DIR,  &P7DIR };
volatile uint8_t * PREN[] = { &P1REN,  &P1REN,  &P2REN,  &P3REN,  &P4REN,  &P5REN,  &P6REN,  &P7REN };
volatile uint8_t * POUT[] = { &P1OUT,  &P1OUT,  &P2OUT,  &P3OUT,  &P4OUT,  &P5OUT,  &P6OUT,  &P7OUT };
volatile uint8_t * PSEL[] = { &P1OUT,  &P1OUT,  &P2OUT,  &P3OUT,  &P4OUT,  &P5OUT,  &P6OUT,  &P7OUT };
void pinMode (uint8_t port, uint8_t bit, pinMode_t mode )
{
    // bit = [0, 1, 2, ..., 7}
    // 0 -->   0x01 (0000.0001)
    // 1 -->   0x02 (0000.0010)
    // 2 -->   0x04 (0000.0100)
    // n -->   ..
    // 7 -->   0x80 (1000.0000)

    uint8_t mask = (0x01 << bit);

    if(mode == input)
    {
        *( PDIR[port] ) &= ~mask;
    }
    if(mode == output)
    {
        *( PDIR[port] ) |= mask;
    }
    if(mode == inPullUp)
    {
        *( PDIR[port] ) &= ~mask;
        *( PREN[port] ) |=  mask;
        *( POUT[port] ) |=  mask;
    }
    if(mode == inPullDown)
    {
        *( PDIR[port] ) &= ~mask;
        *( PREN[port] ) |=  mask;
        *( POUT[port] ) &= ~mask;
    }
    if(mode == unusedPin)
    {
        *( PSEL[port] ) &= ~mask;
        *( POUT[port] ) |=  mask;
        *( PDIR[port] ) &= ~mask;
    }
}

void gpioConfig(void)
{
    //P1.0 - LED vermelho da placa
    P1SEL &= ~BIT0;              // P2.0 (I/O)
    P1OUT &= ~BIT0;              //Configurando a saida P1.0 inicialmente em estado BAIXO
    P1DIR |=  BIT0;              //Configurando pino P1.0 como saida
    // P1.2 - "INT_PCF0x20" - Recebe as interrupcoes vindas da PCF8574( pino INT)
    P1SEL &= ~BIT2;              // P1.2 (I/O)
    P1IE  |=  BIT2;              // Habilitador de interrupcaoo local
    P1IES |=  BIT2;              // Interrupt Edge Select selecionado para o flanco de subida
    P1IFG &= ~BIT2;              // Zerar a IFG, pois mexer no IES pode causar variacoes na IFG
    // P1.3 - "INT_PCF0x20" - TAMBEM recebe as interrupcoes vindas da PCF8574( pino INT), mas as do flanco de subida
    P1SEL &= ~BIT3;              // P1.3 (I/O)
    P1IE  |=  BIT3;              // Habilitador de interrupcao local
    P1IES &= ~BIT3;              // Interrupt Edge Select selecionado para o flanco de descida
    P1IFG &= ~BIT3;              // Zerar a IFG, pois mexer no IES pode causar variacoes na IFG
    // P1.4 - "Pino A do Demux, codigo A0 no CCS" - Bit menos significativo do seletor do SN74HC138N
    pinMode(1,4,unusedPin);
    // P1.5 - "Pino B do Demux, codigo A1 no CCS" - O seletor CBA controla a abertura/fechamento das 4 vï¿½lvulas
    pinMode(1,5,unusedPin);
    // P1.6 - "Pino C do Demux, codigo A2 no CCS" - O bit mais significativo do seletor do SN74HC138N
    pinMode(1,6,unusedPin);
    // P2.0 - Pino para receber a informacao de que o envio de nutrientes para T1 terminou
    pinMode(2,0,unusedPin);
//    P2SEL &= ~BIT0;              // P2.0 (I/O)
//    P2OUT |=  BIT0;              // Pull Up
//    P2REN |=  BIT0;              // Pull Up/Down habilitado
//    P2DIR &= ~BIT0;              // Configurando pino P2.0 como entrada
//    P2IE  |=  BIT0;              // Habilitador de interrupcao local
//    P2IES &= ~BIT0;              // Interrupt Edge Select selecionado para o flanco de descida
//    P2IFG &= ~BIT0;              // Zerar a IFG, pois mexer no IES pode causar variacoes na IFG
//    // P2.2 - Pino para receber a informacao de que o envio de nutrientes para T2 terminou
    pinMode(2,2,unusedPin);
//    P2SEL &= ~BIT2;              // P2.2 (I/O)
//    P2OUT |=  BIT2;              // Pull Up
//    P2REN |=  BIT2;              // Pull Up/Down habilitado
//    P2DIR &= ~BIT2;              // Configurando pino P2.2 como entrada
//    P2IE  |=  BIT2;              // Habilitador de interrupcao local
//    P2IES &= ~BIT2;              // Interrupt Edge Select selecionado para o flanco de descida
//    P2IFG &= ~BIT2;              // Zerar a IFG, pois mexer no IES pode causar variacoes na IFG
    // P2.3 - ECHO    T1
    // P2.4 - ECHO    T2
    // P2.5 - UNUSED I/O PIN
    P2SEL   &= ~BIT5;              // P2.5 (I/O)
    P2OUT   &= ~BIT5;              //Configurando a saida P2.5 inicialmente em estado BAIXO ( = demux desabilitado)
    P2DIR   |=  BIT5;              //Configurando pino P2.5 como saida
    // P2.6 - Configurado para sinalizar ao ESP32 o fim do periodo de evaporacao de cloro no T1, autorizando envio de solucao nutritiva
    pinMode(2,6,unusedPin);
    // P2.7 - Configurado para sinalizar ao ESP32 o fim do periodo de evaporacao de cloro no T2, autorizando envio de solucao nutritiva
    pinMode(2,7,unusedPin);
    // P3.2 - UNUSED I/O PIN
    pinMode(3,2,unusedPin);
    // P3.3 - UNUSED I/O PIN
    pinMode(3,3,unusedPin);                 // TXD (USCI_A0)
    // P3.4 - UNUSED I/O PIN
    pinMode(3,4,unusedPin);                 // RXD (USCI_A0)
    // P3.5 - TRIGGER T2 (TB0.5)
    // P3.6 - TRIGGER T1 (TB0.6)
    // P3.7 - UNUSED I/O PIN
    pinMode(3,7,unusedPin);
    // P4.1 - SDA do i2c
    // P4.2 - SCL do I2c
    // P4.7 - LED verde da placa, sem uso no momento
    // P6.0 - UNUSED I/O PIN
    P6SEL &= ~BIT0;              // P6.0 (I/O)
    P6OUT |=  BIT0;              //Configurando a saida P6.0 inicialmente em estado ALTO
    P6DIR |=  BIT0;              //Configurando pino P6.0 como saida
    // P6.1 - UNUSED I/O PIN
    P6SEL &= ~BIT1;              // P6.1 (I/O)
    P6OUT |=  BIT1;              //Configurando a saida P6.1 inicialmente em estado ALTO
    P6DIR |=  BIT1;              //Configurando pino P6.1 como saida
    // P6.2 - UNUSED I/O PIN
    P6SEL &= ~BIT2;              // P6.2 (I/O)
    P6OUT |=  BIT2;              //Configurando a saida P6.2 inicialmente em estado ALTO
    P6DIR |=  BIT2;              //Configurando pino P6.2 como saida
    // P6.3 - UNUSED I/O PIN
    P6SEL &= ~BIT3;              // P6.3 (I/O)
    P6OUT |=  BIT3;              //Configurando a saida P6.3 inicialmente em estado ALTO
    P6DIR |=  BIT3;              //Configurando pino P6.3 como saida
    // P6.4 - UNUSED I/O PIN
    P6SEL &= ~BIT4;              // P6.4 (I/O)
    P6OUT |=  BIT4;              //Configurando a saida P6.4 inicialmente em estado ALTO
    P6DIR |=  BIT4;              //Configurando pino P6.4 como saida
    // P6.5 - UNUSED I/O PIN
    P6SEL &= ~BIT5;              // P6.5 (I/O)
    P6OUT |=  BIT5;              //Configurando a saida P6.5 inicialmente em estado ALTO
    P6DIR |=  BIT5;              //Configurando pino P6.5 como saida
    // P6.6 - UNUSED I/O PIN
    P6SEL &= ~BIT6;              // P6.6 (I/O)
    P6OUT |=  BIT6;              //Configurando a saida P6.6 inicialmente em estado ALTO
    P6DIR |=  BIT6;              //Configurando pino P6.6 como saida
    // P7.0 - UNUSED I/O PIN
    P7SEL &= ~BIT0;              // P7.0 (I/O)
    P7OUT |=  BIT0;              //Configurando a saida P7.0 inicialmente em estado ALTO
    P7DIR |=  BIT0;              //Configurando pino P7.0 como saida
    // P7.4 - UNUSED I/O PIN
    P7SEL &= ~BIT4;              // P7.4 (I/O)
    P7OUT |=  BIT4;              //Configurando a saida P7.4 inicialmente em estado ALTO
    P7DIR |=  BIT4;              //Configurando pino P7.4 como saida
    // P8.1 - Configurado para operar com 1-Wire
    P8SEL  &=  ~BIT1;                       // I/O
    P8DIR  |=   BIT1;                       // Configurando pino P8.1 como entrada (Alta impedancia)
    P8OUT  &=  ~BIT1;                       // P8.1 = 0
    P8REN  &=  ~BIT1;                       // Pull Up/Down não habilitado, será com resistor externo
    // P8.2 - Configurado para receber o comando de acionamento da bomba que leva a solucao nutritiva para os tanques principais
    pinMode(8,2,unusedPin);
}
