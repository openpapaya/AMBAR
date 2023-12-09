/*
 * funcoes_PCF8574_5529.c
 *
 *  Created on: Mar 5, 2023
 *      Author: guiwz
 */
#include <msp430.h>
#include <stdint.h>
#include "libs/funcoes_PCF8574_5529.h"

#define     INTpin12_PCF0x20    0X0004         // P1.2 - Referenciando 0x0004 como "INTpin12_PCF0x20", equivalente ao PIN2 para a porta P1, ou 0b00000100 (76543210)
#define     INTpin13_PCF0x20    0X0004         // P1.2 - Referenciando 0x0004 como "INTpin12_PCF0x20", equivalente ao PIN2 para a porta P1, ou 0b00000100 (76543210)

//                              *** ENDERECOS I2C ***
// PCF8574_valves - CONTROLE DAS VALVULAS  - Endereco: 0x20
// PCF8574_sNivel - SENSORES DE NIVEL      - Endereco: 0x21
// PCF8574_LCD - LCD                    - Endereco: 0x27
// INA219  - Leitor Tensao/Corrente - Endereco: 0x40

volatile unsigned int flagSnivel = 0;           // Utilizados nas fun��es de rebotes
volatile unsigned int flag2Snivel = 0;           // Utilizados nas fun��es de rebotes

uint8_t writePCF8574(uint8_t addr, uint8_t byte )
{
       UCB1IFG = 0;                            // Boa pratica -  Zerar o registro de flags antes de comecar

       UCB1I2CSA = addr;                       // Confingura o endereco do escravo
       UCB1CTL1 |= UCTXSTT | UCTR;             // Requisita o inicio da comunicacao com TX. UCTXSTT vai gerar o START, UCTR eh o modo transmissor
       while(!(UCB1IFG & UCTXIFG));            //
                                               //
       UCB1TXBUF = byte;                  //                           //
       while( UCB1CTL1 & UCTXSTT );            // Espera o ciclo de ACK acontecer
       if(UCB1IFG & UCNACKIFG)                 // Se o escravo nao estiver presente
       {                                       // vou receber um NACK
           UCB1CTL1 |= UCTXSTP;                // Sou obrigado entao a transmitir um STOP
           while (UCB1CTL1 & UCTXSTP);         // Espero o stop ser de fato transmitido
           return 1;                           // e retorno um codigo de erro
           // COLOCAR aqui um LED de alerta para o erro
       }
       while(!(UCB1IFG & UCTXIFG ));           // Espera o ultimo byte ser carregado
                                               // no registro de deslocamento
       UCB1CTL1 |= UCTXSTP;                    // Peco um STOP
       while (UCB1CTL1 & UCTXSTP);             // Espero o stop ser de fato transmitido
       return 0;
}

uint8_t readPCF8574(uint8_t addr )
{
    //  *** PORTAS DA PCF8574 ***
    //  Nivel A2: P2
    //  Nivel B2: P3
    //  Nivel A1: P0
    //  Nivel B1: P1
    // ~int: P1.2 e P1.3
    i2cRead_UCB1(addr);
    __delay_cycles(1000);
    return read_PCF8574;
}
// Interrupcao da PCF8574 dos sensores de nivel
#pragma vector = PORT1_VECTOR
__interrupt void SNIVEL_TANQUE1e2_ISR()
{
    // P1.2 - Recebe a interrupcao da PCF8574 dos sensores de nivel

     switch (P1IV)
    {
    case 0x00:      // No interrupt pending
        break;
    case 0x02:      //P1.0 - Interrupt Source: Port 1.0 interrupt; Interrupt Flag: P1IFG.0; Interrupt. Priority: Highest
        break;
    case 0x04:      //P1.1 - Interrupt Source: Port 1.1 interrupt; Interrupt Flag: P1IFG.1
        break;
    case 0x06:      // P1.2 - INTpin12_PCF0x20
        // ATENCAO, talvez nao seja preciso apagar o IFG, o proprio IV j� faz isso
        // PM5CTL0 = ~LOCKLPM5;
        P1IFG &= ~INTpin12_PCF0x20;
        P1IE  &= ~INTpin12_PCF0x20;                     // Desativa as proximas interrupcoes pelo BIT2 para que novos eventos nao ocorram
        P1IFG &= ~BIT2;                                 //Zerar a IFG
        flagSnivel  = 1;
        flag2Snivel  = 1;
        break;
    case 0x08:      //P1.3
        // ATENCAO, talvez nao seja preciso apagar o IFG, o proprio IV j� faz isso
        // PM5CTL0 = ~LOCKLPM5;
        P1IFG &= ~INTpin13_PCF0x20;
        P1IE  &= ~INTpin13_PCF0x20;                     // Desativa as proximas interrupcoes pelo BIT2 para que novos eventos nao ocorram
        P1IFG &= ~BIT3;                                 //Zerar a IFG
        flagSnivel  = 1;
        flag2Snivel  = 1;
        break;
    case 0x0A:      //P1.4
        break;
    case 0x0C:      //P1.5
        break;
    case 0x0E:      //P1.6
        break;
    case 0x10:      //P1.7
        break;
    }
}
//--------------------------------------------------------------------------------------
