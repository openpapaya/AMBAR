
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
#include "libs/ambar_uart_5529.h"
#include "libs/ambar_lcd_5529.h"
#include "libs/ambar_timers_5529.h"
#include "libs/ambar_valvulas_5529.h"
#include "libs/funcoes_ina219_5529.h"
#include "libs/funcoes_DS18B20_5529.h"
#include "libs/funcoes_PCF8574_5529.h"
#include "libs/ambar_comESP32_5529.h"

// If you have to write comments, do not write about what the code is doing.
// Instead, write about why the code is doing what it's doing.
// Write comments that you would want to read five years from now
// when you've forgotten everything about this code.
// And the fate of the world is depending on you. No pressure.

//                              *** ENDERECOS I2C ***
// PCF8574 - CONTROLE DAS VALVULAS      - Endereco: 0x20
// PCF8574 - SENSORES DE NIVEL E RELES  - Endereco: 0x21
// PCF8574 - LCD                        - Endereco: 0x27
// INA219  - Leitor Tensao/Corrente     - Endereco: 0x40
// ESP32   - Cliente MQTT               - Endereco: 0x42

#define      PCF8574_valves  0x20
#define      PCF8574_sNivel  0x21
#define      ESP32           0x42
#define      INA219          0x40
#define      PCF8574_LCD1    0x27
#define      PCF8574_LCD2    0x26

//Variaveis Gobais
char fromESP32[1];                 // Menssagem vinda do cliente ESP32 para os tanques principais
char toESP32[1];                   // Menssagem vinda do preparador de nutrientes para o ESP32
extern uint8_t TXESP32Data;
extern uint8_t TXByteCtr;
extern volatile char stringToEsp32[10];
volatile unsigned int prepNutSendingTo_T1;
volatile unsigned int prepNutSendingTo_T2;
volatile uint8_t flagP1IES;
// Sensor de temperatura
volatile uint8_t flag_temp;
volatile uint8_t flag_SU1;
volatile uint8_t flag_SU2;
//volatile unsigned int flag_SU;
volatile unsigned int flag_LCD;
volatile unsigned int flag_regT1_ok;
volatile unsigned int flag_regT2_ok;
volatile unsigned int flag_1_sec;
volatile unsigned int flag_5_sec;
volatile unsigned int flagTvalve;
volatile unsigned int atualizarUart;
volatile unsigned int flag_milisec;
volatile unsigned int rotina_Inicial;
volatile unsigned int menu_entraEsai;
volatile unsigned int opcoesMenu;
// Vindas de ambar_valvulas_5529
extern volatile uint8_t flag_regT1ok;
extern volatile uint8_t flag_regT2ok;
// Variaveis vindas de uartAmbar5529
extern uint8_t flagOpCmd;
extern uint8_t flagOpCmd_UCA0;
// Variaveis vindas de ambar_lcd_5529
extern volatile uint8_t atualizarLCD;
// Variaveis vindas de ambar_ultrassom_5529
extern volatile unsigned int s1_Ultra;
extern volatile unsigned int s2_Ultra;
// Variaveis para tratar da comunicacao com o ESP32


// Variaveis vindas de ambar_uart_5529
extern volatile uint8_t flag_softDog;
extern volatile uint8_t countExit;
extern volatile uint8_t flagClrBuff;
//
extern volatile unsigned int t1_closeRegIN, t2_closeRegIN, t1_closeRegOUT, t2_closeRegOUT;             // Solicitaeheho de fechamento e abertura de vehlvulas
extern volatile unsigned int t1_openRegIN, t2_openRegIN, t1_openRegOUT, t2_openRegOUT;             // Solicitaeheho de fechamento e abertura de vehlvulas


//Protehtipo das funcoes
void gpioConfig();                           //Chama a Funcao io_config, para que o GPIO seja configurado
void lcdInit();
void lcdWriteByte(uint8_t addr, uint8_t byte, uint8_t instr0char1);
void i2cConfig();
void ta0Config(void);                       //Funcao que configura o Timer A0.
void tb0Config(void);                       //Funcao que configura o Timer b0.
void monNivelAB_1_2(void);                  // inicializacao dos tanque 1 e 2
void operador_T1_T2(void);                  //
uint8_t sNivel(void);            // Tratamento de rebotes do SU e dos sensores de nehvel do tanque 1
void rebotesPorta2(void);                   // Tratamento de rebotes do SU e dos sensores de nehvel do tanque 1
void operador_botaoCapacitivo(void);
void evapTimer_T1(void);                    // Contador do tempo de evaporacao do tanque 1
void evapTimer_T2(void);                    // Contador do tempo de evaporacao do tanque 2
void operador_LCD(uint8_t addr);            //
void operador_SU(void);
void uartConfig();
void uart_operador_ambar5529();
void uartWrite_UCA0(volatile char * str );
void uartMonitorAmbar();
void ta2_ultrassom_config(void);            //
void tb0_ultrassom_config(void);            //
uint8_t read_temp();
void serial_esp32();

//                              *** ENDERECOS I2C ***
// PCF8574 - CONTROLE DAS VALVULAS  - Endereco de escrita: 0x20       Endereco de leitura: 0x41
// PCF8574 - SENSORES DE NIVEL      - Endereco de escrita: N/A (0x42) Endereco de leitura: 0x43
// PCF8574 - LCD                    - Endereco de escrita: 0x27       Endereco de leitura: ?
// INA219  - Leitor Tensao/Corrente - Endereco de escrita: 0x40       Endereco de leitura: 0x40
//*************************************************************
// Sensores de nivel conectados as portas P7(A1), P6(B1), P5(A2) e P4(B2) da PCF
//*************************************************************
//  ---------- MAPEAMENTO ENTRE A PCF8574 DO DECOTER E A FUNCAO "i2cWriteByte" ------------
//  |       P1  P2  P3  P4  P5  P6  P7
//  |       0b 7   6   5   4   3   2   1
//  |
//*************************************************************
//      - Configuration After Reset -
// 1. Initialize Ports: PxDIR, PxREN, PxOUT, and PxIES
// 2. Clear LOCKLPM5
// 3. If not wake-up from LPMx.5: clear all PxIFGs to avoid erroneous port interrupts
// 4. Enable port interrupts in PxIE
//**************************************************************
int  main(void)
{
    WDTCTL = WDTPW            |               // Senha (PW) do watchdog timer
            // WDTPW          |               // Modo: Intervalo
            // WDTSSEL__ACLK  |               // Clock: ACLK
            // WDTIS__32K;                    // Intervalo: 1 segundo
            WDTHOLD;   // stop watchdog timer
    // PM5CTL0 = ~LOCKLPM5;
    volatile int i;
    rotina_Inicial = 0;
    flagP1IES = 0;
    flag_temp = 5;
    flag_SU1 = 2;
    flag_SU2 = 0;

    // __low_power_mode_0();                // Esse modo de baixo consumo nao desliga o ACLK
    gpioConfig();                           // Chama a funcao io_config, para que o GPIO seja configurado
    __enable_interrupt();
    //    i2cConfig();                            // Funcao que configura a comunicaeheho I2C
    i2cWriteByte_UCB1(PCF8574_valves, 0b11111111 ); // Desativa todos os TIP127
   __delay_cycles(1000);
    lcdInit(PCF8574_LCD1);                  // Funcao que inicializa o LCD
    lcdLayout(PCF8574_LCD1,defaut);                // Volta para o layout inicial do LCD
    oneWireConfig();                           // Configurando pinos para comunicacao One Wire
    readPCF8574(ADDR_SNIVEL );               //  Importante. Faz uma leitura do PCF dos sensores de nivel para poder resetar a interrupcao( pino 13) caso esteja ativa.
    ta0Config();                            // Funcao que configura o Timer A0.
  //  ina219mode( 0x40, powerdown);           // 000: Power-Down mode reduces the quiescent current and turns off
  //  ina219config(0x40, 0x019F);             // ina219config( 0x40, 0b0001100110011111) -> PGA = 00 => 40mV / 0.1 ohm = 400 mA de corrente máxima esperada
    // ina219config( 0x40, 0b0000000110011111) ->
  //  ina219calibration(0x40);                // Escreve no endereco do registrador de calibragem do ina219, utilizando a equacao
    // cal = 0,04096/(current_LSB*Rshunt). Esta como current_LSB = 20 uA/bit e Rshunt= 0.1 ohm
    // i2cScanner
    tb0_ultrassom_config();                 // Configuraeheho do TB0 para o trigger do SU
    ta2_ultrassom_config();                 //
    uartConfig(1);                          // TERMINAL - Solititar comunicacao serial pelo. P4.5 = UCA1RXD. P4.4 = UCA1TXD. 1 = UCA1,
    uartConfig(0);                          // ESP32 - Solititar comunicacao serial pelo. P3.4 = UCA0RXD. P3.3 = UCA0TXD. 0 = UCA0,

    monNivelAB_1_2();                       // Monitorar niveis ao inicializar
    uartMonitorAmbar();                     //

    while(1)
    {

//        read_PCF8574 = UCB1RXBUF;
//        UCB1IFG &= ~UCRXIFG;                  // Clear USCI_B1 RX int flag
//        UCB1IFG &= ~UCTXIFG;                  // Clear USCI_B1 TX int flag
//        UCB1CTL1 |= UCTXSTP;                  // I2C stop condition
        if(countExit <= 0)
        {
            flagClrBuff = 1;
        }
        if (flagOpCmd == 1)                 // Flag alterada pela interrupcao pela uart
        {
            // Colocar como task de alta prioridade
            uart_operador_ambar5529();      //
        }
        if( atualizarUart & (!(flagTvalve)) == 1)
        {
            // Task de prioridade idle
            uartMonitorAmbar();             // Tela de monitoramento no terminal
            atualizarUart = 0;
        }
        if(flag_1_sec & atualizarLCD)
        {
            // lcd_T1Tool = 1 --> T1 enchendo
            // "evap_T1" (lcd_T1Tool=2) --> Timer de evaporacao
            // lcd_T1Tool = 3 --> Evap OK/Pronto para receber nutrientes
            // lcd_T1Tool = 4 --> T1 regando
            // lcd_T1Tool = 5 --> T1 vazio
            // lcd_T1Tool = 6 --> Erro no T1

            if(flagTvalve)
            {
               // ina219Print(ina219, 4);
                volatile static int tValve = 13;
                if(--tValve <= 0)
                {
                    //   operador_info(0x27);         // Atualiza o estado dos tanques
                    i2cWriteByte_UCB1( PCF8574_valves , 0xFF);          // Desativar todos os TIP127
                    __delay_cycles(1000);
                    lcdLayout(PCF8574_LCD1,defaut);                // Volta para o layout inicial do LCD
                    atualizarUart = 1;
                    flagTvalve = 0;
                    tValve = 13;
                }
            }
            else
            {
                lcdWriteByte(PCF8574_LCD1,0x0C, 0);      // Desligar o cursor e o blink 0  0  0  0  1  D  C  B,
                static volatile int refresh = 2;
                if(refresh-- <= 0)
                {
                    operador_LCD(0x27);         // Atualiza o sistema sobre estado dos tanques
                    refresh = 2;
                }
                sNivel();           // Tratamento de rebotes do SU e dos sensores de nehvel do tanque 1
                evapTimer_T1();
                evapTimer_T2();
              //  ina219Print(ina219, 3);
                operador_SU();
                // Enviando nivel dos tanques para o ESP32 (addr 0x42)
                // i2cWriteByte_UCB0(0x42, 1);
                //                TXESP32Data = 0xFF;
                //                TXByteCtr = 1;
                //               // while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
                //                UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
                //                //i2cWriteByte_UCB0(0x42, s1_Ultra);
                //                TXESP32Data = s1_Ultra;
                //                TXByteCtr = 1;
                //                //while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
                //                UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
                //
                //                //i2cWriteByte_UCB0(0x42, 2);
                //                TXESP32Data = 0x04;
                //                TXByteCtr = 1;
                //              //  while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
                //                UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
                //                //i2cWriteByte_UCB0(0x42, s2_Ultra);
                //                TXESP32Data = s2_Ultra;
                //                TXByteCtr = 1;
                //               // while (UCB0CTL1 & UCTXSTP);             // Ensure stop condition got sent
                //UCB0CTL1 |= UCTR + UCTXSTT;             // I2C TX, start condition
                operador_T1_T2();
            }
            flag_1_sec = 0;
            volatile static int flag_5_sec = 5;
            if(flag_5_sec-- <= 0)
            {
                flag_5_sec = 5;
                serial_esp32();
                read_temp();
            }
            volatile static int flag_60_sec = 60;
            if(flag_60_sec-- <= 0)
            {
                flag_60_sec = 60;
                lcdInit(PCF8574_LCD1);
                lcdLayout(PCF8574_LCD1,defaut);                // Volta para o layout inicial do LCD

            }
        }
    } // while
} // int main


// ------------------------ Roteiro de decisoes -----------------------------------------
//      - A regra aqui eh que nenhuma linha tenha mais que um estado diferente em relacao eh linha anterior, para evitar sinalizaehehes simutaneas
//      - eh assumido que os tanques possuem volume suficiente para o tempo de rega ser maior que o tempo de evaporacao de cloro

//        agua no T1| agua no T2 | Enchendo T1| Enchendo T2 | Evaporando cloro T1 | Evaporando cloro  T2 | Ok para regar por T1 | Ok para regar pot T2 | saida ON T1 | saida ON T2
//  Na inicializacao:

//          T1 VAZIO, T2 CHEIO       :      0       0       0       0       0       0       0       0       0       0       *Estado 1*
//
//          T1 CHEIO, T2 VAZIO       :      0       0       0       0       0       0       0       0       0       0       *Estado 1*
//          T1 CHEIO, T2 CHEIO       :      0       0       0       0       0       0       0       0       0       0       *Estado 1*


//                                                  T1.agua|T2.agua|Ench_T1|Ench_T2|Evap_T1|Evap_T2|RegT1ok|RegT2ok|saidaT1|saidaT2
//        T1 VAZIO, T2 VAZIO                    :      0       0       0       0       0       0       0       0       0       0       *Estado 1*   T1 VAZIO, T2 VAZIO
//  Abrir registro de entrada do T1             :      0       0      -1-      0       0       0       0       0       0       0       *Estado 2*
//        T1 acabou de encher                   :     -1-      0       1       0       0       0       0       0       0       0       *Estado 3*
//  Fechar registro de entrada do T1            :      1       0      -0-      0       0       0       0       0       0       0       *Estado 4*
//  Iniciando evaporacao de cloro do T1         :      1       0       0       0      -1-      0       0       0       0       0       *Estado 5*
//  Finalizada evaporacao pelo T1               :      1       0       0       0       1       0      -1-      0       0       0       *Estado 6*
//  Abrir registro de saida do T1               :      1       0       0       0       1       0       1       0      -1-      0       *Estado 7*
//  Abrir registro de entrada do T2             :      1       0       0      -1-      1       0       1       0       1       0       *Estado 8*
//        T2 acabou de encher                   :      1      -1-      0       1       1       0       1       0       1       0       *Estado 9*
//  Fechar registro de entrada do T2            :      1       1       0      -0-      1       0       1       0       1       0       *Estado 10*
//  Iniciando evaporacao de cloro do T2         :      1       1       0       0       1      -1-      1       0       1       0       *Estado 11*
//  Finalizada evaporacao pelo T2               :      1       1       0       0       1       1       1      -1-      1       0       *Estado 12*
//  T1 VAZIO, T2 CHEIO                          :     -0-      1       0       0       1       1       1       1       1       0       *Estado 13*  T1 VAZIO, T2 CHEIO
//  Fechar registro de saida do T1              :      0       1       0       0       1       1       1       1      -0-      0       *Estado 14*
//  Zerar timer de evaporacao de cloro em T1    :      0       1       0       0      -0-      1       1       1       0       0       *Estado 16*
//  T1 nao autorizado para regar                :      0       1       0       0       0       1      -0-      1       0       0       *Estado 17*
//  Abrir registro de entrada do T1             :      0       1      -1-      0       0       1       0       1       0       0       *Estado 15*
//  Abrir registro de saida do T2               :      0       1       1       0       0       1       0       1       0      -1-      *Estado 18*
//        T1 acabou de encher                   :     -1-      1       1       0       0       1       0       1       0       1       *Estado 19*
//  Fechar registro de entrada do T1            :      1       1      -0-      0       0       1       0       1       0       1       *Estado 20*
//  Iniciando evaporacao de cloro do T1         :      1       1       0       0      -1-      1       0       1       0       1       *Estado 21*
//  Finalizada evaporacao pelo T1               :      1       1       0       0       1       1      -1-      1       0       1       *Estado 22*
//  T1 CHEIO, T2 VAZIO                          :      1      -0-      0       0       1       1       1       1       0       1       *Estado 23*  T1 CHEIO, T2 VAZIO
//  Fechar registro de saida do T2              :      1       0       0       0       1       1       1       1       0      -0-      *Estado 24*
//  Zerar timer de evaporacao de cloro em T2    :      1       0       0       0       1      -0-      1       1       0       0       *Estado 25*
//  T2 nao autorizado para regar                :      1       0       0       0       1       0      -0-      1       0       0       *Estado 26*
//  Abrir registro de entrada do T2             :      1       0       0      -1-      1       0       0       1       0       0       *Estado 27*
//  Abrir registro de saida do T1               :      1       0       0       1       1       0       0       1      -1-      0       *Estado 28*
//        T2 acabou de encher                   :      1      -1-      0       1       1       0       0       1       1       0       *Estado 29*
//  Fechar registro de entrada do T2            :      1       1       0      -0-      1       0       0       1       1       0       *Estado 30*
//  Iniciando evaporacao de cloro do T2         :      1       1       0       0       1      -1-      0       1       1       0       *Estado 31*
//  Finalizada evaporacao pelo T2               :      1       1       0       0       1       1      -1-      1       1       0       *Estado 32*
//  T1 CHEIO, T2 CHEIO                          :      1       1       0       0       1       1       1       1       1       0       *Estado 33*  T1 CHEIO, T2 CHEIO escolhido nesse estado para ficar compativel com o o mesmo estado de inicializacao quando T1 CHEIO e T2 CHEIO, em que T1 devereh ser o primeiro a regar

//                NivelA1|NivelB1|NivelA2|NivelB2|| V.IN1 | VOUT1 | V.IN2 | VOUT2 ||         T1    |    T2    ||regT1ok|regT2ok|t1_regar|t2_regar|
//                   0       0       0       0   ||     ERROR     |     ERROR     || -->  ERRO T1  | ERRO T2  ||   x   |   x   |   x    |   x    |
//                   0       0       0       1   ||     ERROR     |       |   1   || -->  ERRO T1  | T2 CHEIO ||   x   |   1   |   x    |   1    |
//                   0       0       1       0   ||     ERROR     |   1   |       || -->  ERRO T1  | T2 VAZIO ||   x   |   0   |   x    |   0    |
//                   0       0       1       1   ||     ERROR     |       |   1   || -->  ERRO T1  | T2 CHEIO ||   x   |   1   |   x    |   1    |
//                   0       1       0       0   ||   0   |   1   |     ERROR     || -->  T1 CHEIO | ERRO T2  ||   1   |   x   |   1    |   x    |
//                   0       1       0       1   ||   0   |   1   |   0   |   0   || -->  T1 CHEIO | T2 CHEIO ||   1   |   1   |   1    |   0    |
//                   0       1       1       0   ||   0   |   1   |   1   |   0   || -->  T1 CHEIO | T2 VAZIO ||   1   |   0   |   1    |   0    |
//                   0       1       1       1   ||   0   |   0   |   0   |   1   || -->  T1 CHEIO | T2 CHEIO ||   1   |   1   |   1    |   0    |
//                   1       0       0       0   ||   1   |   0   |     ERROR     || -->  T1 CHEIO | T2 CHEIO ||   1   |   1   |   1    |   0    |
//                   1       0       0       1   ||   1   |   0   |   0   |   1   || -->  T1 VAZIO | T2 CHEIO ||   0   |   1   |   0    |   1    |
//                   1       0       1       0   ||   1   |   0   |   0   |   0   || -->  T1 VAZIO | T2 VAZIO ||   0   |   0   |   0    |   0    |
//                   1       0       1       1   ||   1   |   0   |   0   |   1   || -->  T1 VAZIO | T2 CHEIO ||   0   |   1   |   0    |   1    |
//                   1       1       0       0   ||   0   |   1   |     ERROR     || -->  T1 CHEIO | ERRO T2  ||   1   |   x   |   1    |   x    |
//                   1       1       0       1   ||   0   |   1   |   0   |   0   || -->  T1 CHEIO | T2 CHEIO ||   1   |   1   |   1    |   0    |
//                   1       1       1       0   ||   0   |   1   |   1   |   0   || -->  T1 CHEIO | T2 VAZIO ||   1   |   0   |   1    |   0    |
//                   1       1       1       1   ||   1   |   0   |   0   |   0   || -->  T1 CHEIO | T2 CHEIO ||   1   |   1   |   1    |   0    |

//    _____________               AB
//    |-----------|--> Nivel A    01
//    |-----------|
//    |-----------|               11
//    |-----------|
//    |___________|--> Nivel B    10
//                            00 == ERRO
//----------------------------------
// PH:
//
// Com o pH errado, metade dos nutrientes recomendados serÃ¡ suficiente para causar uma overfert.
// Com o pH certo, a dose mÃ¡xima recomendada de qualquer nutriente farÃ¡ as plantas crescerem saudÃ¡veis â€‹â€‹e fortes, aproveitando-os ao mÃ¡ximo.
//
// ----------------------------------
// Condutividade ElÃ©trica:
//
//EC stands for Electrical Conductivity. This measures the amount of nutrients (salts) in the water.
// The higher the EC, the more salt is in the water.
// This is important because too much or too little nutrients can harm the plants.
// Para saber se vocÃª estÃ¡ usando muito ou pouco fertilizante, Ã© preciso monitorar o valor da EC da soluÃ§Ã£o de Ã¡gua + nutrientes.
// Partindo, por exemplo, de uma soluÃ§Ã£o com concentraÃ§Ã£o de 1,0, Ã© preciso avaliar no dia seguinte Ã  rega se o valor de EC aumentou ou diminuiu.
// Supondo que apÃ³s a rega a leitura da Ã¡gua seja de 1,4 , isto significa que a soluÃ§Ã£o inicial estava muito concentrada,
// ou seja, Ã© preciso dissolver a quantidade de nutrientes adicionado com a adiÃ§Ã£o de mais Ã¡gua. Se no dia seguinte o valor da EC estiver 0,7,
// por exemplo, a planta absorveu os nutrientes mais rapidamente do que a Ã¡gua. Portanto, pode-se aumentar a quantidade de nutrientes.
// Electrical conductivity is measure of the ionic strength of a solution and can be converted into concentration.
// -----------------------------------
// Total de SÃ³lidos Dissolvidos (TDS)
//
// Since the conductivity of a nutrient solution is related to the concentration of dissolved solids present,
// conductivity readings can also be expressed as total dissolved solids (TDS). The measurement of TDS indicates the amount of ions dissolved in a solution.
// In terms of hydroponics, TDS specifies the salt concentration and strength of a nutrient solution.
// A true mass measurement of TDS is determined by a gravimetric analysis.
// However, an estimated measurement of TDS can more quickly and simply be extrapolated from an EC measurement.
// ----------------------------------
// OxigÃªnio Dissolvido:
//
// DO stands for dissolved oxygen and measures how much oxygen is dissolved in the water.
// The higher the DO, the more oxygen is available for plants. This is important for plant respiration.
// ----------------------------------
// Temperatura (ÂºC)
//
// Temperature is important to measure because it affects how much oxygen is dissolved in the water. The warmer the water, the less dissolved oxygen there is.
// The temperature affects all these chemical reactions and should be monitored to ensure that the plants are not too hot or too cold.
//
// EC and pH are important because they affect how well plants absorb nutrients.
// DO is crucial because it affects how much oxygen plants have available to them.
// Temperature is critical because it can affect plant metabolism.
// Ensure to keep the roots of the plants submerged in water at all times.
// This will help prevent oxygen depletion and keep the roots cooler, which is vital for plant growth.
// It would be best if you recirculated the nutrient solution regularly. This will help to keep the concentrations of nutrients and dissolved oxygen at optimal levels.
// You should monitor the temperature of the hydroponic system closely. It can adversely affect plant growth if it gets too hot or too cold.
