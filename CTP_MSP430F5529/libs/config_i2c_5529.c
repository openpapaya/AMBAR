/*
 * I2C.c
 *
 *  Created on: 5 de jul de 2021
 *      Author: guiwz
 */

#include <msp430.h>
#include <stdint.h>

#include <libs/config_i2c_5529.h>
#include <libs/config_timers_5529.h>

//                             *** ENDERECOS I2C ***
// PCF8574 - CONTROLE DAS VALVULAS  - Endereco: 0x20
// PCF8574 - SENSORES DE NIVEL      - Endereco: 0x21
// PCF8574 - LCD                    - Endereco: 0x27
// INA219  - Leitor Tensao/Corrente - Endereco: 0x40
// ESP32   - Cliente MQTT           - Endereco: 0x42
#define      PCF8574_valves  0x20
#define      PCF8574_sNivel  0x21
#define      ESP32           0x42
#define      INA219          0x40
#define      PCF8574_LCD1    0x27
#define      PCF8574_LCD2    0x26

uint8_t TXByteCtr = 1;
uint8_t TXESP32Data;
uint8_t TXdata;

void i2cConfig()
{
    //    UCB1CTL1  = UCSWRST;                     // Reseta a interface, ou seja, desliga o modulo subindo o RESET
    //    UCB1CTL0  = UCMST | UCMODE_3 | UCSYNC;   // Interface pe mestre, modo I2C, sincrono
    //    UCB1CTL1 |= UCSSEL__SMCLK | UCSWRST;               // Usa SMCLK @1MHz
    //    UCB1BRW   = 10;                         // SCL @ 100kHz => SMCLK / 10 = 100kHz
    //
    //    P4SEL |=   BIT2 | BIT1;                 // UCB1: Portas P4.1 (SDA) e P4.2 (SCL) sao para a comunicacao i2c
    //
    //    UCB1CTL1 &= ~UCSWRST;                   // Zera o bit de RST para deixar a interface funcionar, ou seja, liga o modulo

    P4SEL    |=   BIT2 | BIT1;                   // Assign I2C pins to USCI_B1
    UCB1CTL1 |= UCSWRST;                      // Enable SW reset
    UCB1CTL0  = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
    UCB1CTL1  = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
    UCB1BRW   = 10;                         // SCL @ 100kHz => SMCLK / 10 = 100kHz
    UCB1CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
//    UCB1IE   |= UCTXIE;                         // Enable TX interrupt
//    UCB1IE   |= UCRXIE;                         // Enable RX interrupt

}
//------------------------------------------------------------------------------
// The USCIAB0_ISR is structured such that it can be used to transmit any
// number of bytes by pre-loading TXByteCtr with the byte count.
//------------------------------------------------------------------------------
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USCI_B1_VECTOR
__interrupt void USCI_B1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_B1_VECTOR))) USCI_B1_ISR (void)
#else
#error Compiler not supported!
#endif
{
    switch(__even_in_range(UCB1IV,12))
    {
    case  0: break;                           // Vector  0: No interrupts
    case  2: break;                           // Vector  2: ALIFG
    case  4: break;                           // Vector  4: NACKIFG
    case  6: break;                           // Vector  6: STTIFG
    case  8: break;                           // Vector  8: STPIFG
    case 10:                                  // Vector 10: RXIFG

        read_PCF8574 = UCB1RXBUF;
        UCB1CTL1 |= UCTXSTP;                  // I2C stop condition
        UCB1IFG &= ~UCRXIFG;                  // Clear USCI_B1 TX int flag
        // flag para avisar de nova leitura

        break;
    case 12:                                  // Vector 12: TXIFG
        if (TXByteCtr)                          // Check TX byte counter
        {
            UCB1TXBUF = TXdata;              // Load TX buffer
            TXByteCtr--;                          // Decrement TX byte counter
        }
        else
        {
            UCB1CTL1 |= UCTXSTP;                  // I2C stop condition
            UCB1IFG &= ~UCTXIFG;                  // Clear USCI_B1 TX int flag
        }
        break;
    default: break;
    }
}
void i2cRead_UCB1(uint8_t addr)
{
    //    P4REN |=   BIT2 | BIT1;                   // Assign I2C pins to USCI_B1
    //    P4OUT |=   BIT2 | BIT1;                   // Assign I2C pins to USCI_B1

    //   --- Inicializacao ---
    P4SEL    |=   BIT2 | BIT1;                   // Assign I2C pins to USCI_B1
    UCB1CTL1 |= UCSWRST;                      // Enable SW reset
    UCB1CTL0  = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
    UCB1CTL1  = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
    UCB1BRW   = 10;                         // SCL @ 100kHz => SMCLK / 10 = 100kHz
    //
    UCB1I2CSA = addr;
    UCB1CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
    UCB1IE   |= UCRXIE;                         // Enable TX interrupt
 //   UCB1IE   |= UCTXIE;                         // Enable TX interrupt
    UCB1CTL1 &= ~UCTR;                  // Modo receptor
    UCB1CTL1 |= UCTXSTT;                // Requisita o inicio da comunicacao com TX. UCTXSTT gera o START

}
uint8_t i2cWrite_UCB1(uint8_t addr, uint8_t  byte, uint8_t nBytes)
{
    //    P4REN |=   BIT2 | BIT1;                   // Assign I2C pins to USCI_B1
    //    P4OUT |=   BIT2 | BIT1;                   // Assign I2C pins to USCI_B1
    //
    P4SEL     |=   BIT2 | BIT1;                   // Assign I2C pins to USCI_B1
    UCB1CTL1  |= UCSWRST;                      // Enable SW reset
    UCB1CTL0   = UCMST + UCMODE_3 + UCSYNC;     // I2C Master, synchronous mode
    UCB1CTL1   = UCSSEL_2 + UCSWRST;            // Use SMCLK, keep SW reset
    UCB1BRW    = 10;                           // SCL @ 100kHz => SMCLK / 10 = 100kHz
    UCB1I2CSA  = addr;                         // Slave Address is 048h
    UCB1CTL1  &= ~UCSWRST;                     // Clear SW reset, resume operation
    UCB1IE    = UCTXIE;                        // Enable TX interrupt
    TXByteCtr  = 1;                          // Load TX byte counter
    TXdata     = byte;                            // Holds TX data
    UCB1CTL1  |= UCTR + UCTXSTT;             // I2C TX, start condition
    return 0;
}
// Versao antiga:
//uint8_t i2cWrite_UCB1(uint8_t addr, uint8_t * data, uint8_t nBytes)
//{
//   // __disable_interrupt();
//    UCB1IFG = 0;                            // Boa pratica -  Zerar o registro de flags antes de comecar
//
//    UCB1I2CSA = addr;                       // Confingura o endereco do escravo
//    UCB1CTL1 |= UCTXSTT | UCTR;             // Requisita o inicio da comunicacao com TX. UCTXSTT vai gerar o START, UCTR eh o modo transmissor
//    while(!(UCB1IFG & UCTXIFG));            //
//                                            //
//    UCB1TXBUF = * data ++;                  //
//    nBytes--;                               //
//    while( UCB1CTL1 & UCTXSTT );            // Espera o ciclo de ACK acontecer
//    if(UCB1IFG & UCNACKIFG)                 // Se o escravo nao estiver presente
//    {                                       // vou receber um NACK
//        UCB1CTL1 |= UCTXSTP;                // Sou obrigado entao a transmitir um STOP
//        while (UCB1CTL1 & UCTXSTP);         // Espero o stop ser de fato transmitido
//     //   __enable_interrupt();
//        return 1;                           // e retorno um codigo de erro
//        // COLOCAR aqui um LED de alerta para o erro
//    }
//    while(nBytes--)     //
//    {
//        while(!(UCB1IFG & UCTXIFG ));       // Escreve no buffer de TX
//        UCB1TXBUF = *data++;                // para destravar o ciclo de ACK
//    }
//    while(!(UCB1IFG & UCTXIFG ));           // Espera o ultimo byte ser carregado
//                                            // no registro de deslocamento
//    UCB1CTL1 |= UCTXSTP;                    // Peco um STOP
//    while (UCB1CTL1 & UCTXSTP);             // Espero o stop ser de fato transmitido
//   // __enable_interrupt();
//    return 0;
//}

// Slave Address is 042h
uint8_t i2cWrite_UCB0(uint8_t addr, uint8_t * data, uint8_t nBytes)
{
    //   __disable_interrupt();
    UCB0IFG = 0;                            // Boa pratica -  Zerar o registro de flags antes de comecar

    UCB0I2CSA = addr;                       // Confingura o endereco do escravo
    UCB0CTL1 |= UCTXSTT | UCTR;             // Requisita o inicio da comunicacao com TX. UCTXSTT vai gerar o START, UCTR eh o modo transmissor
    while(!(UCB0IFG & UCTXIFG));            //
    //
    UCB0TXBUF = * data ++;                  //
    nBytes--;                               //
    while( UCB0CTL1 & UCTXSTT );            // Espera o ciclo de ACK acontecer
    if(UCB0IFG & UCNACKIFG)                 // Se o escravo nao estiver presente
    {                                       // vou receber um NACK
        UCB0CTL1 |= UCTXSTP;                // Sou obrigado entao a transmitir um STOP
        while (UCB0CTL1 & UCTXSTP);         // Espero o stop ser de fato transmitido
        //   __enable_interrupt();
        return 1;                           // e retorno um codigo de erro
        // COLOCAR aqui um LED de alerta para o erro
    }
    while(nBytes--)     //
    {
        while(!(UCB0IFG & UCTXIFG ));       // Escreve no buffer de TX
        UCB0TXBUF = *data++;                // para destravar o ciclo de ACK
    }
    while(!(UCB0IFG & UCTXIFG ));           // Espera o ultimo byte ser carregado
    // no registro de deslocamento
    UCB0CTL1 |= UCTXSTP;                    // Peco um STOP
    while (UCB0CTL1 & UCTXSTP);             // Espero o stop ser de fato transmitido
    return 0;
}
uint8_t i2cWriteByte_UCB1(uint8_t addr, uint8_t byte)
{
    return i2cWrite_UCB1(addr, byte, 1);
}
uint8_t i2cWriteByte_UCB0(uint8_t addr, uint8_t byte)
{
    return i2cWrite_UCB0(addr, &byte, 1);
}
// ------------------------------------------------------------------
// "i2cwriteByte (uint8_t addr, uint8_t byte)" � o "portao de embarque" para os bytes de comunicacao
// com o LCD, por meio do protocolo i2c. O byte de dados � enviado em duas partes, sempre no MSnibble
// com o LSnibble sendo reservado para os bits de controle, BL, EN, W/~R, RS
// tendo como caracter�stica o envio de tres "pacotes", o primeiro com EN=0, o segundo EN=1
// e o último com EN=0:                 __
//                             EN --> __|  |__
// ------------------------------------------------------------------
// No flanco de descida do bit de enable o LCD l� os dados nos pinos
//}
// Exemplo, enviando a letra A para o LCD na posicao em que o cursor estiver, apos isso o cursor move-se automaticamente
// para a proxima posicao
// 1 - Primeiros sinais enviados pelo I2C: START (abaixa a linha) + Addr(7 bits) + R/~W (0 para escrever)
// No caso de um periferico com endere�o 0x27  => Start + 010 0111 + 0(escrita) , 0 (recebeu o ACK) => Segue para envio
// apos isso, envia as linhas abaixo
// Uma palavra = 2 bytes = 0x49 = 0100 1001
// Uma palavra = 2 bytes = 0x4D = 0100 1101
// Uma palavra = 2 bytes = 0x49 = 0100 1001

// Uma palavra = 2 bytes = 0x19 = 0001 1001
// Uma palavra = 2 bytes = 0x1D = 0001 1101
// Uma palavra = 2 bytes = 0x19 = 0001 1001
