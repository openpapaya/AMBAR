/*
 * timer.c
 *
 *  Created on: 5 de jul de 2021
 *      Author: guiwz
 */
#include <msp430.h>
#include <stdint.h>

#include <libs/config_gpio_5529.h>
#include <libs/config_timers_5529.h>

// TA0 em ACLK. Utilizado para tempo de evaporacao, rega, e tratamento de rebotes. TA0 gera interrupcoess a cada 1 segundo
// TA1 em SMCLK, TA1 utilizado nas funcoes "wait"
// TA2 Nos su's
// TB0 Sendo utilizado para os TRIGGERs e ECHOs dos SUs

//----------------------------------------------------------
//----------------- Configuracao do Real Time Clock---------------------------------
//-- Ira fornecer mediï¿½ï¿½o para o calendario de adubacoes, coleta de dados, etc -----
//void rtc_config()
//{
//
//}
//----------------------------------------------------------------------------------
// Timer para contar 1 segundo
void ta0Config()                           //Funï¿½ï¿½o que configura o Timer A0.
{
    TA0CTL   = TASSEL__ACLK | MC__UP;       // TASSEL em ACLK = 32.768 Hz . MC_UP -> Contagem vai ate TA0CCR0.
    TA0CCR0  = 0x8000 - 1;                  // Conta ~ 1 segundo
    TA0CCTL0 = CCIE;                        // Para utilizar a interrupcao do timer A
}

volatile uint16_t  count;
//volatile uint8_t  flagTvalve;
//volatile uint8_t  tValve = 7;
//
//extern volatile uint8_t flag_softDog;
//extern volatile uint8_t countExit;

void wait( uint16_t time, timeunit_t unit )
{

    if ( unit == us)
    {
        // Use o SMCLK
        TA1CTL  = TASSEL__SMCLK | MC__UP | TACLR;
        TA1CCR0 = time;
        while(! ( TA1CCTL0 & CCIFG ));
        TA1CCTL0 &= ~CCIFG;
    }
    else
    {
        // Use o ACLK
        TA1CTL = TASSEL__ACLK | MC__UP | TACLR;
        if ( unit == ms )                   // Passo de contagem Ã© passo =  1/32768 = 0.00003051757 segundos = 1/ 2Â¹â�µ
                                            // Quero contar atÃ© time , portanto time = N * passo = N/2Â¹â�µ
                                            // N Ã© no timer A1 o "TA1CCR0", se o tempo serÃ¡ trabalhado em ms,
                                            // time (ms) = (N/2Â¹â�µ) * 1024 = N/2â�µ, finalmente, isolando o N,
                                            // (Assumindo 1000 = 1024 = 2Â¹â�°, pois facilita para o processador)
                                            // N = TA1CCR0 = time * 2â�µ, que Ã© deslocar para a esquerda 5 vezes em binÃ¡rio
        {
            TA1CCR0 = (time << 5) - 1;       // N = time* 32768 / 32768 = time * 2^5
                                             // (aproximando 1000 por 1024)
            while(!( TA1CCTL0 & CCIFG ));
            TA1CCTL0 &= ~CCIFG;
        }
        if ( unit == sec)
        {
            count    = time;
            TA1CCR0  = 0x8000 - 1;            // Conta 1 segundo
            TA1CCTL0 = CCIE;
            while( count);
        }
        if ( unit == min)
        {
            count    = time * 60;

            TA1CCR0  = 0x8000 - 1;          // Conta 1 segundo
            TA1CCTL0 = CCIE;
            while( count);
        }

        TA1CTL = MC__STOP | TACLR;;

    }
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_CCR0_ISR()
{
    count-- ;
//    if(flagTvalve)                              // Tempo de abertura/fechamento das valvulas
//    {
//        tValve--;
//    }
//    if(flag_softDog)
//    {
//        countExit--;
//    }
}

