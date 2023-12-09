/*
 * sensor_DS18B20.c
 *
 *  Created on: 28 de nov de 2021
 *      Author: guiwz
 */


#include    <msp430.h>
#include    <stdint.h>
#include    "libs/ambar_lcd_5529.h"
#include    "libs/funcoes_DS18B20_5529.h"
#include    "libs/config_timers_5529.h"

// Constantes para os atrasos
#define OW_RST1 88
#define OW_RST2 10
#define OW_WR   10
#define OW_RD   7
#define PCF8574_LCD1    0x27
#define PCF8574_LCD2    0x26

volatile unsigned int bit = 0;
volatile unsigned int byte_read = 0x00;
volatile unsigned int bit_read = 0x00;
volatile uint16_t  tempCelsius_int = 0;
volatile float    tempCelsius = 0;

void oneWireConfig()
{
    P8SEL  &=  ~BIT2;                       // I/O
    P8DIR  |=   BIT2;                       // Configurando pino P8.1 como entrada (Alta impedancia)
    P8OUT  &=  ~BIT2;                       // P8.1 = 0
    P8REN  &=  ~BIT2;                       // Pull Up/Down não habilitado, será com resistor externo
}

uint8_t read_temp()
{
    reset_1W();                       // 1- Wire reset
    //        if(ow_rst() == 0){
    //            led_VM();                     // LED avisa do erro
    //            while(1);                     // Trava execuÃ§Ã£o
    //        }
    writeByte_1W(0xCC);               // Skip ROM = CCh
    writeByte_1W(0x44);               // Convert T = 44h. This command begins a temperature conversion
    wait(1,ms);                     // Temperature conversion time mÃ¡x = 750 ms
    reset_1W();
    //writeByte_1W(0x33);             // Read ROM = 33h. This command allows the bus master to read the DS18B20â€™s

    // 8-bit family code, unique 48-bit serial number, and 8-bit CRC.
    // This command can only be used if there is a single DS18B20 on the bus
    writeByte_1W(0xCC);               // Skip ROM = CCh
    writeByte_1W(0xBE);               // Read Scratchpad = BEh. The temperature information
    wait(1,ms);                     // Temperature conversion time mÃ¡x = 750 ms
    // can be retrieved over the 1-Wire interface by issuing BEh
    readTemp_1W();                    // FunÃ§Ã£o para leitura da temperatura
    // A   m   b   a   r               T   Q   1           |       T   Q   2
    //x00 x01 x02 x03 x04 x05 x06 x07 x08 x09 x0A x0B x0C x0D x0E x0F x10 x11 x12 x13
    // E   S   T   A   D   O   :   -   X   X   X   X   -   |   -   X   X   X   X   -
    //x40 x41 x42 x43 x44 x45 x46 x47 x48 x49 x4A x4B x4C x4D x4E x4F x50 x51 x52 x53
    // N   I   V   E   L   -   :   -   X   X   X   X   -   |   -   X   X   X   X   -
    //x14 x15 x16 x17 x18 x19 x1A x1B x1C x1D x1E x1F x20 x21 x22 x23 x24 x25 x26 x27
    // T   E   M   P   .       :       X   X   X   X   C   |       X   X   X   X   C
    //x54 x55 x56 x57 x58 x59 x5A x5B x5C x5D x5E x5F x60 x61 x62 x63 x64 x65 x66 x67
    lcdCursor(PCF8574_LCD1, 0x5C);          // LCD na posiÃ§Ã£o 0x07
    lcdFloatNum(PCF8574_LCD1,tempCelsius, temp);       // Imprime o conteÃºdo da variÃ¡ael
    lcdCursor(PCF8574_LCD1,0x60);
    lcdPrint(PCF8574_LCD1," C ");
    return 1;
}

// RELEASE LINE

// LINE PULLLOW

// ONE WIRE (OW) RESET
uint8_t reset_1W()
{
    char i,aux;
    ow_pin_low();
    for(i=0; i<OW_RST1;i++) __no_operation();
    ow_pin_hiz();
    for(i=0; i<OW_RST2; i++) __no_operation();
    aux = ow_pin_rd();
    for(i=0; i<(OW_RST1-OW_RST2); i++);  __no_operation();
    return aux^BIT0;
}
// Operações de 1 bit - Escrever 1
void    ow_wr_1(void)
{
    char i;
    ow_pin_low();                       // Drives DQ low
    ow_pin_hiz();                       // Releases the bus. O tempo de linha abaixada na escrita do 1 eh tao pequeno que n precisa nem segurar o processador
    for(i=0; i<OW_WR; i++) __no_operation();
}

// Escrever zero
void    ow_wr_0(void)
{
    char i;
    ow_pin_low();                       // Drives DQ low
    for(i=0; i<OW_WR; i++) __no_operation();
    ow_pin_hiz();                       // Releases the bus
    __no_operation();
}

uint8_t    readBit_1W()
{
    char i;
    ow_pin_low();                           // Drives DQ low
    ow_pin_hiz();
    bit_read = ow_pin_rd();                         // Ler barramento 1-Wire
    for(i=0; i<OW_RD; i++)  __no_operation();
    return bit_read;

}

// Operações de vários bits
void    writeByte_1W(uint8_t value)
{
    char i;
    for(i=0; i<8; i++){
        if((value&BIT0) == 0) ow_wr_0();
        else                  ow_wr_1();
        value = value >> 1;
    }
}
uint8_t readByte_1W()
{
    char i;
    for(i=0; i<8; i++){
        byte_read = byte_read>>1;
        if(readBit_1W() == BIT0)     byte_read |= 0x80;
    }
    return byte_read;
}
float readTemp_1W()
{
    volatile static uint16_t  LSB_temp  = 0x00;
    volatile static uint16_t  MSB_temp  = 0x00;
    volatile static uint16_t temp      = 0x0000;
//    volatile static uint16_t  tempCelsius_int = 0;
//    volatile float    tempCelsius = 0;

    readByte_1W();                          // Leitura do primeiro byte enviado pelo sensor
    LSB_temp = byte_read;                   // Armazena o primeiro byte
    readByte_1W();                          // Leitura do segundo byte enviado pelo sensor
    MSB_temp = byte_read;                   // Armazena o segundo byte
    MSB_temp = MSB_temp << 8;               // Aloca corretamente os 8 bits do MSB da leitura de temperatura
    temp = MSB_temp + LSB_temp;
    tempCelsius = temp * 0.0625;            // Valor que o datasheet manda multiplicar para converter a medida para celcius
    //tempCelsius_int = tempCelsius*100;    //
    return tempCelsius;                     // Funcao retorna o valor da temperatura em float
}

//********************************** utilizando o clock modificado **********************************
///*
// * sensor_DS18B20.c
// *
// *  Created on: 28 de nov de 2021
// *      Author: guiwz
// */
//
//#include    <msp430.h>
//#include    <stdint.h>
//#include    "timer.h"
//#include    "lcd.h"
//
//volatile unsigned int bit = 0;
//volatile unsigned int Byte_Read = 0x00;
//volatile unsigned int LSB_temp  = 0x00;
//volatile unsigned int MSB_temp  = 0x00;
//volatile unsigned int temp      = 0x0000;
//volatile float        tempCelsius = 0;
//volatile int          tempCelsius_int = 0;
//
//
//void OneWireConfig()
//{
//    P1SEL  &=  ~BIT2;                       // I/O
//    P1DIR  &=  ~BIT2;                       //Configurando pino P1.2 como entrada
//    P1REN  &=  ~BIT2;                       // Pull Up/Down não habilitado, será com resistor externo
//
//}
//
////void release_1W(uint8t port, uint8t bit)
//void release_1W(void)
//{
//    P1SEL  &=  ~BIT2;                       // I/O
//    P1DIR  &=  ~BIT2;                       //Configurando pino P1.2 como entrada
//    P1REN  &=  ~BIT2;                       // Pull Up/Down não habilitado, será com resistor externo
//
//}
////void pullLow_1W(uint8t port, uint8t bit)
//void pullLow_1W(void)
//{
//    P1SEL   &= ~BIT2;                        // P1.2 (I/O)
//    P1DIR   |=  BIT2;                        // Saída
//    P1OUT   &= ~BIT2;                        // Saída em LOW
//}
//uint8_t    reset_1W()
//{
//    int result;
//        pullLow_1W();                       // Drives DQ low
//        __delay_cycles(6000);               // Tentando aproximar de 480 us,
//        __delay_cycles(500);               // Tentando aproximar de 480 us,
//                                            // fiz uma regra de três para me basear no tempo de escrever 1, porque ele funcionou
//        //wait(1, sec);                    // 524 us. 480 us minimum, 960 us maximum
////                                          // Cada passo é de 9,5368e-7 s => 480 us = x * passo
//        release_1W();                       // Releases the bus
//        __delay_cycles(800);                // Tentando aproximar de 70 us
//        //wait(52, us);                     // 50 us. 15 us minimum, 60 us maximum
//        if(( P1IN & BIT2) == 0)
//        {
//            result = 1;
//        }
//        else
//        {
//            result = 0;
//        }
//        __delay_cycles(4805);               // Tentando aproximar de 410 us
//       // wait(410, us);                    // 500 us. Complete the reset sequence recovery
//        return result;                      // Return sample presence pulse result
//}
//// Operações de 1 bit
//void    writeBit_1W(uint8_t value)
//{
//    if(value)
//    {
//        pullLow_1W();                       // Drives DQ low
//        __delay_cycles(50);                        // Datasheet pede 6 us, para contar até 6, uC precisa de 6.291 passos.
//        release_1W();                       // Releases the bus
//        __delay_cycles(750);                       // Datasheet pede 64 us, para contar até 64, uC precisa de 67.108 passos.
//    }
//    else
//    {
//        pullLow_1W();                       // Drives DQ low
//        __delay_cycles(750);                // Datasheet pede 60 us, para contar até 60, uC precisa de 62.914 passos.
//        release_1W();                       // Releases the bus
//        __delay_cycles(70);                 // Datasheet pede 10 us, para contar até 60, uC precisa de 10.486 passos.
//    }
//}
//
//uint8_t    readBit_1W()
//{
//    //volatile int bit;                       // O "volatile" é para dizer ao compilador para não fazer nenhuma otimização na variável x.
//    pullLow_1W();                           // Drives DQ low
//    __delay_cycles(50);                     // Datasheet pede 6 us, para contar até 6, uC precisa de 6.291 passos.
//    release_1W();                           // Releases the bus
//    __delay_cycles(75);                     // Datasheet pede 9 us, para contar até 9, uC precisa de 9.437 passos.
//    if(( P1IN & BIT2) == 0)
//    {
//        bit = 0;
//    }
//    else
//    {
//        bit = 1;
//    }
//    __delay_cycles(700);                      // Datasheet pede 55 us, para contar até 55, uC precisa de 57.671 passos.
//    return bit;
//
//}
//
//// Operações de vários bits
//void    writeByte_1W(uint8_t value)
//{
//    uint8_t mask = 0x01;                    // Bit menos significante primeiro. LS-bit first
//    while(mask)
//    {
//        writeBit_1W(value & mask);
//        mask = mask << 1;                   // Deslocar a máscara de um bit para esquerda
//    }
//    //return writeBit_1W
//    __delay_cycles(1200);                 // Cerca de 100 us
//}
//uint8_t readByte_1W()
//{
//    uint8_t mask = 0x01;                    // Começa com a máscara apontando para o LSb
//    // 1000 0000
//    // 0100 0000
//    // 0010 0000
//    // 0001 0000
//    // 0000 1000
//    // 0000 0100
//    // 0000 0010
//    // 0000 0001
//    // 0000 0000 (sai do while)
//     Byte_Read = 0x00;
//    while(mask)                             // Recebendo bits, um a um
//    {                                       // Setando o valor apontado pela máscara
//        readBit_1W();
//        if(bit)                    // Se a leitura da função "readBit_1W()" for 1
//        {
//            Byte_Read |= mask;
//        }
//
//        mask = mask << 1;                   // Deslocar a máscara de um bit para esquerda
//    }
//
//
//    return Byte_Read;                           // Retornar o valor lido (1 byte)
//}
//uint16_t readTemp_1W()
//{
//
//    LSB_temp = 0x00;                // Define a variável "LSB_temp" para receber o 8 bits LS da temp
//    MSB_temp = 0x00;                // Define a variável "MSB_temp" para receber o 8 bits MS da temp
//    readByte_1W();
//    LSB_temp = Byte_Read;
//    readByte_1W();
//    MSB_temp = Byte_Read;
//    MSB_temp = MSB_temp << 8;               // Aloca corretamente os 8 bits do MSB da leitura de temperatura
//    temp = MSB_temp + LSB_temp;
//    tempCelsius = temp * 0.0625;
//    tempCelsius_int = tempCelsius*100;
//    lcdCursor(0x47);
//    lcdNumVir3(tempCelsius_int);
//    //////////////////////////////////////////////////
//    // Visor do LCD: 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F
//    //               0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F
//    // A   L   A   R   M   :                       C
//    //x00 x01 x02 x03 x04 x05 x06 x07 x08 x09 x0A x0B x0C x0D x0E x0F
//    // T   E   M   P       :                       C
//    //x40 x41 x42 x43 x44 x45 x46 x47 x48 x49 x4A x4B x4C x4D x4E x4F
//    return tempCelsius_int;
//}
//
