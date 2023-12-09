/*
 * LCD_OperadorAmbarc
 *
 *  Created on: 9 de jul de 2021
 *      Author: guiwz
 */
//
#include <msp430.h>
#include <stdint.h>

#include "libs/config_i2c_5529.h"
#include "libs/config_lcd_5529.h"
#include "libs/config_timers_5529.h"
#include "libs/ambar_lcd_5529.h"
#include "libs/ambar_uart_5529.h"

#define     INSTR  0
#define     CHAR  1
#define     PCF8574_LCD1        0x27
#define     PCF8574_LCD2        0x26

//Var�veis Gobais
extern volatile unsigned int evap_T1, evap_T2;              //Vari�vel "evap_T1" e "evap_T2", permite a entrada na fun��o timerevap_T1(2)
extern volatile unsigned int t1_regar, t2_regar;            // Solicitacao de rega
extern volatile unsigned int flag_LCD;
extern volatile unsigned int prepNutSendingTo_T1;
extern volatile unsigned int prepNutSendingTo_T2;
volatile unsigned int lcd_T1Tool,  lcd_T2Tool;
volatile uint8_t atualizarLCD = 1;
volatile char stringToEsp32[10];
uint8_t line = 0x00;
// BIT7 BIT6 BIT5 BIT4 BIT3 BIT2 BIT1 BIT0
void lcdPrint(uint8_t addr,uint8_t * str)
{
    while(*str)
    {
        lcdWriteByte(addr,*str, CHAR);
        str++;
    }
}
//---- Funcao para escrever numeros em ponto flutuante no LCD, com vírgula após o primeiro algarismo e três casas decimais --------------
void lcdFloatNum(uint8_t addr,float v, grandeza tipo)
{
    volatile unsigned int x = 0;
    volatile char y = 0;
    volatile unsigned int i = 0;
    volatile float p = 0;
    if( tipo == temp )
    {
        // Separando milhar
        p = v/1000;                      // Separar centena, e dessa forma escrever o número referente na tela
        y = (int)p;                      // separa o inteiro da parte fracionária
        // Ex: x = #4095,731 --> f = 4095,731/1000 = 4.095731 --> z = (int)f = #4


        if(y == 0)                   // Se a casa do milhar nao existe no valor, nao imprima
        {
            x++;                     // x = 1
        }
        else
            lcdWriteByte(addr, 0x30 + y, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
        // Separando centena
        v = v - 1000*y;                  // v = 4095,731 - 1000*4 = 95,731
        p = v/100;                       // f = 95,731/100 = 0.95731
        y = (int)p;                      // separa o inteiro da parte fracionária
        // z = 0
        if(x == 1)                   // Se o contador aponta que a casa do milhar nao existe
        {
            if(y == 0)               // Se a casa da centena existe, imprima
                x++;                 // x = 2
        }
        else                         // Se a casa do milhar ja existe, continue imprimindo normalmente
        {
            lcdWriteByte(addr, 0x30 + y, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
        }
        // Separando dezena
        v = v - 100*y;                   // v = 095,731 - 100*0 = 95,731
        p = v/10;                        // f = 9.5731
        y = (int)p;                      // separa o inteiro da parte fracionária
        // z = 9
        if(x == 2)                   // Se o contador aponta que a casa do milhar e da centena nao existem
        {
            if(y == 0)               // Se a da dezena tb nao existe,
                x++;                 // x = 3
            else                     // Se existe, envio o numero para o LCD
                lcdWriteByte(addr, 0x30 + y, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
        }
        else                         // Se a casa do milhar ou da centena ja existe, continue imprimindo normalmente
        {
            lcdWriteByte(addr,0x30 + y, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
        }
        // Separando unidade
        v = v - 10*y;                    // v = 95,731 - 10*9 = 5,731
        y = (int)v;
            lcdWriteByte(addr,0x30 + y, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
         lcdPrint(addr,",");
        // Separando primeira casa decimal
        v = v - y;                       // v = 5,731 - 5 = 0,731
        v = 10*v;                        // v = 7,31
        y = (int)v;
        lcdWriteByte(addr,0x30 + y, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
    }
    if( tipo == ina219rd)
    {
        // Separando milhar
        p = v/1000;                      // Separar centena, e dessa forma escrever o número referente na tela
        y = (int)p;                      // separa o inteiro da parte fracionária
        // Ex: x = #4095,731 --> f = 4095,731/1000 = 4.095731 --> z = (int)f = #4

        if(y == 0)                   // Se a casa do milhar nao existe no valor, nao imprima
        {
            x++;                     // x = 1
        }
        else
            lcdWriteByte(addr, 0x30 + y, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
        // Separando centena
        v = v - 1000*y;                  // v = 4095,731 - 1000*4 = 95,731
        p = v/100;                       // f = 95,731/100 = 0.95731
        y = (int)p;                      // separa o inteiro da parte fracionária
        // z = 0
        if(x == 1)                   // Se o contador aponta que a casa do milhar nao existe
        {
            if(y == 0)               // Se a casa da centena existe, imprima
                x++;                 // x = 2
        }
        else                         // Se a casa do milhar ja existe, continue imprimindo normalmente
        {
            lcdWriteByte(addr, 0x30 + y, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
        }
        // Separando dezena
        v = v - 100*y;                   // v = 095,731 - 100*0 = 95,731
        p = v/10;                        // f = 9.5731
        y = (int)p;                      // separa o inteiro da parte fracionária
        // z = 9
        if(x == 2)                   // Se o contador aponta que a casa do milhar e da centena nao existem
        {
            if(y == 0)               // Se a da dezena tb nao existe,
                x++;                 // x = 3
            else                     // Se existe, envio o numero para o LCD
                lcdWriteByte(addr, 0x30 + y, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
        }
        else                         // Se a casa do milhar ou da centena ja existe, continue imprimindo normalmente
        {
            lcdWriteByte(addr, 0x30 + y, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
        }
        // Separando unidade
        v = v - 10*y;                    // v = 95,731 - 10*9 = 5,731
        y = (int)v;
            lcdWriteByte(addr, 0x30 + y, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
            lcdPrint(addr, ",");
        // Separando primeira casa decimal
        v = v - y;                       // v = 5,731 - 5 = 0,731
        v = 10*v;                        // v = 7,31
        y = (int)v;
        lcdWriteByte(addr, 0x30 + y, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
//        // Separando segunda casa decimal
//        v = v - y;                       // v = 7,31 - 7 = 0,31
//        v = 10*v;                        // v = 3,1
//        y = (int)v;
//        lcdWriteByte(addr,0x30 + y, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
//        // Separando terceira casa decimal
//        v = v - y;                       // v = 3,1 - 3 = 0,1
//        v = 10*v;                        // v = 1,0
//        y = (int)v;
//        lcdWriteByte(addr,0x30 + y, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
    }
}
//----------------- Funcao para escrever numeros no LCD ---------------------------------
void lcdNum(uint8_t addr,char x)
{
    char z;
    {
        int f;
        // Separando centena
        f = x/100;                          // Separar centena, e dessa forma escrever o nï¿½mero referente na tela
        z = (int)f;                         // separa o inteiro da parte fracionï¿½ria
                                            // Ex: x = #121 --> f = 121/100 = 1,21 --> z = (int)f = #1
        lcdWriteByte(addr, 0x30 + z, CHAR);       // Da tabele ASCII, os nï¿½meros comeï¿½am em 0x30 = 0, 0x31 = 1, ..., atï¿½ o 0x39 = 9.

//         Separando dezena
        x = x - 100*z;
        f = x/10;                           // Separar dezena, e dessa forma escrever o nï¿½mero referente na tela
        z = (int)f;                         // separa o inteiro da parte fracionï¿½ria
                                            // Ex: x = #121 --> x = x - 100*1 = 21  --> f = x/10 = 2,1 --> z = (int)f = #2
        lcdWriteByte(addr, 0x30 + z, CHAR);       // Da tabele ASCII, os nï¿½meros comeï¿½am em 0x30 = 0, 0x31 = 1, ..., atï¿½ o 0x39 = 9.

        // Separando unidade
        x = x - 10*z;                       // Separar dezena, e dessa forma escrever o nï¿½mero referente na tela
                                            // Ex: x = 21 - 10*2 = 1
        lcdWriteByte(addr, 0x30 + x, CHAR);       // Da tabele ASCII, os nï¿½meros comeï¿½am em 0x30 = 0, 0x31 = 1, ..., atï¿½ o 0x39 = 9.
    }
}
//---- Funcao para escrever numeros no LCD, com vírgula após o primeiro algarismo e duas casas decimais --------------
void lcdNumVir2(uint8_t addr,uint16_t b)
{
    char k;
    {
        int g;
        // Separando milhar
        g = b/1000;                         // Separar centena, e dessa forma escrever o número referente na tela
        k = (int)g;                         // separa o inteiro da parte fracionária
                                            // Ex: x = #4095 --> f = 4095/1000 = 4.095 --> z = (int)f = #4
        lcdWriteByte(addr, 0x30 + k, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
        // Separando centena
        b = b - 1000*k;                     // v = 4095 - 1000*4 = 095
        g = b/100;                          // f = 095/100 = 0.95
        k = (int)g;                         // separa o inteiro da parte fracionária
                                            // z = 0

        lcdWriteByte(addr, 0x30 + k, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
        // Separando dezena
        b = b - 100*k;                      // v = 095 - 100*0 = 95
        g = b/10;                           // f = 9.5
        k = (int)g;                         // separa o inteiro da parte fracionária
        //Acidionar regras de arredondamento
//        if(z >= 5)
//        {
//          z++;
//        }
                                            // z = 9
        lcdWriteByte(addr, 0x30 + k, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
        lcdPrint(addr, ",");                      // Escreve "," no LCD e pula uma linha
        //Separando unidade
        b = b - 10*k;                       // v = 95 - 10*9 = 5
                                            //
        lcdWriteByte(addr, 0x30 + b, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
    }
}
//---- Funcao para escrever numeros no LCD, com vírgula após o primeiro algarismo e três casas decimais --------------
void lcdNumVir3(uint8_t addr,uint16_t v)
{
    char y;
    {
        int p;
        // Separando milhar
        p = v/1000;                         // Separar centena, e dessa forma escrever o número referente na tela
        y = (int)p;                         // separa o inteiro da parte fracionária
                                            // Ex: x = #4095 --> f = 4095/1000 = 4.095 --> z = (int)f = #4
        lcdWriteByte(addr, 0x30 + y, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.

        // Separando centena
        v = v - 1000*y;                     // v = 4095 - 1000*4 = 095
        p = v/100;                          // f = 095/100 = 0.95
        y = (int)p;                         // separa o inteiro da parte fracionária
                                            // z = 0
        lcdWriteByte(addr, 0x30 + y, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.

        lcdPrint(addr, ",");                      // Escreve "," no LCD e pula uma linha

        // Separando dezena
        v = v - 100*y;                      // v = 095 - 100*0 = 95
        p = v/10;                           // f = 9.5
        y = (int)p;                         // separa o inteiro da parte fracionária
                                            // z = 9
        lcdWriteByte(addr, 0x30 + y, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.

        // Separando unidade
        v = v - 10*y;                       // v = 95 - 10*9 = 5
                                            //
        lcdWriteByte(addr, 0x30 + v, CHAR);       // Da tabele ASCII, os numeros começam em 0x30 = 0, 0x31 = 1, ..., até o 0x39 = 9.
    }
}
//----------------- Funcao para escrever nï¿½meros no LCD ---------------------------------
void lcdNumbers_Hour(uint8_t addr,char x)
{
    char z;
    {
        int f;
        //x = 8 - x;                          // contador decressivo de 8 horas, desativado, a contagem decressiva foi colocada na funcao evap
        // Separando dezena
        f = x/10;                           // Separar dezena, e dessa forma escrever o nï¿½mero referente na tela
        z = (int)f;                         // separa o inteiro da parte fracionï¿½ria
                                            // Ex: x = #121 --> x = x - 100*1 = 21  --> f = x/10 = 2,1 --> z = (int)f = #2
        lcdWriteByte(addr, 0x30 + z, CHAR);       // Da tabele ASCII, os nï¿½meros comeï¿½am em 0x30 = 0, 0x31 = 1, ..., atï¿½ o 0x39 = 9.

        // Separando unidade
        x = x - 10*z;                       // Separar dezena, e dessa forma escrever o nï¿½mero referente na tela
                                            // Ex: x = 21 - 10*2 = 1
        lcdWriteByte(addr, 0x30 + x, CHAR);       // Da tabele ASCII, os nï¿½meros comeï¿½am em 0x30 = 0, 0x31 = 1, ..., atï¿½ o 0x39 = 9.
    }
}
void lcdNumbers_Minute(uint8_t addr,char x)
{
    char z;
    {
        int f;
       // x = 59 - x;                       // contador decressivo de 59 minutos, desativado, a contagem decressiva foi colocada na funcao evap
        // Separando dezena
        f = x/10;                           // Separar dezena, e dessa forma escrever o nï¿½mero referente na tela
        z = (int)f;                         // separa o inteiro da parte fracionï¿½ria
                                            // Ex: x = #121 --> x = x - 100*1 = 21  --> f = x/10 = 2,1 --> z = (int)f = #2
        lcdWriteByte(addr, 0x30 + z, CHAR);       // Da tabele ASCII, os nï¿½meros comeï¿½am em 0x30 = 0, 0x31 = 1, ..., atï¿½ o 0x39 = 9.

        // Separando unidade
        x = x - 10*z;                       // Separar dezena, e dessa forma escrever o nï¿½mero referente na tela
                                            // Ex: x = 21 - 10*2 = 1
        lcdWriteByte(addr, 0x30 + x, CHAR);       // Da tabele ASCII, os nï¿½meros comeï¿½am em 0x30 = 0, 0x31 = 1, ..., atï¿½ o 0x39 = 9.
    }
}

//----------------------------------------------------------------------------------------------
// ---------------- Funï¿½ï¿½es para mudar a posiï¿½ï¿½o do cursor do LCD -------------------------------
//----------------------------------------------------------------------------------------------
void lcdCmd(uint8_t addr,uint8_t byte, uint8_t  instr0char1)       // Funcao que prepara os dois nibbles que serï¿½o os 8 bits de instruï¿½ï¿½o
{
    lcdWriteNibble( addr,byte >> 4,   instr0char1 );        // Manda primeiro o MSNibble, seguindo a lï¿½gica do protocolo
    lcdWriteNibble( addr,byte & 0x0F, instr0char1 );        // depois o LSNibble
}
void lcdCursor(uint8_t addr,uint8_t cPosit)                        // Funcao que recebe do usuï¿½rio a posiï¿½ï¿½o desejada do cursor
{
    lcdCmd(addr, BIT7 | cPosit , INSTR);                   // A posiï¿½ï¿½o do cursor ï¿½ alterada por meio dessa operaï¿½ï¿½o
}                                                     // com RS=0 e o bit mais significativo (BIT7) em alto
                                                      // Ou seja, se queremos a posiï¿½ï¿½o 0x41:
                                                      // Uma palavra = 2 bytes = 0x68 = 1100 1000
                                                      // Uma palavra = 2 bytes = 0x6C = 1100 1100
                                                      // Uma palavra = 2 bytes = 0x68 = 1100 1000

                                                      // Uma palavra = 2 bytes = 0x98 = 1001 1000
                                                      // Uma palavra = 2 bytes = 0x9C = 1001 1100
                                                      // Uma palavra = 2 bytes = 0x98 = 1001 1000

// Visor do LCD: 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F 0x10 0x11 0x12 0x13
//               0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F 0x50 0x51 0x52 0x53
//               0x14 0x15 0x16 0x17 0x18 0x19 0x1A 0x1B 0x1C 0x1D 0x1E 0x1F 0x20 0x21 0x22 0x23 0x24 0x25 0x26 0x27
//               0x54 0x55 0x56 0x57 0x58 0x59 0x5A 0x5B 0x5C 0x5D 0x5E 0x5F 0x60 0x61 0x62 0x63 0x64 0x65 0x66 0x67


void lcdTimer(uint8_t addr,tanque numero, processo tempo)
{
    if ( numero == T1)                      // T1
    {
        if(tempo == compostTea)
        {
        }
        else if(tempo == EVAP)
        {
//            lcdCursor(addr, 0x48);                //
//                    //XXXXXXXXXXXXXXXX
//            lcdNumbers_Hour(addr, t1hora);        // Eh dentro de
//            lcdCursor(addr, 0x4A);
//            lcdPrint(addr, "h");
//            lcdNumbers_Minute(addr, t1min);
        }
        else if(tempo == regandoTempo)
        {
        }
        else if(tempo == estimativaTempo)
        {
        }
    }
    if ( numero == T2)                  // T1
    {
        if(tempo == compostTea)
        {
        }
        else if(tempo == EVAP)
        {
//            lcdCursor(addr, 0x4f);                    //
//                    //XXXXXXXXXXXXXXXX
//            lcdNumbers_Hour(addr, t2hora);
//            lcdCursor(addr, 0x51);
//            lcdPrint(addr, "h");
//            lcdNumbers_Minute(addr, t2min);
        }
        else if(tempo == regandoTempo)
        {
        }
        else if(tempo == estimativaTempo)
        {
        }
    }
}
//----------------------------------------------------------------------------------------------
//++++++++++++++++++++++++++++++==
int operador_LCD(uint8_t addr)
{
//////////////////////////////////////////////////
// Visor do LCD: 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F 0x10 0x11 0x12 0x13
//               0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F 0x50 0x51 0x52 0x53
//               0x14 0x15 0x16 0x17 0x18 0x19 0x1A 0x1B 0x1C 0x1D 0x1E 0x1F 0x20 0x21 0x22 0x23 0x24 0x25 0x26 0x27
//               0x54 0x55 0x56 0x57 0x58 0x59 0x5A 0x5B 0x5C 0x5D 0x5E 0x5F 0x60 0x61 0x62 0x63 0x64 0x65 0x66 0x67

// S   I   R   -   V   6   -   -   T   Q   1   -   -   |   -   T   Q   2   -   -
//x00 x01 x02 x03 x04 x05 x06 x07 x08 x09 x0A x0B x0C x0D x0E x0F x10 x11 x12 x13
// E   S   T   A   D   O   :   -   X   X   X   X   -   |   -   X   X   X   X   -
//x40 x41 x42 x43 x44 x45 x46 x47 x48 x49 x4A x4B x4C x4D x4E x4F x50 x51 x52 x53
// N   I   V   E   L   -   :   -   X   X   X   X   -   |   -   X   X   X   X   -
//x14 x15 x16 x17 x18 x19 x1A x1B x1C x1D x1E x1F x20 x21 x22 x23 x24 x25 x26 x27
// T   E   M   P   -   -   :   -   X   X   X   X   -   |   -   X   X   X   X   -
//x54 x55 x56 x57 x58 x59 x5A x5B x5C x5D x5E x5F x60 x61 x62 x63 x64 x65 x66 x67
//--------------------------------------------------------------------------------------------------
// Contador ativado para quando T1 esta enchendo, serve para que o LCD nï¿½o seja atualiziado a to do momento, economizando energia
//--------------------------------------------------------------------------------------------------
    if(atualizarLCD == 1)
    {
        // lcd_T1Tool = 1 --> T1 enchendo
                // "evap_T1" (lcd_T1Tool=2) --> Timer de evaporacao
                // lcd_T1Tool = 3 --> Evap OK/Pronto para receber nutrientes
                // lcd_T1Tool = 4 --> T1 regando
                // lcd_T1Tool = 5 --> T1 vazio
                // lcd_T1Tool = 6 --> Erro no T1
        if(lcd_T1Tool == 1)                 // lcd_T1Tool =  flag para o tanque 1 enchendo
        {
            lcdWriteByte(addr, 0x02, INSTR);          // Return home
            lcdCursor(addr, 0x48);                    // Pular para linha de cima
                    //XXXXXXXXXXXXXXXX
            lcdPrint(addr, "Ench");                   // Funcao para escrever caracteres no LCD
    //        stringToEsp32[0] = '1';
    //        i2cWriteByte_UCB0(0x42, 3);
    //        i2cWriteByte_UCB0(0x42, 1);
    //       uartWrite_UCA0("11");
            t2_regar = 1;
        }
    //-------------------------------------------------
    //Informar da evaporacao do tanque 1
    //-------------------------------------------------
        else if(lcd_T1Tool == 2 )                        // Se a flag "evap_T1" esta levantada, entra na contagem para atualizaï¿½ï¿½o do estado
        {
            lcdWriteByte(addr, 0x02, INSTR);          // Return home
            lcdCursor(addr, 0x48);                    // Pular para linha de cima
                    //XXXXXXXXXXXXXXXX
                lcdTimer(addr, T1, EVAP);
                stringToEsp32[0] = '2';
     //           i2cWriteByte_UCB0(0x42, 3);
      //          i2cWriteByte_UCB0(0x42, 2);
       //         uartWrite_UCA0("12");
        }
    //--------------------------------------------------------------------------------------------------
    // Contador ativado para indicar no LCD que evap 1 ok ( lcd_T1Tool = 3;)
    //--------------------------------------------------------------------------------------------------
        else if(lcd_T1Tool == 3)
        {
            lcdCursor(addr, 0x48);                    // Pular para linha de cima
                    //XXXXXXXXXXXXXXXX
            lcdPrint(addr, " OK   ");

       //         stringToEsp32[0] = '3';     // Dizer que esta diponivel
       //     i2cWriteByte_UCB0(0x42, 3);
       //         i2cWriteByte_UCB0(0x42, 3);
        //    uartWrite_UCA0("13");                       // Comunicacao com o ESP32
        }
    //-------------------------------------------------
    //Contador da rega do tanque 1
    //-------------------------------------------------
        else if(lcd_T1Tool == 4)
        {
            lcdCursor(addr, 0x48);                    // Pular para linha de cima
                    //XXXXXXXXXXXXXXXX
            lcdPrint(addr, "REG ");     // Funcao para escrever caracteres no LCD
      //      stringToEsp32[0] = '4';
      //      i2cWriteByte_UCB0(0x42, 3);
      //      i2cWriteByte_UCB0(0x42, 4);
       //     uartWrite_UCA0("14");                       // Comunicacao com o ESP32
        }
        else if(lcd_T1Tool == 5)
        {
            lcdCursor(addr, 0x48);                    // Pular para linha de cima
                    //XXXXXXXXXXXXXXXX
            lcdPrint(addr, "VAZI");     // Funcao para escrever caracteres no LCD
       //     stringToEsp32[0] = '5';
       //     i2cWriteByte_UCB0(0x42, 3);
       ///     i2cWriteByte_UCB0(0x42, 5);
        //    uartWrite_UCA0("15");                       // Comunicacao com o ESP32
        }
        else if(lcd_T1Tool == 6)
        {
            lcdCursor(addr, 0x48);                    // Pular para linha de cima
                    //XXXXXXXXXXXXXXXX
            lcdPrint(addr, "ERRO");     // Funcao para escrever caracteres no LCD
        //    stringToEsp32[0] = '6';
       //     i2cWriteByte_UCB0(0x42, 3);
       //     i2cWriteByte_UCB0(0x42, 6);
       //     uartWrite_UCA0("16");                       // Comunicacao com o ESP32
        }
      //  uartWrite_UCA0(stringToEsp32);
        //--------------------------------------------------------------------------------------------------
        // Contador ativado quando T2 esta enchendo, serve para periodicamente informar o nivel no visor LCD
        //--------------------------------------------------------------------------------------------------
        if(lcd_T2Tool == 1)
        {
            lcdCursor(addr, 0x4F);                    // Pular para linha de baixo
                    //XXXXXXXXXXXXXXXX
            lcdPrint(addr, "Ench");                   // Funcao para escrever caracteres no LCD
            t1_regar = 1;
       //     stringToEsp32[1] = '1';
       //     i2cWriteByte_UCB0(0x42, 4);
       //     i2cWriteByte_UCB0(0x42, 1);
        //    uartWrite_UCA0("21");                       // Comunicacao com o ESP32
        }

    //-------------------------------------------------
    //Contador da evaporacao do tanque 2
    //-------------------------------------------------
        else if(lcd_T2Tool == 2)
        {
            lcdCursor(addr, 0x4f);                    // Pular para linha de baixo
                    //XXXXXXXXXXXXXXXX
            lcdTimer(addr, T2, EVAP);
       //     stringToEsp32[1] = '2';
       //     i2cWriteByte_UCB0(0x42, 4);
       //     i2cWriteByte_UCB0(0x42, 2);
        //    uartWrite_UCA0("22");                       // Comunicacao com o ESP32
        }
    //--------------------------------------------------------------------------------------------------
    // Contador ativado para indicar no LCD que evap 2 ok ( lcd_T2Tool = 3;)
    //--------------------------------------------------------------------------------------------------
        else if(lcd_T2Tool == 3)
        {
            lcdCursor(addr, 0x4f);                    // Pular para linha de baixo
                    //XXXXXXXXXXXXXXXX
            lcdPrint(addr, " OK  ");

      //          stringToEsp32[1] = '3';     // Dizer que esta diponivel
      //      i2cWriteByte_UCB0(0x42, 4);
        //        i2cWriteByte_UCB0(0x42, 3);

        //    uartWrite_UCA0("23");                       // Comunicacao com o ESP32
        }
    //-------------------------------------------------
    //Contador da rega do tanque 2
    //-------------------------------------------------
        else if(lcd_T2Tool == 4)                     //
        {
            lcdCursor(addr, 0x4F);                    // Pular para linha de baixo
                    //XXXXXXXXXXXXXXXX
            lcdPrint(addr, "REG ");     // Funcao para escrever caracteres no LCD
     //       stringToEsp32[1] = '4';
     //       i2cWriteByte_UCB0(0x42, 4);
     //       i2cWriteByte_UCB0(0x42, 4);
      //      uartWrite_UCA0("24");                       // Comunicacao com o ESP32
        }
        else if(lcd_T2Tool == 5)                     //
        {
            lcdCursor(addr, 0x4F);                    // Pular para linha de baixo
                    //XXXXXXXXXXXXXXXX
            lcdPrint(addr, "VAZI");     // Funcao para escrever caracteres no LCD
     //       stringToEsp32[1] = '5';
     //       i2cWriteByte_UCB0(0x42, 4);
     //       i2cWriteByte_UCB0(0x42, 5);
      //      uartWrite_UCA0("25");                       // Comunicacao com o ESP32
        }
        else if(lcd_T2Tool == 6)                     //
        {
            lcdCursor(addr, 0x4F);                    // Pular para linha de baixo
                    //XXXXXXXXXXXXXXXX
            lcdPrint(addr, "ERRO");     // Funcao para escrever caracteres no LCD
      //      stringToEsp32[1] = '6';
    //        i2cWriteByte_UCB0(0x42, 4);
    //        i2cWriteByte_UCB0(0x42, 6);
        }
      //  uartWrite_UCA0("15");                       // Teste comunicacao com o ESP32
      //  uartWrite_UCA0(stringToEsp32);
       // uartWrite_UCA0("\n");
        flag_LCD = 0;
    }
    return 0;
}
//-----------------------------------------------
void lcdLayout(uint8_t addr,mostrarNoLCD layout)
{
    // Visor do LCD: 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F 0x10 0x11 0x12 0x13
    //               0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F 0x50 0x51 0x52 0x53
    //               0x14 0x15 0x16 0x17 0x18 0x19 0x1A 0x1B 0x1C 0x1D 0x1E 0x1F 0x20 0x21 0x22 0x23 0x24 0x25 0x26 0x27
    //               0x54 0x55 0x56 0x57 0x58 0x59 0x5A 0x5B 0x5C 0x5D 0x5E 0x5F 0x60 0x61 0x62 0x63 0x64 0x65 0x66 0x67
    if( layout == defaut)
    {
        lcdWriteByte(addr, 0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr, 0x00);                  // Pular para linha 0x14

        //              XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr, "  Ambar TQ1  | TQ2  ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr, 0x40);                      // Mudar posicao do cursor

        lcdPrint(addr, "ESTADO: ---- | ---- ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr, 0x14);                      // Mudar posicao do cursor

        lcdPrint(addr, "NIVEL : -- L | -- L ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr, "TEMP. :             ");     // Funcao para escrever caracteres no LCD

    }
    else if( layout == initPrep)
    {
        // Visor do LCD: 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F
        //               0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F
        //             XXXXXXXXXXXXXXXX
        lcdCursor(addr,0x00);
        lcdPrint(addr,"   Preparando   ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                    // Mudar posicao do cursor

        lcdPrint(addr," adubo liquido  ");   // Funcao para escrever caracteres no LCD
    }
    else if( layout == prepOk)
    {
        // Visor do LCD: 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F
        //               0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F
        //             XXXXXXXXXXXXXXXX
        lcdCursor(addr,0x00);
        lcdPrint(addr,"  Adubo liquido ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                    // Mudar posicao do cursor

        lcdPrint(addr,"     pronto     ");   // Funcao para escrever caracteres no LCD
    }
    else if( layout == prepSend)
    {
        // Visor do LCD: 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F
        //               0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F
        //             XXXXXXXXXXXXXXXX
        lcdCursor(addr,0x00);
        lcdPrint(addr,"  Adubo liquido ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                    // Mudar posicao do cursor

        lcdPrint(addr,"     enviado    ");   // Funcao para escrever caracteres no LCD
    }
    else if( layout == menucmd)
    {
        lcdWriteByte(addr, 0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr, " Testes, digite o   ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                    // Mudar posicao do cursor

        lcdPrint(addr, "  comando + ENTER   ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                    // Mudar posicao do cursor

        lcdPrint(addr," ou  ESC para sair  ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                    // Mudar posicao do cursor

        lcdPrint(addr, ":                   ");   // Funcao para escrever caracteres no LCD
    }
    else if( layout == senhacentral)
    {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr,"   Digite a senha   ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                    // Mudar posicao do cursor

        lcdPrint(addr,"  + ENTER, ou ESC   ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                    // Mudar posicao do cursor

        lcdPrint(addr,"     para sair:      ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                    // Mudar posicao do cursor

        lcdPrint(addr,"                     ");   // Funcao para escrever caracteres no LCD
    }
    else if( layout == senhaincorreta)
    {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr,"  SENHA INCORRETA   ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                    // Mudar posicao do cursor

        lcdPrint(addr," tente novamente ou  ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                    // Mudar posicao do cursor

        lcdPrint(addr,"  ESC para sair  ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                    // Mudar posicao do cursor

        lcdPrint(addr," :                   ");   // Funcao para escrever caracteres no LCD
        lcdCursor(addr,0x57);                    // Mudar posicao do cursor
    }
    else if( layout == errodecmd)
    {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr,"  Erro de comando   ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                    // Mudar posicao do cursor

        lcdPrint(addr," tente novamente ou ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                    // Mudar posicao do cursor

        lcdPrint(addr,"   ESC para sair    ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                    // Mudar posicao do cursor

        lcdPrint(addr," :                  ");   // Funcao para escrever caracteres no LCD
        lcdCursor(addr,0x57);                    // Mudar posicao do cursor
    }
    else if( layout == confirmacaocmd)
    {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x04);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        rx.read = 0;                           // Coloca o ponteiro circular no inicio kkkkkk
        // Dentro do while abaixo eh escrito no LCD o comando solicitado, esse comando esta armazenado no buffer do UART
        while(rx.size)
        {
            uint8_t i;
            uint8_t rxReadMem = rx.read;
            for (i = rx.size; i == 0; i--)
            {
                if(rx.buffer[rx.read] >= 'a' && rx.buffer[rx.read] <= 'z')
                {
                    rx.buffer[rx.read] = rx.buffer[rx.read] - 32;
                }
                rx.read++;
                rx.read &= 0x0F;
            }
            rx.read = rxReadMem;
            lcdWriteByte(addr,rx.buffer[rx.read], CHAR);    // Escreve no LCD letra por letra digitada
            rx.read++;
            rx.read &= 0x0F;
            rx.size--;
        }

        lcdCursor(addr,0x40);                    // Mudar posicao do cursor

        lcdPrint(addr," Pressione ENTER    ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                    // Mudar posicao do cursor

        lcdPrint(addr," para confirmar  a  ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                    // Mudar posicao do cursor

        lcdPrint(addr,"       acao        ");   // Funcao para escrever caracteres no LCD
    }
    else if(layout == v1inabrindo)
        {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr," Abrindo entrada do ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                    // Mudar posicao do cursor

        lcdPrint(addr,"      Tanque 1      ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                    // Mudar posicao do cursor

        lcdPrint(addr," Corrente:          ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                    // Mudar posicao do cursor

        lcdPrint(addr," Consumo:           ");   // Funcao para escrever caracteres no LCD

        }
    else if(layout == v1infechando)
        {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr," Fechando entrada do");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                    // Mudar posicao do cursor

        lcdPrint(addr,"      Tanque 1      ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                    // Mudar posicao do cursor

        lcdPrint(addr," Corrente:          ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                    // Mudar posicao do cursor

        lcdPrint(addr," Consumo:           ");   // Funcao para escrever caracteres no LCD

        }
    else if(layout == v1outabrindo)
        {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr," Abrindo saida do   ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                    // Mudar posicao do cursor

        lcdPrint(addr,"      Tanque 1      ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                    // Mudar posicao do cursor

        lcdPrint(addr," Corrente:          ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                    // Mudar posicao do cursor

        lcdPrint(addr," Consumo:           ");     // Funcao para escrever caracteres no LCD

        }
    else if(layout == v1outfechando)
        {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr," Fechando saida do  ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                    // Mudar posicao do cursor

        lcdPrint(addr,"      Tanque 1      ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                    // Mudar posicao do cursor

        lcdPrint(addr," Corrente:          ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                    // Mudar posicao do cursor

        lcdPrint(addr," Consumo:           ");   // Funcao para escrever caracteres no LCD

        }
    else if(layout == v2inabrindo)
        {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                  // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr," Abrindo entrada do ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                    // Mudar posicao do cursor

        lcdPrint(addr,"      Tanque 2      ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                    // Mudar posicao do cursor

        lcdPrint(addr," Corrente:          ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                    // Mudar posicao do cursor

        lcdPrint(addr," Consumo:             ");   // Funcao para escrever caracteres no LCD

        }
    else if(layout == v2infechando)
        {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr," Fechando entrada do");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                    // Mudar posicao do cursor

        lcdPrint(addr,"      Tanque 2      ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                    // Mudar posicao do cursor

        lcdPrint(addr," Corrente:          ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                    // Mudar posicao do cursor

        lcdPrint(addr," Consumo:           ");   // Funcao para escrever caracteres no LCD

        }
    else if(layout == v2outabrindo)
        {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr," Abrindo saida do   ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                    // Mudar posicao do cursor

        lcdPrint(addr,"      Tanque 2      ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                    // Mudar posicao do cursor

        lcdPrint(addr," Corrente:          ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                    // Mudar posicao do cursor

        lcdPrint(addr," Consumo:           ");   // Funcao para escrever caracteres no LCD

        }
    else if(layout == v2outfechando)
        {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr," Fechando saida do  ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                    // Mudar posicao do cursor

        lcdPrint(addr,"      Tanque 2      ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                    // Mudar posicao do cursor

        lcdPrint(addr," Corrente:          ");   // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                    // Mudar posicao do cursor

        lcdPrint(addr," Consumo:           ");   // Funcao para escrever caracteres no LCD

        }
    else if(layout == erroT1)
        {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr,"     AMBAR V6.1     ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                      // Mudar posicao do cursor

        lcdPrint(addr,"      ERRO NO       ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                      // Mudar posicao do cursor

        lcdPrint(addr,"      Tanque 1      ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"                    ");     // Funcao para escrever caracteres no LCD
        wait(5, sec);
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                  // Pular para linha 0x14

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr,"SIR V4  TQ1  | TQ2  ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                      // Mudar posicao do cursor

        lcdPrint(addr,"ESTADO: ERRO | ---- ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                      // Mudar posicao do cursor

        lcdPrint(addr,"NIVEL : ---- | ---- ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"TEMP. : ---- | ---- ");     // Funcao para escrever caracteres no LCD

        }
    else if(layout == erroT2)
        {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr,"     AMBAR V6.1     ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                      // Mudar posicao do cursor

        lcdPrint(addr,"      ERRO NO       ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                      // Mudar posicao do cursor

        lcdPrint(addr,"      Tanque 2      ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"                    ");     // Funcao para escrever caracteres no LCD

        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00
        wait(5, sec);
        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr,"SIR V4  TQ1  | TQ2  ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                      // Mudar posicao do cursor

        lcdPrint(addr,"ESTADO: ---- | ERRO ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                      // Mudar posicao do cursor

        lcdPrint(addr,"NIVEL : ---- | ---- ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"TEMP. : ---- | ---- ");     // Funcao para escrever caracteres no LCD

        }
    else if(layout == erroT1T2)
        {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr,"     AMBAR V6.1     ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                      // Mudar posicao do cursor

        lcdPrint(addr,"      ERRO NOS      ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                      // Mudar posicao do cursor

        lcdPrint(addr,"    Tanques 1 e 2   ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"                    ");     // Funcao para escrever caracteres no LCD

        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00
        wait(5, sec);
        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr,"SIR V4  TQ1  | TQ2  ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                      // Mudar posicao do cursor

        lcdPrint(addr,"ESTADO: ERRO | ERRO ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                      // Mudar posicao do cursor

        lcdPrint(addr,"NIVEL : ---- | ---- ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"TEMP. : ---- | ---- ");     // Funcao para escrever caracteres no LCD

        }
    else if(layout == menu)
    {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr,"  PASSE PELO MENU   ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                      // Mudar posicao do cursor

        lcdPrint(addr," POR MEIO DE TOQUES ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                      // Mudar posicao do cursor

        lcdPrint(addr,"  CURTOS NO BOTAO   ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"                     ");     // Funcao para escrever caracteres no LCD

    }
    else if(layout == opcaoinstrucomposttea)
    {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr,"5-  ESCOLHA ESSA    ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                      // Mudar posicao do cursor

        lcdPrint(addr,"OPCAO PARA RECEBER");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                      // Mudar posicao do cursor

        lcdPrint(addr," AS INSTRUCOES DO ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr," CHA DE COMPOSTO  ");     // Funcao para escrever caracteres no LCD

    }
    else if(layout == instrucomposttea)
    {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr,"   INSTRUCOES:      ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                      // Mudar posicao do cursor

        lcdPrint(addr,"ADICIONE AGUA LIMPA ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                      // Mudar posicao do cursor

        lcdPrint(addr,"ATE ATINGIR A MARCA ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"LIMITE DO RECIPIENTE");     // Funcao para escrever caracteres no LCD
        wait(7, sec);
        lcdPrint(addr," NO LOCAL INDICADO, ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                      // Mudar posicao do cursor

        lcdPrint(addr,"COLOQUE 10G DE ACIDO");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                      // Mudar posicao do cursor

        lcdPrint(addr,"       HUMICO.      ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"                    ");     // Funcao para escrever caracteres no LCD
        wait(7, sec);
        lcdPrint(addr," NO LOCAL INDICADO, ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                      // Mudar posicao do cursor

        lcdPrint(addr,"  COLOQUE 5 LITROS  ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                      // Mudar posicao do cursor

        lcdPrint(addr,"    DE COMPOSTO.    ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"                    ");     // Funcao para escrever caracteres no LCD
        wait(7, sec);
        lcdPrint(addr,"TERMINADO O PROCESSO");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                      // Mudar posicao do cursor

        lcdPrint(addr," ESCOLHA NO MENU A  ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                      // Mudar posicao do cursor

        lcdPrint(addr,"   OPCAO 'CHA DE    ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"    COMPOSTAGEM'    ");     // Funcao para escrever caracteres no LCD
        //wait(7, sec);
    }
    else if(layout == opcaocomposttea)
       {
           lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

           lcdCursor(addr,0x00);                    // Pular para linha 0x00

           //        XXXXXXXXXXXXXXXXXXXX
           lcdPrint(addr,"6-CHA DE COMPOSTAGEM");     // Funcao para escrever caracteres no LCD

           lcdCursor(addr,0x40);                      // Mudar posicao do cursor

           lcdPrint(addr,"  PARA DAR INICIO,  ");     // Funcao para escrever caracteres no LCD

           lcdCursor(addr,0x14);                      // Mudar posicao do cursor

           lcdPrint(addr,"PROSSIGA COM DUPLO ");     // Funcao para escrever caracteres no LCD

           lcdCursor(addr,0x54);                      // Mudar posicao do cursor

           lcdPrint(addr,"  TOQUE NO BOTAO   ");     // Funcao para escrever caracteres no LCD

       }
    else if(layout == composttea)
       {
           lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

           lcdCursor(addr,0x00);                    // Pular para linha 0x00

           //        XXXXXXXXXXXXXXXXXXXX
           lcdPrint(addr," CHA DE COMPOSTAGEM ");     // Funcao para escrever caracteres no LCD

           lcdCursor(addr,0x40);                      // Mudar posicao do cursor

           lcdPrint(addr,"VERIFIQUE SE COLOCOU");     // Funcao para escrever caracteres no LCD

           lcdCursor(addr,0x14);                      // Mudar posicao do cursor

           lcdPrint(addr,"   OS INGREDIENTES  ");     // Funcao para escrever caracteres no LCD

           lcdCursor(addr,0x54);                      // Mudar posicao do cursor

           lcdPrint(addr,"      E CONFIRME     ");     // Funcao para escrever caracteres no LCD
           //wait(1, sec);
       }
    else if(layout == iniciocomposttea)
       {
           lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

           lcdCursor(addr,0x00);                    // Pular para linha 0x00

           //        XXXXXXXXXXXXXXXXXXXX
           lcdPrint(addr," CHA DE COMPOSTAGEM ");     // Funcao para escrever caracteres no LCD

           lcdCursor(addr,0x40);                      // Mudar posicao do cursor

           lcdPrint(addr,"      INICIADO      ");     // Funcao para escrever caracteres no LCD

           lcdCursor(addr,0x14);                      // Mudar posicao do cursor

           lcdPrint(addr," VOLTE A ESSE MENU  ");     // Funcao para escrever caracteres no LCD

           lcdCursor(addr,0x54);                      // Mudar posicao do cursor

           lcdPrint(addr,"PARAR VER O RELOGIO ");     // Funcao para escrever caracteres no LCD

       }
    else if(layout == instrufertirrigacao)
    {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr,"   PARA COMECAR O   ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                      // Mudar posicao do cursor

        lcdPrint(addr," PROCESSO, PROSSIGA");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                      // Mudar posicao do cursor

        lcdPrint(addr,"   COM UM DUPLO    ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"   TOQUE NO BOTAO  ");     // Funcao para escrever caracteres no LCD

    }
    else if(layout == opcaofertirrigacao)
    {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr,"1- FERTIRRIGACAO:   ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                      // Mudar posicao do cursor

        lcdPrint(addr," TENHA ADICIONADO O ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                      // Mudar posicao do cursor

        lcdPrint(addr,"NUTRIENTE E ESCOLHA");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"     ESSA OPCAO    ");     // Funcao para escrever caracteres no LCD

    }
    else if(layout == pumpNutOn)
    {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

             //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr,"Enviando nutrientes ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                      // Mudar posicao do cursor

        lcdPrint(addr," ao tanque principal");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                      // Mudar posicao do cursor

        lcdPrint(addr,"                    ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"                    ");     // Funcao para escrever caracteres no LCD

    }
    else if(layout == iniciopferti)
    {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr," PROCESSO INICIADO  ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                      // Mudar posicao do cursor

        lcdPrint(addr," >--  IRRIGACAO >-- ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                      // Mudar posicao do cursor

        lcdPrint(addr," --< BOMBEAMENTO --<");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr," >--  AERACAO    >--");     // Funcao para escrever caracteres no LCD

        //        XXXXXXXXXXXXXXXXXXXX
        // Visor do LCD: 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F 0x10 0x11 0x12 0x13
        //               0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F 0x50 0x51 0x52 0x53
        //               0x14 0x15 0x16 0x17 0x18 0x19 0x1A 0x1B 0x1C 0x1D 0x1E 0x1F 0x20 0x21 0x22 0x23 0x24 0x25 0x26 0x27
        //               0x54 0x55 0x56 0x57 0x58 0x59 0x5A 0x5B 0x5C 0x5D 0x5E 0x5F 0x60 0x61 0x62 0x63 0x64 0x65 0x66 0x67
       // PrÃ³ximo movimento das setas
      wait(100, ms);
       lcdCursor(addr,0x41);

       lcdPrint(addr,"->-");     // Funcao para escrever caracteres no LCD

       lcdCursor(addr,0x50);

       lcdPrint(addr,"->-");     // Funcao para escrever caracteres no LCD

       lcdCursor(addr,0x15);                      // Mudar posicao do cursor

       lcdPrint(addr,"-<-");     // Funcao para escrever caracteres no LCD

       lcdCursor(addr,0x25);                      // Mudar posicao do cursor

       lcdPrint(addr,"-<-");     // Funcao para escrever caracteres no LCD

       lcdCursor(addr,0x55);                      // Mudar posicao do cursor

       lcdPrint(addr,"->-");     // Funcao para escrever caracteres no LCD

       lcdCursor(addr,0x65);                          // Mudar posicao do cursor

       lcdPrint(addr,"->-");     // Funcao para escrever caracteres no LCD
        // PrÃ³ximo movimento das setas
       wait(700, ms);
        lcdCursor(addr,0x41);

        lcdPrint(addr,"-->");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x50);

        lcdPrint(addr,"-->");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x15);                      // Mudar posicao do cursor

        lcdPrint(addr,"^--");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x25);                      // Mudar posicao do cursor

        lcdPrint(addr,"<--");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x55);                      // Mudar posicao do cursor

        lcdPrint(addr,"-->");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x65);                          // Mudar posicao do cursor

        lcdPrint(addr,"--^");     // Funcao para escrever caracteres no LCD
        // PrÃ³ximo movimento das setas
       wait(700, ms);
        lcdCursor(addr,0x41);

        lcdPrint(addr,">--");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x50);
        lcdPrint(addr,">--");     // Funcao para escrever caracteres no LCD
        lcdCursor(addr,0x15);                      // Mudar posicao do cursor

        lcdPrint(addr,"--<");     // Funcao para escrever caracteres no LCD
        lcdCursor(addr,0x25);                      // Mudar posicao do cursor

        lcdPrint(addr,"--<");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x55);                      // Mudar posicao do cursor

        lcdPrint(addr,">--");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x65);                          // Mudar posicao do cursor

        lcdPrint(addr,">--");     // Funcao para escrever caracteres no LCD
        // PrÃ³ximo movimento das setas
       wait(700, ms);

        lcdCursor(addr,0x41);

        lcdPrint(addr,"->-");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x50);
        lcdPrint(addr,"->-");     // Funcao para escrever caracteres no LCD
        lcdCursor(addr,0x15);                      // Mudar posicao do cursor

        lcdPrint(addr,"-<-");     // Funcao para escrever caracteres no LCD
        lcdCursor(addr,0x25);                      // Mudar posicao do cursor

        lcdPrint(addr,"-<-");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x55);                      // Mudar posicao do cursor

        lcdPrint(addr,"->-");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x65);                          // Mudar posicao do cursor

        lcdPrint(addr,"->-");     // Funcao para escrever caracteres no LCD

        // PrÃ³ximo movimento das setas
        wait(700, ms);
        lcdCursor(addr,0x41);

        lcdPrint(addr,"-->");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x50);
        lcdPrint(addr,"-->");     // Funcao para escrever caracteres no LCD
        lcdCursor(addr,0x15);                      // Mudar posicao do cursor

        lcdPrint(addr,"^--");     // Funcao para escrever caracteres no LCD
        lcdCursor(addr,0x25);                      // Mudar posicao do cursor

        lcdPrint(addr,"<--");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x55);                      // Mudar posicao do cursor

        lcdPrint(addr,"-->");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x65);                          // Mudar posicao do cursor

        lcdPrint(addr,"--^");     // Funcao para escrever caracteres no LCD

    }
    else if(layout == sensorestanque)
    {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr,"2- SENSORES TANQUE  ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                      // Mudar posicao do cursor

        lcdPrint(addr,"    PH: XXX         ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                      // Mudar posicao do cursor

        lcdPrint(addr," TEMP.: XXX C       ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"                    ");     // Funcao para escrever caracteres no LCD

    }
    else if(layout == noactsensorestanque)
    {
        //        XXXXXXXXXXXXXXXXXXXX
        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"     NO ACTION      ");     // Funcao para escrever caracteres no LCD

    }
    else if(layout == sensoresar)
    {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr,"4-  SENSORES  AR    ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                      // Mudar posicao do cursor

        lcdPrint(addr,"    TEMP.: XX C     ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                      // Mudar posicao do cursor

        lcdPrint(addr,"UR:XX% VENTO:XX km/h");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"                    ");     // Funcao para escrever caracteres no LCD

    }
    else if(layout == noactsensoresar)
    {
        //        XXXXXXXXXXXXXXXXXXXX
        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"     NO ACTION      ");     // Funcao para escrever caracteres no LCD

    }
    else if(layout == consumodeagua)
    {
        lcdWriteByte(addr,0x01, INSTR);          // Limpa o LCD

        lcdCursor(addr,0x00);                    // Pular para linha 0x00

        //        XXXXXXXXXXXXXXXXXXXX
        lcdPrint(addr,"3- CONSUMO DE AGUA  ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x40);                      // Mudar posicao do cursor

        lcdPrint(addr,"FLUXO:XXX Litros/min");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x14);                      // Mudar posicao do cursor

        lcdPrint(addr,"DIARIO: XXX Litros  ");     // Funcao para escrever caracteres no LCD

        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"                    ");     // Funcao para escrever caracteres no LCD

    }
    else if(layout == noactconsumodeagua)
    {
        //        XXXXXXXXXXXXXXXXXXXX
        lcdCursor(addr,0x54);                      // Mudar posicao do cursor

        lcdPrint(addr,"     NO ACTION      ");     // Funcao para escrever caracteres no LCD

    }
}




