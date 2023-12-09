

#include <msp430.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "libs/ambar_ultrassom_5529.h"
#include "libs/config_lcd_5529.h"
#include "libs/ambar_lcd_5529.h"
#include "libs/config_timers_5529.h"

#define PCF8574_LCD1        0x27
#define PCF8574_LCD2        0x26
//GND
//ECHO    T1:   P2.4
//TRIGGER T1:   P3.6
//ECHO    T2:   P2.5
//TRIGGER T2:   P3.5
//VCC
//--------------- SENSOR DE ULTRASSOM -------------------
volatile unsigned int contadorEchoT1[2];
volatile unsigned int contadorEchoT2[2];
volatile unsigned int kT1=0;
volatile unsigned int kT2=0;
volatile unsigned int iT1=0;
volatile unsigned int iT2=0;
volatile unsigned int diffVec_T1[6];
volatile unsigned int diffVec_T2[6];
volatile unsigned int diff_T1 = 0;
volatile unsigned int diff_T2 = 0;
volatile unsigned int s1_Ultra;
volatile unsigned int s2_Ultra;
volatile float volumeT1;
volatile float volumeT2;
volatile unsigned int timerSU_1;
volatile unsigned int timerSU_1_OK;
volatile unsigned int timerSU_2;
volatile unsigned int timerSU_2_OK;
volatile unsigned int flag_SU;
volatile unsigned int medianaMedicoesSU_T1;
volatile unsigned int ordenacaoMedicoesSU_T1;
volatile unsigned int medianaMedicoesSU_T2;
volatile unsigned int ordenacaoMedicoesSU_T2;
//------------------------------------------
volatile unsigned int t1seg_SU;
volatile unsigned int t2seg_SU;
volatile unsigned int t1min_SU;
volatile unsigned int t2min_SU;
//------------------------------------------
//t1seg_SU = 0;
//t1min_SU = 0;                // Variï¿½veis para o sensor de ultrassom do tanque 1
//t2seg_SU = 0;
//t2min_SU = 0;                // Variï¿½veis para o sensor de ultrassom do tanque 2
//timerSU_1 = 0;
//timerSU_2 = 0;



// Interrupção pelo TIMER B0.
//#pragma vector = TIMERB0_VECTOR
//__interrupt void SU_TANQUE_1e2_ISR()
//{
//    switch (TB0IV)
//    {
//    case 0x02:                              //
//        break;
//    case 0x04:                              //
//        break;
//    case 0x06:
//        break;
//    case 0x08:      //
//        break;
//    case 0x0A:      // TB0CCR5 CCIFG
//
//        tempoT2[iT2] = TB0CCR5;
//        iT2 += 1;
//        if(iT2==2)
//        {
//            diff_T2 = tempoT2 [iT2-1] - tempoT2 [iT2-2];
//            iT2=0;
//        }
//        //PM5CTL0 = ~LOCKLPM5;
//        TB0CCTL5 &= ~CCIFG;
//        break;
//    case 0x0C:      // TB0CCR6 CCIFG
//
//        tempoT1[iT1] = TB0CCR6;
//        iT1 += 1;
//        if(iT1==2)
//        {
//            diff_T1 = tempoT1 [iT1-1] - tempoT1 [iT1-2];
//            iT1=0;
//        }
//       // PM5CTL0 = ~LOCKLPM5;
//        TB0CCTL6 &= ~CCIFG;
//        break;
//    case 0x0E:      //P2.6
//        break;
//    }
//}

// Interrupção pelo TA2, timer para captura do sinal Echo do sensor de ultrassom do Tanque 1 -------------------
#pragma vector = TIMER2_A0_VECTOR
__interrupt void SU_TANQUE1_ISR(void)
{
    contadorEchoT1[iT1] = TA2CCR0;
    iT1 += 1;
    //    if(iT1==2)
    //    {
    if(iT1==2)
    {
        diff_T1 = contadorEchoT1 [iT1-1] - contadorEchoT1 [iT1-2];
        iT1=0;
    }
    ////        if(kT1 <= 6 )                            // Se o sensor de ultrassom ainda não obteve 7 medidas
    ////        {
    ////        diff_T1 = contadorEchoT1 [iT1-1] - contadorEchoT1 [iT1-2];
    ////        iT1=0;
    ////        diffVec_T1[kT1] = diff_T1;
    ////        kT1 += 1;
    ////        }
    ////    }
    TA2CCTL0 &= ~CCIFG;                     // Zera a flag de captura do timer TA2.0
}
////// Interrupção pelo TA2, timer para captura do sinal Echo do sensor de ultrassom do Tanque 2 -------------------
#pragma vector = TIMER2_A1_VECTOR
__interrupt void SU_TANQUE2_ISR(void)
{
    contadorEchoT2[iT2] = TA2CCR1;
    iT2 += 1;
    if(iT2==2)
    {
        diff_T2 = contadorEchoT2 [iT2-1] - contadorEchoT2 [iT2-2];
        iT2=0;
    }
    //    PM5CTL0 = ~LOCKLPM5;
    //    contadorEchoT2[iT2] = TA2CCR1;
    //    iT2 += 1;
    ////    if(iT2==2)
    ////    {
    ////        if(kT2 <= 6)                             // Se o sensor de ultrassom ainda não obteve 7 medidas
    ////        {
    ////        diff_T2 = contadorEchoT2 [iT2-1] - contadorEchoT2 [iT2-2];
    ////        iT2=0;
    ////        diffVec_T2[kT2] = diff_T2;
    ////        kT2 += 1;
    ////        }
    ////    }
    TA2CCTL1 &= ~CCIFG;                     // Zera a flag de captura do timer TA2.0
}
//----------------------------------------------------------------------------------
void operador_SU()
{

    // Adicionar um timer para o SU nï¿½o operar to do tempo
    // Estimando distancia "s1_Ultra" e escrevendo no visor. TANQUE 1
    // Estimando distancia "s2_Ultra" e escrevendo no visor. TANQUE 2
//    if(kT1>=6)                              // Se o sensor de ultrassom já obteve 7 medidas
//    {
        //        TA2CCTL0 &= ~CCIE;                     // Desabilita interrupções pelo SU1
        //        TA2CCTL1 &= ~CCIE;                     // Desabilita interrupções pelo SU2
        // A seguir a ordenação tipo bolha do vetor "diffVec_T1[k]"
//    if(toggle = 1)
//    {
//        // Sensor do tanque 1
//        P3DIR |=  BIT6;                         // P3.6 como output para o Trigger. TB0.6
//        P3SEL |=  BIT6;                         // Modo periférico selecionado
//
//    }
        kT1 = 0;
        volatile unsigned int m1 = 0;
        volatile unsigned int n1 = 0;
       // while(n1 <= 6)
            //         {
            //            m1= 0;
            //             while(m1 < 6)
            //             {
            //                 if(diffVec_T1[m1] > diffVec_T1[m1+1])
            //                 {
            //                     ordenacaoMedicoesSU_T1 = diffVec_T1[m1];
            //                     diffVec_T1[m1] = diffVec_T1[m1+1];
            //                     diffVec_T1[m1+1] = ordenacaoMedicoesSU_T1;
            //                 }
            //                m1+=  1;
            //             }
            //            n1+= 1;
            //         }
            //         medianaMedicoesSU_T1 = diffVec_T1[3];
            //         s1_Ultra = abs (medianaMedicoesSU_T1)/ 58;                  // Toma o valor absoluto de "diff_T1" e divde por 58. 58 vem do datasheet do sensor de ultrassom
            s1_Ultra = abs (diff_T1/*medianaMedicoesSU_T1*/)/ 58;                  // Toma o valor absoluto de "diff_T1" e divde por 58. 58 vem do datasheet do sensor de ultrassom
        // volume para um balde cilindrico
        //volumeT1 = 3.1415*(8.4)*s1_Ultra;
        lcdCursor(PCF8574_LCD1, 0x1C);
        lcdNum(PCF8574_LCD1,s1_Ultra);
        if(s1_Ultra > 250)
        {
            s1_Ultra = 250;
        }

       // inteiroParaString(s1_Ultra, tanque1L);

    //    if(kT2>=6)                              // Se o sensor de ultrassom já obteve 7 medidas
    //    {
    //        kT2 = 0;
    //        // A seguir a ordenação tipo bolha do vetor "diffVec_T1[k]"
    //        volatile unsigned int m2 = 0;
    //        volatile unsigned int n2 = 0;
    //         while(n2 <= 6)
    //         {
    //             m2 = 0;
    //             while(m2 < 2)
    //             {
    //                 if(diffVec_T2[m2] > diffVec_T2[m2+1])
    //                 {
    //                     ordenacaoMedicoesSU_T2 = diffVec_T2[m2];
    //                     diffVec_T2[m2] = diffVec_T2[m2+1];
    //                     diffVec_T2[m2+1] = ordenacaoMedicoesSU_T2;
    //                 }
    //                 m2 +=  1;
    //             }
    //             n2 += 1;
    //         }
    //         medianaMedicoesSU_T2 = diffVec_T2[3];
    //         s2_Ultra = abs (medianaMedicoesSU_T2)/ 58;                  // Toma o valor absoluto de "diff_T1" e divde por 58. 58 vem do datasheet do sensor de ultrassom
    s2_Ultra = abs (diff_T2 /*medianaMedicoesSU_T2*/ )/ 58;                  // Toma o valor absoluto de "diff_T1" e divde por 58. 58 vem do datasheet do sensor de ultrassom
    //volumeT2 = 3.1415*(8.4)*s2_Ultra;
    lcdCursor(PCF8574_LCD1, 0x23);
    lcdNum(PCF8574_LCD1, s2_Ultra);
    if(s2_Ultra > 250)
    {
        s2_Ultra = 250;
    }
}

//----------------- TIMERS DO SENSOR DE ULTRASSOM ----------------------------------
// Timer para Trigger do sensor utrassom. P3.6
void tb0_ultrassom_config()
{
    //----------------- SENSOR DE ULTRASSOM ------------------------
    // Sensor do tanque 1
    P3DIR |=  BIT6;                         // P3.6 como output para o Trigger. TB0.6
    P3SEL |=  BIT6;                         // Modo periférico selecionado

    // Sensor do tanque 2
    P3DIR |=  BIT5;                         // P3.5 como output para o Trigger. TB0.5
    P3SEL |=  BIT5;                         // Modo periférico selecionado

//    // Sensor do tanque 3
//    P7DIR |=  BIT4;                         // P7.6 como output para o Trigger. TB0.4
//    P7SEL |=  BIT4;                         // Modo periférico selecionado

    // Configurando o Timer B0 para o sensor de ultrassom do Tanque 1
    TB0CTL  = TASSEL__SMCLK | MC__UP;       // TASSEL em SMCLK = SMCLK = 1.048.570 Hz . MC_UP -> Contagem vai ate TA0CCR0, com periodo de
    // pelo menos 60 ms --> basta definir TA0CCR0 como 0xFFFE, que eh o maximo valor de contagem (65535)
    //
    // trigger sera de  10us --> no modo Toggle/Reset
    TB0CCR0  = 0xFFFE;                      // HCSR04 pede ao menos 60 ms de espacamento entre os triggers
    // TB0R conta de 0 ate 65.535, com o SMCLk, o tempo gasto de 63 ms, entao TB0CCR0 pode ser 0xFFFE

    // Trigger para o Tanque 1
    TB0CCR6  = 0x000A;                      // Conta 9.5368 us ~= 10 us. No pino 3.6 temos a unidade de captura TB0CCR6
    TB0CCTL6 = OUTMOD_7;                    // Modo Reset/Set do PWM que tera periodo de ~65ms e 10us em modo alto
    wait(50,ms);
    // Trigger para o Tanque 2
    TB0CCR5  = 0x000A;                      // Conta 9.5368 us ~= 10 us. No pino 3.6 temos a unidade de captura TB0CCR6
    TB0CCTL5 = OUTMOD_7;                    // Modo Reset/Set do PWM que tera periodo de ~65ms e 10us em modo alto
    wait(50,ms);
    // Trigger para o Tanque 3
//    TB0CCR2  = 0x000A;                      // Conta 9.5368 us ~= 10 us. No pino 3.6 temos a unidade de captura TB0CCR6
//    TB0CCTL2 = OUTMOD_7;                    // Modo Reset/Set do PWM que tera periodo de ~65ms e 10us em modo alto
}
// Timer para captura do ECHO dos dois sensores de ultrassom. P2.3 e P2.4
void ta2_ultrassom_config()
{
    // T1
    P2DIR &= ~BIT3;                         // P2.3. TA2.0. PORTA UTILIZADA PARA CAPTURA DO SINAL ECHO. ENTRADA
    P2SEL |=  BIT3;                         // P2.3 COM MODO periferico SELECIONADO
    // T2
    P2DIR &= ~BIT4;                         // P2.4. TA2.1. PORTA UTILIZADA PARA CAPTURA DO SINAL ECHO. ENTRADA
    P2SEL |=  BIT4;                         // P2.4 COM MODO periferico SELECIONADO
    //T3? Nao me lembro
//    P2DIR &= ~BIT5;                         // P2.5. TA2.2. PORTA UTILIZADA PARA CAPTURA DO SINAL ECHO. ENTRADA
//    P2SEL |=  BIT5;                         // P2.5 COM MODO periferico SELECIONADO

    TA2CTL   = TASSEL__SMCLK | MC_2;        // TASSEL em SMCLK = SMCLK = 1.048.570 Hz . MC_2 == MC__CONTINUOS
    TA2CCTL0 = CAP | CCIE | CCIS_0 | CM_3 | SCS;  // Modo captura | Interrupt enable | 00b = CCIxA |Both rising and falling edges
                                                  // | SYNCHRONOUS CAP
    TA2CCTL1 = CAP | CCIE | CCIS_0 | CM_3 | SCS;  // MODO CAPTURA | INTERRUPT ENABLE | 00B = CCIXA |BOTH RISING AND FALLING EDGES
                                                  // | SYNCHRONOUS CAP
    //TA2CCTL2 = CAP | CCIE | CCIS_0 | CM_3 | SCS;  // MODO CAPTURA | INTERRUPT ENABLE | 00B = CCIXA |BOTH RISING AND FALLING EDGES
                                                  // | SYNCHRONOUS CAP
}
//----------------------------------------------------------------------------------



