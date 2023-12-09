/*
 * timer.c
 *
 *  Created on: 5 de jul de 2021
 *      Author: guiwz
 */
#include <msp430.h>
#include <stdint.h>
#include "libs/config_gpio_5529.h"
#include "libs/config_timers_5529.h"
#include "libs/ambar_timers_5529.h"
#include "libs/ambar_lcd_5529.h"
#include "libs/config_lcd_5529.h"
// Tempo de evaporacao do cloro dos tanques
#define EVAPSEG 0
#define EVAPMIN 0
#define EVAPHORA 5

#define     INT_PCF0x20     0X0004         // P1.2 - Referenciando 0x0004 como "nivelB_1", equivalente ao PIN2 para a porta P1, ou 0b00000100 (76543210)
#define     botaoCap1       0x0080         // P2.7 - Referenciando 0x0080 como "botaoTouch1", equivalente ao PIN7 para a porta P2, ou 0b10000000 (76543210)
#define     PCF8574_LCD1        0x27
#define     PCF8574_LCD2        0x26
// TA0 em ACLK. Utilizado para tempo de evapora��o, rega, e tratamento de rebotes. TA0 gera interrup��es a cada 1 segundo
// TA1 utilizado nas fun��es "wait" e também como timer do botão capacitivo
// TA2 nos su's
// TB0 Sendo utilizado para os TRIGGERs e ECHOs dos SUs
// TB1
//----------------------------------------------------------
//----------------- Configura��o do Real Time Clock---------------------------------
//-- Ir� fornecer medi��o para o calend�rio de aduba��es, coleta de dados, etc -----
//void rtc_config()
//{
//
//}
//----------------------------------------------------------------------------------

uint16_t  count;
//Var�veis Gobais
extern volatile unsigned int flagSnivel;
extern volatile unsigned int flag2Snivel;
extern volatile unsigned int flagbotaoMenu;
extern volatile unsigned int evap_T1, evap_T2;                        //Vari�vel "evap_T1" e "evap_T2", permite a entrada na fun��o timerevap_T1(2)
extern volatile unsigned int regT1ok, regT2ok;              // Indicacoes para tanques prontos para regar
extern volatile unsigned int flag_regT1ok, flag_regT2ok;    // Indicacoes para tanques prontos para regar
extern volatile unsigned int t1_closeRegIN, t2_closeRegIN, t1_closeRegOUT, t2_closeRegOUT;             // Solicita��o de fechamento e abertura de v�lvulas
extern volatile unsigned int t1_openRegIN, t2_openRegIN, t1_openRegOUT, t2_openRegOUT;             // Solicita��o de fechamento e abertura de v�lvulas
extern volatile unsigned int t1_regar, t2_regar;            // Solicita��o de rega
extern volatile unsigned int t1_encher, t2_encher;          // Solicita��o para encher os tanques
extern volatile unsigned int lcd_T1Tool,  lcd_T2Tool;
extern volatile unsigned int flag_SU;
extern volatile unsigned int flag_LCD;
extern volatile unsigned int milisec_bCap;
extern volatile unsigned int sec_bCap;
extern volatile unsigned int sec_bCap10;
extern volatile unsigned int tseg_valve;
//extern int flagNivelA_1, flagNivelB_1, flagNivelA_2, flagNivelB_2;           // Utilizados nas fun��es de rebotes
extern volatile unsigned int flag_1_sec;
extern volatile int flag_Operador_T1_T2;
unsigned volatile int rotina_Inicial;
extern volatile int menu_entraEsai;
extern volatile int opcoesMenu;
extern volatile uint8_t mem_byte_interrupt;
extern volatile uint8_t flagP1IES;
extern volatile uint8_t t1Past;
extern volatile uint8_t t1Now;
extern volatile uint8_t t2Past;
extern volatile uint8_t t2Now;


// Variaveis vindas de uartAmbar5529
extern volatile uint8_t flag_softDog;
extern volatile uint8_t countExit;

//                  Interrupcoes do TIMER A0                    //
//----------------- Acontece a cada 1 segundo ------------------//
#pragma vector = TIMER0_A0_VECTOR                     //pragma vector = 53
__interrupt void TA0_CCR0_ISR()
{

    flag_1_sec = 1;
    count-- ;
    if(flag_softDog)
    {
        countExit--;
    }
    //flag_SU = 1;
}

//  Funcao para tratar as instabilidades
uint8_t sNivel(void)
{
    //        volatile uint8_t mem_byte_interrupt = 0;
    // Sensores de nivel conectados as portas P0(A1), P1(B1), P2(A2) e P3(B2) da PCF8574

//    if(flagSnivel == 1)
//    {
//        mem_byte_interrupt = readPCF8574(ADDR_SNIVEL); //  "mem_byte_interrupt" vai guardar o valor da primeira leitura dos sensores
//        t2Now  = (mem_byte_interrupt >> 2);
//        t2Now &= 0x03;         // Exemplo (0b11111011) &= 0x03 = 0b00001011 &= 0x03 = 0b00000011
//        t1Now  = mem_byte_interrupt;
//        t1Now &= 0x03;         // Exemplo (0b11111011 >> 4) &= 0x03 = 0b00000010 &= 0x03 = 0b00000010
//        flagSnivel = 0;
//    }
    volatile static int t1t2_Reb = 5;              // Even if the static variables are shared variables, but in different thread there can be different values
    // for a static variable in the local cache of a thread. To make it consistent for all threads, just declare it as
    // static volatile. So each time it will fetch from main memory.
    if(--t1t2_Reb <= 0)                          //
    {
        t1t2_Reb = 7;                            //Retorna o contador para zero
        t1Past = t1Now;
        t2Past = t2Now;
        //  B2-A2-B1-A1
        //0b x  x  x  x <- LSB
        mem_byte_interrupt = readPCF8574(ADDR_SNIVEL);      // Ao final do tempo contado, compara se a primeira leitura continua a mesma
        t2Now  = (mem_byte_interrupt >> 2);
        t2Now &= 0b0011;         // Exemplo (0b11111011) &= 0x03 = 0b00001011 &= 0x03 = 0b00000011
        t1Now  = mem_byte_interrupt;
        t1Now &= 0b0011;         // Exemplo (0b11111011 >> 4) &= 0x03 = 0b00000010 &= 0x03 = 0b00000010

        if(t1Now==0b0001)         // - Erro no T1 - Se sensor de nivel de cima indica que ha agua e o de baixo nao
        {
            lcd_T1Tool = 6;
            t1_closeRegIN = 1;  // Fechar entrada de agua, por seguranca
        }
        if(t2Now==0b0001)         // - Erro no T2
        {
            lcd_T2Tool = 6;
            t2_closeRegIN = 1;  // Fechar entrada de agua, por seguranca
        }
        // ----------------------------------------------------
        //           AB             AB
        if(t1Past==0b0011 && t1Now==0b0010)         // - T1 regando
        {
            t1Past = t1Now;
        }//          AB             AB
        if(t2Past==0b0011 && t2Now==0b0010)         // - T2 regando
        {
            t2Past = t2Now;
        }
        // -----------------------------------------------------
        //           AB             AB
        if(t1Past==0b0010 && t1Now==0b0000)         // - T1 ESVAZIOU
        {
            lcd_T1Tool = 5;
            t1_regar = 0; t2_regar = 1; t1_closeRegOUT = 1; t1_openRegIN = 1; flag_Operador_T1_T2 = 1;
            t1Past = t1Now;
        }//            AB             AB
        if(t2Past==0b0010 && t2Now==0b0000)         // - T2 ESVAZIOU
        {
            lcd_T2Tool = 5;
            t2_regar = 0; t1_regar = 1; t2_closeRegOUT = 1; t2_openRegIN = 1; flag_Operador_T1_T2 = 1;
            t2Past = t2Now;
        }
        //           AB             AB
        if(t1Past==0b0011 && t1Now==0b0000)         // - T1 ESVAZIOU
        {
            lcd_T1Tool = 5;
            t1_regar = 0; t2_regar = 1; t1_closeRegOUT = 1; t1_openRegIN = 1; flag_Operador_T1_T2 = 1;
            t1Past = t1Now;
        }//            AB             AB
        if(t2Past==0b0011 && t2Now==0b0000)         // - T2 ESVAZIOU
        {
            lcd_T2Tool = 5;
            t2_regar = 0; t1_regar = 1; t2_closeRegOUT = 1; t2_openRegIN = 1; flag_Operador_T1_T2 = 1;
            t2Past = t2Now;
        }
        // -----------------------------------------------------
        if(t1Past==0b0000 && t1Now==0b0010)         // - T1 enchendo
        {
            lcd_T1Tool = 1;
            t1Past = t1Now;
        }
        if(t2Past==0b0000 && t2Now==0b0010)         // - T2 enchendo
        {
            lcd_T2Tool = 1;
            t2Past = t2Now;
        }
        // -----------------------------------------------------
        if(t1Past==0b0010 && t1Now==0b0011)         // - T1 ENCHEU
        {
            lcd_T1Tool = 2;
            t1_regar = 0; t1_closeRegIN = 1; regT1ok = 0; evap_T1 = 1; flag_Operador_T1_T2 = 1;
            t1Past = t1Now;
        }
        if(t2Past==0b0010 && t2Now==0b0011)         // - T2 ENCHEU
        {
            lcd_T2Tool = 2;
            t2_regar = 0; t2_closeRegIN = 1; regT2ok = 0; evap_T2 = 1; flag_Operador_T1_T2 = 1;
            t2Past = t2Now;
        }
        if(t1Past==0b0000 && t1Now==0b0011)         // - T1 ENCHEU
        {
            lcd_T1Tool = 2;
            t1_regar = 0; t1_closeRegIN = 1; regT1ok = 0; evap_T1 = 1; flag_Operador_T1_T2 = 1;
            t1Past = t1Now;
        }
        if(t2Past==0b0000 && t2Now==0b0011)         // - T2 ENCHEU
        {
            lcd_T2Tool = 2;
            t2_regar = 0; t2_closeRegIN = 1; regT2ok = 0; evap_T2 = 1; flag_Operador_T1_T2 = 1;
            t2Past = t2Now;
        }
        // -----------------------------------------------------
//        if(t1Past==0x03 && t1Now==0x01)         // - T1 ENCHEU
//        {
//            lcd_T1Tool = 2;
//            t1_regar = 0; t1_closeRegIN = 1; regT1ok = 0; evap_T1 = 1; flag_Operador_T1_T2 = 1;
//            t1Past = t1Now;
//        }
//        if(t2Past==0x03 && t2Now==0x01)         // - T2 ENCHEU
//        {
//            lcd_T2Tool = 2;
//            t2_regar = 0; t2_closeRegIN = 1; regT2ok = 0; evap_T2 = 1; flag_Operador_T1_T2 = 1;
//            t2Past = t2Now;
//        }
        // -----------------------------------------------------
        P1IFG &= ~BIT2;             // Zera a flag do pino P1.2 (INT_PCF0x20)
        P1IE  |=  BIT2;             // Reabilita a interrupcao do pino P1.2
        P1IFG &= ~BIT3;             // Zera a flag do pino P1.2 (INT_PCF0x20)
        P1IE  |=  BIT3;             // Reabilita a interrupcao do pino P1.2
        flagSnivel = 1;
    }
    return 0;
} // uint8_t sNivel(void)

void evapTimer_T1()                         //Contador regressivo do tempo de evaporacaoo do tanque 1
{
    static volatile int t1seg  = 0;         // Even if the static variables are shared variables, but in different thread there can be different values
    static volatile int t1min  = 0;         // for a static variable in the local cache of a thread. To make it consistent for all threads, just declare it as
    static volatile int t1hora = EVAPHORA;         // static volatile. So each time it will fetch from main memory.

    if(evap_T1 == 1)                        // Se a flag "evap_T1" est� levantada, entra na contagem para atualiza��o do estado
    {                                       // do LCD em lcd_T1Tool = 2
        lcd_T1Tool = 2;                     // Vai indicar no LCD que evap 1 come�ou
        // P3OUT |=  BIT7;                  // LED amarelo 1 informa inicio de contagem de tempo de evaporacao
        //----------------------
        lcdCursor(PCF8574_LCD1,0x48);                // A cada minuto o timer de evaporacao no LCD eh atualizado,
        lcdNumbers_Hour(PCF8574_LCD1,t1hora);        // foi colocada aqui dentro a fim de atualizar somente uma vez por minuto
        lcdCursor(PCF8574_LCD1,0x4A);                // Atualiza a contagem mostrada no LCD
        lcdPrint(PCF8574_LCD1,"h");                  // Atualiza a contagem mostrada no LCD
        lcdNumbers_Minute(PCF8574_LCD1,t1min);       //
        //---------------------
        t1seg--;
        if(t1seg <= 0)                      //
        {
            t1seg = EVAPSEG;                // Volta t1seg para 59
            if(t1min-- <= 0)                //
            {
                t1min = EVAPMIN;
                if(t1hora-- <= 0)
                {
                    t1hora  = EVAPHORA;
                    evap_T1 = 0;
                    regT1ok = 1;
                    flag_regT1ok = 1;
                    if(t2_regar == 0 )
                    {
                        t1_regar = 1;
                    }
                    if(rotina_Inicial == 1)         // Se o sistema iniciou com os dois tanques vazios
                    {
                        if(regT2ok == 0)
                        {
                            t2_openRegIN = 1;
                            rotina_Inicial = 0;
                            t1_regar = 1;
                        }
                    }
                    flag_Operador_T1_T2 = 1;        // Chama o operador das v�lvulas
                    flag_LCD = 1;                   // Chama o operador do LCD
                    lcd_T1Tool = 3;                 // Vai indicar no LCD que evap 1 ok

                }}}
    }
}
void evapTimer_T2()                         //Contador da evaporacao do tanque 2
{
    static volatile int t2seg  = 0;         // Even if the static variables are shared variables, but in different thread there can be different values
    static volatile int t2min  = 0;         // for a static variable in the local cache of a thread. To make it consistent for all threads, just declare it as
    static volatile int t2hora = EVAPHORA;         // static volatile. So each time it will fetch from main memory.
    if(evap_T2 == 1)                        // Se a flag "evap_T2" esta levantada, entra na contagem para atualizacao do estado
    {                                       // do LCD em lcd_T2Tool = 2
        lcd_T2Tool = 2;                     // Vai indicar no LCD que evap 2 come�ou
        //----------------
        lcdCursor(PCF8574_LCD1,0x4f);                //
        lcdNumbers_Hour(PCF8574_LCD1,t2hora);
        lcdCursor(PCF8574_LCD1,0x51);
        lcdPrint(PCF8574_LCD1,"h");
        lcdNumbers_Minute(PCF8574_LCD1,t2min);
        //----------------
        t2seg--;
        if(t2seg <= 0)                      // Se ja contou 60 segundos
        {
            t2seg = EVAPSEG;                // Volta t2seg para 59
            if(t2min--    <= 0)                //
            {
                t2min = EVAPMIN;
                if(t2hora-- <= 0)
                {
                    t2hora  = EVAPHORA;
                    evap_T2 = 0;
                    regT2ok = 1;
                    flag_regT2ok = 1;
                    if(t1_regar == 0 )
                    {
                        t2_regar = 1;
                    }
                    t1_encher = 1;            // Pode encher o tanque 1
                    flag_Operador_T1_T2 = 1;  // Chama o operador das v�lvulas
                    flag_LCD = 1;             // Chama o operador do LCD
                    lcd_T2Tool = 3;           // Vai indicar no LCD que evap 2 ok
                }}}
    }
}





