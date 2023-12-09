/*
 * uartAmbar5529.c
 *
 *  Created on: 6 de fev de 2023
 *      Author: guiwz
 */
#include <msp430.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "libs/ambar_uart_5529.h"
#include "libs/ambar_valvulas_5529.h"
#include "libs/ambar_lcd_5529.h"
#include "libs/config_lcd_5529.h"
#include "libs/config_timers_5529.h"

#define     PCF8574_LCD1        0x27
#define     PCF8574_LCD2        0x26

volatile uint8_t comando      = 0;
volatile uint8_t process      = 0;
volatile uint8_t flagOpCmd    = 0;
volatile uint8_t flag_softDog = 0;
volatile uint8_t countExit    = 7;
volatile uint8_t flagOpCmd_UCA0    = 0;
volatile uint8_t flagDefaultLayout = 0;
volatile uint8_t flagClrBuff = 0;
volatile uint8_t readChars   =0;
volatile uint8_t flagCmdFind = 0;                          // Flag para entrar nas funcoes de testes do sistema
volatile uint8_t flagTestConfirm = 0;
volatile uint8_t id_ok = 0;
volatile uint8_t serialBegin = 0;

extern char fromESP32[0];

// INA219

volatile char stringcccc  [4];
volatile char ina219 [5];

// Testes no sistema
volatile uint8_t resultStrCmp = 0;
char   T1INVABRIR[] = "T1IN.ABRIR";
char  T1INVFECHAR[] = "T1IN.FECHAR";
char  T1OUTVABRIR[] = "T1OUT.ABRIR";
char T1OUTVFECHAR[] = "T1OUT.FECHAR";
char   T2INVABRIR[] = "T2IN.ABRIR";
char  T2INVFECHAR[] = "T2IN.FECHAR";
char  T2OUTVABRIR[] = "T2OUT.ABRIR";
char T2OUTVFECHAR[] = "T2OUT.FECHAR";
char MONITOR[] = "MONITOR";
char stringCmd[16];

extern uint8_t atualizarLCD;
extern volatile unsigned int t1_closeRegIN, t2_closeRegIN, t1_closeRegOUT, t2_closeRegOUT;             // Solicitaï¿½ï¿½o de fechamento e abertura de vï¿½lvulas
extern volatile unsigned int t1_openRegIN, t2_openRegIN, t1_openRegOUT, t2_openRegOUT;             // Solicitaï¿½ï¿½o de fechamento e abertura de vï¿½lvulas

char charCmd = 0;
// \a - Alarm, Alarme = Toca o alarme sonoro do sistema
// \b - Back space, Retrocesso = Apaga o caractere Ã  esquerda do cursor
// \n - NewLine, Nova linha = Pula uma linha
// \t - TabulaÃ§Ã£o horizontal = Equivale Ã  dar um TAB na string
// \r - Carriage Return, Retorno do Carro = Volta para o inÃ­cio da linha.
// \0 - Null, Nulo = Caracter nulo ou zero geralmente estabelecido como fim de string
// \v â€“ TabulaÃ§Ã£o Vertical
// \\ â€“ Caracter \
// \â€™ â€“ Caracter â€˜
// \â€� â€“ Caracter â€œ
// \? â€“ Caracter ?
// %% â€“ Caracter %
//
//
#pragma vector = USCI_A1_VECTOR
__interrupt void UART_A1()
{
    // Ler o buffer apenas para zera a flag. Testar usar "UCA1IV; // Apagar RXIFG"
    if( UCA1RXBUF == 0x0D)
    {

    }
    flagOpCmd = 1;
}
void uart_operador_ambar5529()
{
    if(atualizarLCD == 0)
    {
        if( UCA1RXBUF == 0x0D)
        {
            // Interpretacao da tecla "ENTER"

            process++;                          // process=1 entra no menu, process=2 terminou a escrita, process = 3 confirma cmd
            // sinalizando para ir conferir qual foi a msg
            process &= 0x03;                    // 0x03 = 0000 0011. Contador circular com 4 posicoes
            //flagOpCmd = 1;                      // Entrar na funcao operadora de comandos no terminal
        }           // Logica do contador circular:
        //          0001&0011 = 2   Primeiro "Enter", entrar no menu
        //          0010&0011 = 2   Segundo  "Enter", conferir a msg
        //          0011&0011 = 3   Terceiro "Enter", confirmar a instrucao dada
        //          0100&0011 = 0
        //          0101&0011 = 1
        // Ver no terminal a letra que foi escrita
        // Se o buffer nÃ£o estiver cheio
        // Escreva no buffer
        //
        // Vai colocando no rx.buffer as letras digitadas
        // Essa operacao garante que o indexador nao avance o tamanho do buffer
        else if( UCA1RXBUF == 0x1B)             // Se pressionou a tecla "ESC"
        {
            flagClrBuff = 1;
            flagOpCmd = 1;
            serialBegin = 0;
        }
        else if( UCA1RXBUF == 0x20)             // Se pressionou a tecla "ESPACE"
        {

        }
        else if( UCA1RXBUF == 0x08)             // Se pressionou a tecla "Backspace", cancelar o ultimo caractere inserido
        {
            rx.write--;                         // Zerar o ponteiro de escrita do buffer
            rx.size--;                          // Zerar o indexador do buffer circular
        }
    }

    if(process == 0)
    {
        if(serialBegin == 0)                     // Se é a primeira entrada serial
        {
            if(atualizarLCD == 1)
            {
                uartWrite(" Insira a senha para controle da central e tecle ENTER, ou ESC para sair: \n\r");
                lcdLayout(PCF8574_LCD1,senhacentral);
                lcdCursor(PCF8574_LCD1,0x55);
                // Programar o LCD para piscar o cursor
                lcdWriteByte(PCF8574_LCD1,0x0F, INSTR);              // 0  0  0  0  1  D  C  B,
                // Ligar o cursor e o blink
                serialBegin = 1;
                atualizarLCD = 0;
                flagOpCmd = 0;
                flag_softDog = 1;
            }
        }
    }
    if(serialBegin == 1)
    {
        if( UCA1RXBUF != 0x0D && 0x1B )             // Desconsidera os caracteres de "ENTER",  "ESC",
        {                                   //
            UCA1TXBUF = UCA1RXBUF;                  // Escreve no terminal a palavra digitada
            countExit = 7;
            if(rx.size < BUFFER_SIZE)               //
            {                                       //
                rx.buffer[rx.write++] = UCA1RXBUF;      //
                rx.write &= 0x0F;
                rx.refresh = 1;//
                rx.size++;
            }
            else                            // Caso o buffer esteja cheio5
            {                               // descarte o byte que foi
                UCA1IFG &= ~UCRXIFG;        // enviado.
            }
            if(rx.refresh)                      // Serve para marcar que um novo caractere foi recebido no UCA1RXBUF
            {                                   // e assim escrever esse caractere no LCD e no terminal
                charCmd = rx.buffer[rx.i];      // Armazena em "charCmd" o caractere recebido na UART
                stringCmd[rx.i] = rx.buffer[rx.i];   // Salva o conteudo do buffer numa nova string, essa string esta sem utilidade
                lcdWriteByte(PCF8574_LCD1,charCmd, CHAR);    // Escreve no LCD letra por letra digitada
                rx.i++;                         // Ponteiro aponta para a proxima posicao de memoria a ser repassada ao LCD
                rx.i &= 0x0F;                   // Contador circular. Na pratica limita o buffer a conter comandos de 16 caracteres max
                rx.refresh ^= 1;                // Leva rx.refresh para zero
            }
        }
        if(process == 1)
        {
            resultStrCmp = strcmp(rx.buffer,"123");
            if(resultStrCmp == 0)
            {
                id_ok = 1;
            }
            else
            {
                process = 0;
                uartWrite(" Senha incorreta, tente novamente ou ESC para sair\n\r ");
                lcdLayout(PCF8574_LCD1,senhaincorreta);
            }
            int s = 16;
            while(s)
            {
                rx.buffer[rx.read] = 0;
                rx.read++;
                rx.read &= 0x0F;
                s--;
            }
            rx.i     = 0;
            rx.read  = 0;                       // Zerar o ponteiro de leitura do buffer
            rx.write = 0;                       // Zerar o ponteiro de escrita do buffer
            rx.size  = 0;
            rx.refresh = 0;
        }
        flagOpCmd = 0;                      // Para nao ficar entrando em loop no operador da UART
    }
    if(flagClrBuff == 1)                    // Tecla "ESC" ou comando voltar no app
    {
        int s = 16;
        while(s)
        {
            rx.buffer[rx.read] = 0;
            rx.read++;
            rx.read &= 0x0F;
            s--;
        }
        serialBegin = 0;
        rx.i     = 0;
        rx.read  = 0;                       // Zerar o ponteiro de leitura do buffer
        rx.write = 0;                       // Zerar o ponteiro de escrita do buffer
        rx.size  = 0;
        rx.refresh = 0;
        flagOpCmd = 0;
        if(atualizarLCD == 0)               // e a flag "atualizarLCD" estiver abaixada, voltar para a tela default
        {
            lcdWriteByte(PCF8574_LCD1,0x0C, INSTR);      // Desligar o cursor e o blink 0  0  0  0  1  D  C  B,
            lcdLayout(PCF8574_LCD1,defaut);
            operador_LCD(PCF8574_LCD1);
            uartMonitorAmbar();            // Tela default do terminal serial
            atualizarLCD = 1;
        }
        process  = 0;                       // Zerar o contador circular
        id_ok = 0;
        flagClrBuff = 0;
    }
    if( id_ok)
    {
        if(process == 1)
        {                                 //
            if( UCA1RXBUF != 0x0D && 0x1B )             // Desconsidera os caracteres de "ENTER",  "ESC",
            {                                   //
                UCA1TXBUF = UCA1RXBUF;                  // Escreve no terminal a palavra digitada
                if(rx.size < BUFFER_SIZE)               //
                {                                       //
                    rx.buffer[rx.write++] = UCA1RXBUF;      //
                    rx.write &= 0x0F;
                    rx.refresh = 1;//
                    rx.size++;
                }
                else                            // Caso o buffer esteja cheio
                {                               // descarte o byte que foi
                    UCA1IFG &= ~UCRXIFG;        // enviado.
                }
                //    flagOpCmd = 1;
            }
        }
        if(process == 0)                        // Se voltar para o contador circular na posicao 0
        {
            if(atualizarLCD == 0)               // e a flag "atualizarLCD" estiver abaixada, voltar para a tela default
            {
                lcdWriteByte(PCF8574_LCD1,0x0C, INSTR);              // 0  0  0  0  1  D  C  B,
                // Desligar o cursor e o blink
                lcdLayout(PCF8574_LCD1,defaut);
                uartMonitorAmbar();                 // Tela default do terminal serial
                atualizarLCD = 1;
            }
        }
        if(process == 1)                        // Somento o caractere ENTER leva ao process = 1. Cada vez que ENTER eh pressionado
        {                                       // process eh incrementado: 0x01&=0x03 -> 0x02&=0x03 -> 0x03&=0x03 -> 0x00&=0x03 -> 0x01&=0x03
            if( serialBegin == 1)              // Se eh o primeiro enter para entrar no menu tecnico, desligar a atualizacao default do LCD
            {
                serialBegin = 0;               // Desliga a atualizacao do LCD
                uartWrite(" Testes, digite o comando + ENTER, ou ESC para sair\n\r");
                lcdLayout(PCF8574_LCD1,menucmd);
                lcdCursor(PCF8574_LCD1,0x55);
                // Programar o LCD para piscar o cursor
                lcdWriteByte(PCF8574_LCD1,0x0F, INSTR);              // 0  0  0  0  1  D  C  B,
                // Ligar o cursor e o blink
            }
            if(rx.refresh)                      // Serve para marcar que um novo caractere foi recebido no UCA1RXBUF
            {                                   // e assim escrever esse caractere no LCD e no terminal
                charCmd = rx.buffer[rx.i];      // Armazena em "charCmd" o caractere recebido na UART
                stringCmd[rx.i] = rx.buffer[rx.i];   // Salva o conteudo do buffer numa nova string, essa string esta sem utilidade
                lcdWriteByte(PCF8574_LCD1,charCmd, CHAR);    // Escreve no LCD letra por letra digitada
                rx.i++;                         // Ponteiro aponta para a proxima posicao de memoria a ser repassada ao LCD
                rx.i &= 0x0F;                   // Contador circular. Na pratica limita o buffer a conter comandos de 16 caracteres max
                rx.refresh ^= 1;                // Leva rx.refresh para zero
            }
            //////////////////////////////////////////////////
            // Visor do LCD: 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F 0x10 0x11 0x12 0x13
            //               0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F 0x50 0x51 0x52 0x53
            //               0x14 0x15 0x16 0x17 0x18 0x19 0x1A 0x1B 0x1C 0x1D 0x1E 0x1F 0x20 0x21 0x22 0x23 0x24 0x25 0x26 0x27
            //               0x54 0x55 0x56 0x57 0x58 0x59 0x5A 0x5B 0x5C 0x5D 0x5E 0x5F 0x60 0x61 0x62 0x63 0x64 0x65 0x66 0x67
        }
        if( process == 2)         // Segundo ENTER apertado, significa que o usuario terminou o comando
        {
            // Comparando duas strings
            //
            uartWrite("\n");                        // Pular para linha de baixo no terminal
            uartWrite("\r");                        // Retorno do carro no terminal
            // ***** AVALIANDO SE O QUE FOI ESCRITO EH UM COMANDO VALIDO *****
//            resultStrCmp = strcmp(rx.buffer,"init prep");
//            if(resultStrCmp == 0)
//            {
//                lcdWriteByte(PCF8574_LCD1,0x0C, INSTR);          // 0  0  0  0  1  D  C  B,
//                // Desligar o cursor e o blink
//                flagClrBuff = 1;                    // Limpar o buffer
//                flagOpCmd   = 1;                    // Permitir a entrada novamente na funcao para poder limpar o buffer
//                return 0;
//            }
            resultStrCmp = strcmp(rx.buffer,"MONITOR");
            if(resultStrCmp == 0)
            {
                lcdWriteByte(PCF8574_LCD1,0x0C, INSTR);          // 0  0  0  0  1  D  C  B,
                // Desligar o cursor e o blink
                flagClrBuff = 1;                    // Limpar o buffer
                flagOpCmd   = 1;                    // Permitir a entrada novamente na funcao para poder limpar o buffer
                return 0;
            }
            resultStrCmp = strcmp(rx.buffer,"monitor");
            if(resultStrCmp == 0)
            {
                lcdWriteByte(PCF8574_LCD1,0x0C, INSTR);          // 0  0  0  0  1  D  C  B,
                // Desligar o cursor e o blink
                flagClrBuff = 1;                    // Limpar o buffer
                flagOpCmd   = 1;                    // Permitir a entrada novamente na funcao para poder limpar o buffer
                return 0;
            }
            resultStrCmp = strcmp(rx.buffer,"Monitor");
            if(resultStrCmp == 0)
            {
                lcdWriteByte(PCF8574_LCD1,0x0C, INSTR);          // 0  0  0  0  1  D  C  B,
                // Desligar o cursor e o blink
                flagClrBuff = 1;                    // Limpar o buffer
                flagOpCmd   = 1;                    // Permitir a entrada novamente na funcao para poder limpar o buffer
                return 0;
            }
            // *****
            resultStrCmp = strcmp(rx.buffer,"1");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_openRegIN = 1;             //
            }
            // *****
            resultStrCmp = strcmp(rx.buffer,"T1IN.ABRIR");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_openRegIN = 1;             //
            }
            resultStrCmp = strcmp(rx.buffer,"ABRIR ENTRADA T1");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_openRegIN = 1;             //
            }
            resultStrCmp = strcmp(rx.buffer,"Abrir entrada T1");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_openRegIN = 1;             //
            }
            resultStrCmp = strcmp(rx.buffer,"abrir entrada t1");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_openRegIN = 1;             //
            }
            // *****
            resultStrCmp = strcmp(rx.buffer,"2");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_closeRegIN = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"T1IN.FECHAR");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_closeRegIN = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"FECHAR ENTRADA T1");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_closeRegIN = 1;             //
            }
            resultStrCmp = strcmp(rx.buffer,"Fechar entrada T1");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_closeRegIN = 1;             //
            }
            resultStrCmp = strcmp(rx.buffer,"fechar entrada t1");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_closeRegIN = 1;             //
            }
            // *****
            resultStrCmp = strcmp(rx.buffer,"3");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_openRegOUT = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"T1OUT.ABRIR");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_openRegOUT = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"ABRIR SAIDA T1");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_openRegOUT = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"Abrir saida T1");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_openRegOUT = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"abrir saida t1");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_openRegOUT = 1;
            }
            // ******
            resultStrCmp = strcmp(rx.buffer,"4");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_closeRegOUT = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"T1OUT.FECHAR");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_closeRegOUT = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"FECHAR SAIDA T1");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_closeRegOUT = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"Fechar saida T1");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_closeRegOUT = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"fechar saida t1");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t1_closeRegOUT = 1;
            }
            // ***** Tanque 2 *****
            resultStrCmp = strcmp(rx.buffer,"5");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_openRegIN = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"T2IN.ABRIR");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_openRegIN = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"ABRIR ENTRADA T2");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_openRegIN = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"Abrir entrada T2");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_openRegIN = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"abrir entrada t2");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_openRegIN = 1;
            }
            //*****
            resultStrCmp = strcmp(rx.buffer,"6");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_closeRegIN = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"T2IN.FECHAR");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_closeRegIN = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"FECHAR ENTRADA T2");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_closeRegIN = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"Fechar entrada T2");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_closeRegIN = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"fechar entrada t2");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_closeRegIN = 1;
            }
            //*****
            resultStrCmp = strcmp(rx.buffer,"7");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_openRegOUT = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"T2OUT.ABRIR");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_openRegOUT = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"ABRIR SAIDA T2");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_openRegOUT = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"Abrir saida T2");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_openRegOUT = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"abrir saida t2");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_openRegOUT = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"8");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_closeRegOUT = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"T2OUT.FECHAR");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_closeRegOUT = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"FECHAR SAIDA T2");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_closeRegOUT = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"Fechar saida T2");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_closeRegOUT = 1;
            }
            resultStrCmp = strcmp(rx.buffer,"fechar saida t2");
            if(resultStrCmp == 0)             // Correspondencia encontrada
            {
                flagCmdFind = 1;
                t2_closeRegOUT = 1;
            }
            //
            if(flagCmdFind == 0)             // Nao encontrou correspondencia com os comandos
            {
                int s = 16;
                while(s)
                {
                    rx.buffer[rx.read] = 0;
                    rx.read++;
                    rx.read &= 0x0F;
                    s--;
                }
                serialBegin = 0;
                rx.i     = 0;
                rx.read  = 0;                       // Zerar o ponteiro de leitura do buffer
                rx.write = 0;                       // Zerar o ponteiro de escrita do buffer
                rx.size  = 0;
                rx.refresh = 0;
                process = 1;                // Volta para o recebimento de comando
                // Enviar msg de comando errado e sugerir apertar "ENTER" para tentar de novo ou "ESC" para sair
                uartWrite("Comando não existe, tente novamente ou ESC para sair\n\r");
                lcdLayout(PCF8574_LCD1,errodecmd);

            }
            else if(flagCmdFind == 1)
            {
                uartWrite("Pressione ENTER para confirmar a acao\n\r");
                lcdWriteByte(PCF8574_LCD1,0x0C, INSTR);      // Desligar o cursor e o blink. 0  0  0  0  1  D ~C  ~B
                lcdLayout(PCF8574_LCD1,confirmacaocmd);      // Esperar confirmacao de escolha do comando encontrado
            }
        }
        if( process == 3)                       // Comando confirmado, levantar flags para efetivar o comando
        {
            if(flagCmdFind == 1)
            {
                // Realizar comando
                flagTestConfirm = 1;                // Flag utilizada logo abaixo, para entrar na funcao "testesAmbar5529()"
                flagClrBuff = 1;                    // Zerar todo o conteudo do buffer que armazena os comandos digitados
                flagCmdFind = 0;
            }
        }
        flagOpCmd = 0;                          // Zera a flag para entrar no operador do uart, mas se as opcoes abaixo forem verdadeiras, continua.
        if(flagTestConfirm == 1)
        {
            // Realizar comando
            testesAmbar5529();                  // Aciona o dispositivo solicitado
            uartWrite("ATENCAO: Resete a central apos efetivar comandos no menu tecnico");
            uartWrite("\n\r");                  //  Pular para linha de baixo no terminal, retorno do carro no terminal
            flagTestConfirm = 0;
            flagClrBuff = 1;
            flagOpCmd = 1;
            id_ok = 0;
        }
    }

}
void uartRead(char*str)
{
    //                     wr           Se eu olhar apenas para os ponteiros n eh possivel dizer se o buffer esta cheio ou vazio
    //          |S|M|I|C|2|1|S|I|       preciso entao do size++ e size-- para saber o tamanho do buffer, se esta cheio, se esta
    //                     rd           vazio
    //
    readChars = rx.size;            // Guarda a quantidade de caracteres digitados
    if(rx.size > 0)
    {
        //   rx.size--;                            // Decrementa o tamanho do buffer, porque o ultimo caractere foi o codigo
        while(rx.size)                        // da tecla "ENTER" e queremos so o que foi passado como comando
        {
            // if(rx.buffer[rx.read] != 0x0D){   // Enquanto nao chegar no codigo 0x0D (eh a tecla "ENTER") continue copiando
            *str++ = rx.buffer[rx.read++];    // Faz a leitura do buffer e armazena o caractere na string palavraCmd
            rx.read &= 0x0F;                  // Essa operacao garante que o index n avance o tamanho do buffer
            rx.size--;                        // Decrementa o tamanho do buffer
            // }
        }
    }
    else
    {
        //        return 1;
    }
}
//---- Funcao para converter float em string --------------
void ina219_FloatParaString(float v)
{
    volatile unsigned int x = 0;
    volatile char y = 0;
    volatile unsigned int i = 0;
    volatile unsigned int p = 0;

    // Separando milhar
    p = v/1000;                      // Separar centena, e dessa forma escrever o nÃºmero referente na tela
    y = (int)p;                      // separa o inteiro da parte fracionaria
    // Ex: x = #4095,731 --> f = 4095,731/1000 = 4.095731 --> z = (int)f = #4
    if(y == 0)                   // Se a casa do milhar nao existe no valor, nao imprima
    {
        ina219[i++]= 0x30;
    }
    else
        ina219[i++]= 0x30 + y;
    ina219[i++]= ',';
    // Separando centena
    v = v - 1000*y;                  // v = 4095,731 - 1000*4 = 95,731
    p = v/100;                       // f = 95,731/100 = 0.95731
    y = (int)p;                      // separa o inteiro da parte fracionaria
    // z = 0
    if(x == 1)                   // Se o contador aponta que a casa do milhar nao existe
    {
        if(y == 0)               // Se a casa da centena existe, imprima
            x++;                 // x = 2
    }
    else                         // Se a casa do milhar ja existe, continue imprimindo normalmente
    {
        ina219[i++]= 0x30 + y;
    }
    // Separando dezena
    v = v - 100*y;                   // v = 095,731 - 100*0 = 95,731
    p = v/10;                        // f = 9.5731
    y = (int)p;                      // separa o inteiro da parte fracionaria
    // z = 9
    if(x == 2)                   // Se o contador aponta que a casa do milhar e da centena nao existem
    {
        if(y == 0)               // Se a da dezena existe, imprima
            x++;                 // x = 3
    }
    else                         // Se a casa do milhar ou da centena ja existe, continue imprimindo normalmente
    {
        ina219[i++]= 0x30 + y;
    }
    // Separando unidade
    v = v - 10*y;                    // v = 95,731 - 10*9 = 5,731
    y = (int)v;
    ina219[i++]= 0x30 + y;
//    // Separando primeira casa decimal
//    v = v - y;                       // v = 5,731 - 5 = 0,731
//    v = 10*v;                        // v = 7,31
//    y = (int)v;
//    ina219[i++]= ',';
//    ina219[i++]= 0x30 + y;
//    // Separando segunda casa decimal
//    v = v - y;                       // v = 7,31 - 7 = 0,31
//    v = 10*v;                        // v = 3,1
//    y = (int)v;
//
//    ina219[i++]= 0x30 + y;
//
//    // Separando terceira casa decimal
//    v = v - y;                       // v = 3,1 - 3 = 0,1
//    v = 10*v;                        // v = 1,0
//    y = (int)v;
//    ina219[i]= 0x30 + y;
}
//---- Funcao para converter float em string --------------
void floatParaString(float v, dataType varName)
{
    volatile unsigned int x = 0;
    volatile char y = 0;
    volatile unsigned int i = 0;
    volatile unsigned int p = 0;
    volatile char ina219 [5];
    // Separando milhar
    p = v/1000;                      // Separar centena, e dessa forma escrever o nÃºmero referente na tela
    y = (int)p;                      // separa o inteiro da parte fracionaria
    // Ex: x = #4095,731 --> f = 4095,731/1000 = 4.095731 --> z = (int)f = #4
    if( varName == ina2190x40)
    {
        if(y == 0)                   // Se a casa do milhar nao existe no valor, nao imprima
        {
            x++;                     // x = 1
        }
        else
            ina219[i++]= 0x30 + y;
    }
    // Separando centena
    v = v - 1000*y;                  // v = 4095,731 - 1000*4 = 95,731
    p = v/100;                       // f = 95,731/100 = 0.95731
    y = (int)p;                      // separa o inteiro da parte fracionaria
    // z = 0
    if( varName == ina2190x40)
    {
        if(x == 1)                   // Se o contador aponta que a casa do milhar nao existe
        {
            if(y == 0)               // Se a casa da centena existe, imprima
                x++;                 // x = 2
        }
        else                         // Se a casa do milhar ja existe, continue imprimindo normalmente
        {
            ina219[i++]= 0x30 + y;
        }
    }
    // Separando dezena
    v = v - 100*y;                   // v = 095,731 - 100*0 = 95,731
    p = v/10;                        // f = 9.5731
    y = (int)p;                      // separa o inteiro da parte fracionaria
    // z = 9
    if( varName == ina2190x40)
    {
        if(x == 2)                   // Se o contador aponta que a casa do milhar e da centena nao existem
        {
            if(y == 0)               // Se a da dezena existe, imprima
                x++;                 // x = 3
        }
        else                         // Se a casa do milhar ou da centena ja existe, continue imprimindo normalmente
        {
            ina219[i++]= 0x30 + y;
        }
    }
    // Separando unidade
    v = v - 10*y;                    // v = 95,731 - 10*9 = 5,731
    y = (int)v;
    if( varName == ina2190x40)
    {
        ina219[i++]= 0x30 + y;
    }
    // Separando primeira casa decimal
    v = v - y;                       // v = 5,731 - 5 = 0,731
    v = 10*v;                        // v = 7,31
    y = (int)v;
    if( varName == ina2190x40)
    {
        ina219[i++]= ',';
        ina219[i++]= 0x30 + y;
    }
    // Separando segunda casa decimal
    v = v - y;                       // v = 7,31 - 7 = 0,31
    v = 10*v;                        // v = 3,1
    y = (int)v;
    if( varName == ina2190x40)
    {
        ina219[i++]= 0x30 + y;
    }
    // Separando terceira casa decimal
    v = v - y;                       // v = 3,1 - 3 = 0,1
    v = 10*v;                        // v = 1,0
    y = (int)v;
    if( varName == ina2190x40)
    {
        ina219[i]= 0x30 + y;
    }
}
//---- Funcao para converter inteiros em string --------------
void inteiroParaString(uint16_t v, dataType varName)
{
    volatile char y = 0;
    volatile unsigned int i = 0;
    volatile unsigned int p = 0;
    //|   Ambar    |    HORA    | Contador |  Tanque 1 (250L) | pH   |   OxigÃªnio     |  Condutividade |  Total de     |Temperatura  |
    //|  Tanque 1  |            |          |    (Events)      |      | Dissolvido(DO) |   Elehtrica(EC) | SÃ³lidos (TDS) |   (ÂºC)      |
    //| xx/xx/xxxx |  HH:MM:SS  |   NNNN   |    XXX Litros    | X,X  |    X mg/L      |    XXXX uS/Cm  |   XXX ppm     |   XX,X ÂºC   |
    //typedef enum { data, tanque1L, phT1, doT1, ecT1, tdsT1} dataType;
    if( varName == data)
    {
        if( varName == ano){
            // Separando milhar
            p = v/1000;                         // Separar centena, e dessa forma escrever o nÃºmero referente na tela
            y = (int)p;                         // separa o inteiro da parte fracionaria
            // Ex: x = #4095 --> f = 4095/1000 = 4.095 --> z = (int)f = #4
            stringDateYr[i++] = y + 0x30;
            // Separando centena
            v = v - 1000*y;                     // v = 4095 - 1000*4 = 095
            p = v/100;                          // p = 095/100 = 0.95
            y = (int)p;                         // separa o inteiro da parte fracionaria --> y = 0
            stringDateYr[i++] = y + 0x30;
            // Separando dezena
            v = v - 100*y;                      // v = 095 - 100*0 = 95
            p = v/10;                           // p = 9.5
            y = (int)p;                         // separa o inteiro da parte fracionaria --> y = 9
            stringDateYr[i++] = y + 0x30;
            // Separando unidade
            v = v - 10*y;                       // v = 95 - 10*9 = 5
            stringDateYr[i++] = v + 0x30;
        }
        if( varName == mes){
            // Separando dezena
            p = v/10;                           // p = 9.5
            y = (int)p;                         // separa o inteiro da parte fracionaria --> y = 9
            stringDateMnt[i++] = y + 0x30;
            // Separando unidade
            v = v - 10*y;                       // v = 95 - 10*9 = 5
            stringDateMnt[i++] = v + 0x30;
        }
        if( varName == dia){
            // Separando dezena
            p = v/10;                           // p = 9.5
            y = (int)p;                         // separa o inteiro da parte fracionaria --> y = 9
            stringDateDay[i++] = y + 0x30;
            // Separando unidade
            v = v - 10*y;                       // v = 95 - 10*9 = 5
            stringDateDay[i++] = v + 0x30;
        }
    }
    else if( varName == tanque1L)                   //
    {
        // Separando centena
        p = v/100;                          // p = 095/100 = 0.95
        y = (int)p;                         // separa o inteiro da parte fracionaria --> y = 0
        stringT1L[i++] = y + 0x30;
        // Separando dezena
        v = v - 100*y;                      // v = 095 - 100*0 = 95
        p = v/10;                           // p = 9.5
        y = (int)p;                         // separa o inteiro da parte fracionaria --> y = 9
        stringT1L[i++] = y + 0x30;
        // Separando unidade
        v = v - 10*y;                       // v = 95 - 10*9 = 5
        stringT1L[i++] = v + 0x30;
        stringT1L[i++] = 'A';

    }
    else if( varName == tempint)                   //
    {
        // Separando centena
        p = v/100;                          // p = 095/100 = 0.95
        y = (int)p;                         // separa o inteiro da parte fracionaria --> y = 0
        stringT1temp[i++] = y + 0x30;
        // Separando dezena
        v = v - 100*y;                      // v = 095 - 100*0 = 95
        p = v/10;                           // p = 9.5
        y = (int)p;                         // separa o inteiro da parte fracionaria --> y = 9
        stringT1temp[i++] = y + 0x30;
        // Separando unidade
        v = v - 10*y;                       // v = 95 - 10*9 = 5
        stringT1temp[i++] = v + 0x30;
        stringT1temp[i++] = 'T';

    }
    else if( varName == tanque2L)                   //
    {
        // Separando centena
        p = v/100;                          // p = 095/100 = 0.95
        y = (int)p;                         // separa o inteiro da parte fracionaria --> y = 0
        stringT2L[i++] = y + 0x30;
        // Separando dezena
        v = v - 100*y;                      // v = 095 - 100*0 = 95
        p = v/10;                           // p = 9.5
        y = (int)p;                         // separa o inteiro da parte fracionaria --> y = 9
        stringT2L[i++] = y + 0x30;
        // Separando unidade
        v = v - 10*y;                       // v = 95 - 10*9 = 5
        stringT2L[i++] = v + 0x30;
        stringT2L[i++] = 'B';

    }
    else if( varName == phT1)                   // Se eh para colocar virgula apos o primeiro numero
    {
        // Separando dezena
        p = v/10;                           // p = 9.5
        y = (int)p;                         // separa o inteiro da parte fracionaria --> y = 9
        stringT1L[i++] = y + 0x30;
        stringT1pH[i++]= 0x30 + y;
        stringT1pH[i++]= ',';
        // Separando unidade
        v = v - 10*y;                       // v = 95 - 10*9 = 5
        stringT1L[i++] = v + 0x30;
    }
    else if( varName == doT1)
    {
        // Separando dezena
        p = v/10;                           // p = 9.5
        y = (int)p;                         // separa o inteiro da parte fracionaria --> y = 9
        stringT1L[i++] = y + 0x30;
        stringT1DO[i++]= 0x30 + y;
        stringT1DO[i++]= ',';
        // Separando unidade
        v = v - 10*y;                       // v = 95 - 10*9 = 5
        stringT1DO[i++] = v + 0x30;
    }
    else if( varName == ecT1)
    {
        // Separando milhar
        p = v/1000;                         // Separar centena, e dessa forma escrever o nÃºmero referente na tela
        y = (int)p;                         // separa o inteiro da parte fracionaria
        // Ex: x = #4095 --> f = 4095/1000 = 4.095 --> z = (int)f = #4
        stringT1EC[i++] = y + 0x30;
        stringT1EC[i++] = ',';
        // Separando centena
        v = v - 1000*y;                     // v = 4095 - 1000*4 = 095
        p = v/100;                          // p = 095/100 = 0.95
        y = (int)p;                         // separa o inteiro da parte fracionaria --> y = 0
        stringT1EC[i++] = y + 0x30;
        // Separando dezena
        v = v - 100*y;                      // v = 095 - 100*0 = 95
        p = v/10;                           // p = 9.5
        y = (int)p;                         // separa o inteiro da parte fracionaria --> y = 9
        stringT1EC[i++] = y + 0x30;
        // Separando unidade
        v = v - 10*y;                       // v = 95 - 10*9 = 5
        stringT1EC[i++] = v + 0x30;
    }
    else if( varName ==  tdsT1)
    {
        // Separando centena
        v = v - 1000*y;                     // v = 4095 - 1000*4 = 095
        p = v/100;                          // p = 095/100 = 0.95
        y = (int)p;                         // separa o inteiro da parte fracionaria --> y = 0
        stringT1TDS[i++] = y + 0x30;
        // Separando dezena
        v = v - 100*y;                      // v = 095 - 100*0 = 95
        p = v/10;                           // p = 9.5
        y = (int)p;                         // separa o inteiro da parte fracionaria --> y = 9
        stringT1TDS[i++] = y + 0x30;
        // Separando unidade
        v = v - 10*y;                       // v = 95 - 10*9 = 5
        stringT1TDS[i++] = v + 0x30;
    }
}
void uartWrite( volatile char * str )
{
    // Como eh uma interface dedicada, nÃ£o serÃ¡ preciso fazer "bitbanging"
    while( *str)
    {
        while(!(UCA1IFG & UCTXIFG));        // Aguarda o buffer de transmissao ficar vazio
        UCA1TXBUF = *str++;
    }
}
void uartWrite_UCA0( volatile char * str )
{
    // Como eh uma interface dedicada, nÃ£o serÃ¡ preciso fazer "bitbanging"
    while( *str)
    {
        while(!(UCA0IFG & UCTXIFG));        // Aguarda o buffer de transmissao ficar vazio
        UCA0TXBUF = *str++;
    }
}
void uartMonitorAmbar( void )
{
    // ------------------------------------------------------------------------------------------------------------------------------
    //|   Ambar    |    HORA    |  Estado  |  Tanque 1 (250L) | pH   |   OxigÃªnio     |  Condutividade |  Total de     |Temperatura  |
    //|  Tanque 1  |            |          |    (Events)      |      | Dissolvido(DO) |   Elehtrica(EC) | SÃ³lidos (TDS) |   (ÂºC)      |
    //| xx/xx/xxxx |  HH:MM:SS  |   NNNN   |    XXX Litros    | X,X  |    X mg/L      |    XXXX uS/Cm  |   XXX ppm     |   XX,X ÂºC   |
    // ------------------------------------------------------------------------------------------------------------------------------
    // https://hydroponicway.com/how-to-measure-ec-ph-do-temperature-in-hydroponics <-- entrar nesse link para ver sugestÃµes de sensores
    //...
    // \a - Alarm, Alarme = Toca o alarme sonoro do sistema
    // \b - Back space, Retrocesso = Apaga o caractere Ã  esquerda do cursor
    // \n - NewLine, Nova linha = Pula uma linha
    // \t - TabulaÃ§Ã£o horizontal = Equivale Ã  dar um TAB na string
    // \r - Carriage Return, Retorno do Carro = Volta para o inÃ­cio da linha.
    // \0 - Null, Nulo = Caracter nulo ou zero geralmente estabelecido como fim de string
    // \v â€“ TabulaÃ§Ã£o Vertical
    // \\ â€“ Caracter \
    // \â€™ â€“ Caracter â€˜
    // \â€� â€“ Caracter â€œ
    // \? â€“ Caracter ?
    // %% â€“ Caracter %
    uartWrite("\n");              // Pular para linha de baixo no terminal
    uartWrite("\r");              // Retorno do carro no terminal
    uartWrite("|********************************************* AMBAR - SISTEMA DE FERTIRRIGAÃ‡ÃƒO ************************************************|\n\r");
    uartWrite("|------------|----HORA----|--Estado--|--Tanque 1 (250L)-|--pH--|----OxigÃªnio----|--Condutividade--|---Total de ---|-Temperatura-|\n\r");
    uartWrite("|  Tanque 1  |            |          |    (Events)      |      | Dissolvido(DO) |   Elehtrica(EC)  | SÃ³lidos (TDS) |   (ÂºC)      |\n\r");
    uartWrite("| xx/xx/xxxx |  HH:MM:SS  |");
    switch(lcd_T1Tool){
    case 1: uartWrite(" Enchendo "); break;
    case 2: uartWrite("Evaporando"); break;
    case 3: uartWrite("  Pronto  "); break;
    case 4: uartWrite(" Regando  "); break;
    case 5: uartWrite("  Vazio   "); break;
    case 6: uartWrite("   ERRO   "); break;
    default: uartWrite(" invÃ¡lido ");
    }
    uartWrite("|    XXX Litros    | X,X  |     x mg/L     |    XXXX uS/Cm  |    XXX ppm     |   XX,X ÂºC   |\n\r");
    // ----------------------------------------
    uartWrite("\n");              // Pular para linha de baixo no terminal
    uartWrite("\r");              // Retorno do carro no terminal
    uartWrite("|---Ambar----|----HORA----|--Estado--|--Tanque 2 (250L)-|--pH--|----OxigÃªnio----|--Condutividade--|---Total de ---|-Temperatura-|\n\r");
    uartWrite("|  Tanque 2  |            |          |    (Events)      |      | Dissolvido(DO) |   Elehtrica(EC)  | SÃ³lidos (TDS) |   (ÂºC)      |\n\r");
    uartWrite("| xx/xx/xxxx |  HH:MM:SS  |");
    switch(lcd_T2Tool){
    case 1: uartWrite(" Enchendo "); break;
    case 2: uartWrite("Evaporando"); break;
    case 3: uartWrite("  Pronto  "); break;
    case 4: uartWrite(" Regando  "); break;
    case 5: uartWrite("  Vazio   "); break;
    case 6: uartWrite("   ERRO   "); break;
    default : uartWrite(" invÃ¡lido ");
    // uartWrite("\n\r\n\r");              // Pular para linha de baixo no terminal, retorno do carro no terminal
    }
    uartWrite("|    XXX Litros    | X,X  |    x mg/L      |    XXXX uS/Cm  |    XXX ppm     |   XX,X ÂºC   |");
    uartWrite("\n\r");
    uartWrite("|*******************************************************************************************************************************|\n\r");
}
void uartWriteFails( volatile char * str, uint8_t strlen )
{
    // Como eh uma interface dedicada, nÃ£o serÃ¡ preciso fazer "bitbanging"
    while( strlen > 0)
    {
        while(!(UCA1IFG & UCTXIFG));        // Aguarda o buffer de transmissao ficar vazio
        UCA1TXBUF = *str++;
        strlen--;
    }
}
void uartWriteFails_UCA0( volatile char * str, uint8_t strlen )
{
    // Como eh uma interface dedicada, nao sera preciso fazer "bitbanging"
    while( strlen > 0)
    {
        while(!(UCA0IFG & UCTXIFG));        // Aguarda o buffer de transmissao ficar vazio
        UCA0TXBUF = *str++;
        strlen--;
    }
}
void uartConfig(uint8_t interface)
{
    if(interface == 1)
    {
        rx.i = 0;
        rx.refresh = 0;
        rx.read  = 0;
        rx.write = 0;
        rx.size  = 0;                         // Zerar o indexador do buffer circular

        UCA1CTL1 = UCSWRST;                 // Reseta a inferface para permitir a configuraÃ§Ã£o
        UCA1CTL0 =
                //                   UCPEN    |               // Habilita paridade
                //                   UCPAR    |               // PAR = 0 eh impar, 1 eh par
                //                   UCMSB    |               // Manda o LSB primeiro (MSB = 0)
                //                   UC7BIT   |               // Manda pacotes de 8 bites (1 byte)
                //                   UCSPB    |               // 0 = um stop, 1 = dois stops
                UCMODE_0 |               // Modo UART
                //                   UCSYNC   |                // AssÃ­ncrono
                0;
        UCA1CTL1 |=  UCSSEL__SMCLK;       // Note que "|=" faz o "bit set", dessa forma o UCSWRST ainda permanece em 1

        // Clock de entrada bate a 2^20 Hz
        // Baudrate = 9600 bits por segundo
        // D = 2^20 / 9600 = 109,227 -> parte inteira = 109, parte fracionaria = 227
        // BRW = 109, BRS = 0x23* 8 = 1,81 => 2
        // Problema: A janela de amostragem eh mto pequena
        // --> Modo de oversampling (OS16 = 1)
        // D = 2^20 / 9600*16(divide por 16 porque eh modo oversampling, sempre manter uma proporcao por 16) = 6,83
        // BRW = 6, BRF = 0,83*16 = 13,2 => 13
        UCA1BRW  = 6;
        UCA1MCTL = UCBRF_13 | UCOS16;       // UCA1MCTL = registro de controle da modulacao
        // Essa foi a configuracao que da uma boa janela de voto de maioria, porque me garante uma proporcao de 1/16 entre o clock de
        // entrada e o clock do bit
        // Fazer a mesma configuracao se fosse o ACLK @32768
        // D = 32768 / 9600 = 3,41 --> BRW = 3, BRS = 0,41*8 = 3,3 ~ 3 . O "BRS" a gente sempre arredonda
        // Sem Oversampling fica BRW = 3 e o BRS = 0,413 * 8 = 3,28 => 3
        P4SEL |= BIT4 | BIT5;               // P4.5 = UCA1RXD , P4.4 = UCA1TXD
        UCA1CTL1 &= ~UCSWRST;
        UCA1IE = UCRXIE;                    // Habilitar interrupcoes, precisa ser depois do reset, pois o mesmo zera
        // o registro
    }
    if( interface == 0)                     // Configurado para se comunicar com o ESP32
    {
        UCA0CTL1 = UCSWRST;                 // Reseta a inferface para permitir a configuraÃ§Ã£o
        UCA0CTL0 =
                //                   UCPEN    |               // Habilita paridade
                //                   UCPAR    |               // PAR = 0 eh impar, 1 eh par
                //                   UCMSB    |               // Manda o LSB primeiro (MSB = 0)
                //                   UC7BIT   |               // Manda pacotes de 8 bites (1 byte)
                //                   UCSPB    |               // 0 = um stop, 1 = dois stops
                UCMODE_0 |               // Modo UART
                //                   UCSYNC   |                // AssÃ­ncrono
                0;
        UCA0CTL1 |=  UCSSEL__SMCLK;       // Note que "|=" faz o "bit set", dessa forma o UCSWRST ainda permanece em 1

        // Clock de entrada bate a 2^20 Hz
        // Baudrate = 9600 bits por segundo
        // D = 2^20 / 9600 = 109,227 -> parte inteira = 109, parte fracionaria = 227
        // BRW = 109, BRS = 0x23* 8 = 1,81 => 2
        // Problema: A janela de amostragem eh mto pequena
        // --> Modo de oversampling (OS16 = 1)
        // D = 2^20 / 9600*16(divide por 16 porque eh modo oversampling, sempre manter uma proporcao por 16) = 6,83
        // BRW = 6, BRF = 0,83*16 = 13,2 => 13
        UCA0BRW  = 6;
        UCA0MCTL = UCBRF_13 | UCOS16;       // UCA0MCTL = registro de controle da modulacao
        // Essa foi a configuracao que da uma boa janela de voto de maioria, porque me garante uma proporcao de 1/16 entre o clock de
        // entrada e o clock do bit
        // Fazer a mesma configuracao se fosse o ACLK @32768
        // D = 32768 / 9600 = 3,41 --> BRW = 3, BRS = 0,41*8 = 3,3 ~ 3 . O "BRS" a gente sempre arredonda
        // Sem Oversampling fica BRW = 3 e o BRS = 0,413 * 8 = 3,28 => 3
        P3SEL |= BIT4 | BIT3;               // P4.5 = UCA0RXD , P4.4 = UCA1TXD
        UCA0CTL1 &= ~UCSWRST;
        UCA0IE = UCRXIE;                    // Habilitar interrupcoes, precisa ser depois do reset, pois o mesmo zera
    }
}

// Interrupcao da USCI_A0
#pragma vector = USCI_A0_VECTOR
__interrupt void usci_a0_int(void)
{
    // Ler o buffer apenas para zera a flag. Testar usar "UCA0IV; // Apagar RXIFG"
    fromESP32[0] = UCA0RXBUF;
    flagOpCmd_UCA0 = 1;
//    UCA1IV;                                 // Apagar RXIFG
//    UCA1TXBUF = UCA1RXBUF;
}
