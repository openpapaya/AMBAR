/*
 * lcd.c
 *
 *  Created on: 9 de jul de 2021
 *      Author: guiwz
 */
//
#include <msp430.h>
#include <stdint.h>

#include <libs/config_i2c_5529.h>
#include <libs/config_lcd_5529.h>
#include <libs/config_timers_5529.h>

//                              *** ENDERECOS I2C ***
// PCF8574 - CONTROLE DAS VALVULAS  - Endereco: 0x20
// PCF8574 - SENSORES DE NIVEL      - Endereco: 0x21
// PCF8574 - LCD                    - Endereco: 0x27
// PCF8574 - LCD                    - Endereco: 0x26    // Misturador
// INA219  - Leitor Tensao/Corrente - Endereco: 0x40
// --------------------------------------------------------------
// Biblioteca de instru��es:
//
//           lcdWriteByte(0x02, INSTR);        // Return home
//
// --------------------------------------------------------------

        // Visor do LCD: 0x00 0x01 0x02 0x03 0x04 0x05 0x06 0x07 0x08 0x09 0x0A 0x0B 0x0C 0x0D 0x0E 0x0F 0x10 0x11 0x12 0x13
        //               0x40 0x41 0x42 0x43 0x44 0x45 0x46 0x47 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4E 0x4F 0x50 0x51 0x52 0x53
        //               0x14 0x15 0x16 0x17 0x18 0x19 0x1A 0x1B 0x1C 0x1D 0x1E 0x1F 0x20 0x21 0x22 0x23 0x24 0x25 0x26 0x27
        //               0x54 0x55 0x56 0x57 0x58 0x59 0x5A 0x5B 0x5C 0x5D 0x5E 0x5F 0x60 0x61 0x62 0x63 0x64 0x65 0x66 0x67

// Nibble = agrupamento de 4 bits
// 1 byte 0xAB ( _ _ _ _   _ _ _ _  )
//           __
// EN --> __|  |__
// Envio dos nibbles:
//       4 bits de dados        |        4 bits de controle
//  -------- MSnibble ----------|---------- LSnibble ----------
//  7   |   6   |   5   |   4   |   3   |   2   |   1   |   0
//  D7  |   D6  |   D5  |   D4  |   Bl  |   E   |   RW  |   RS
//     Primeiro envia o MSnibble, depois o LSnibble
//Testar o endere�o adr para escrita

void lcdWriteNibble( uint8_t addr, uint8_t nibble, uint8_t instr0char1)            // Fun��o que escreve um nibble, ser� usada logo abaixo
{
    nibble <<= 4;                           // Colocar o nibble nos 4 bits mais significativos, o LSnibble sera para
                                            // BL,     EN,      R/~W e      RS
    //          (addr , nibble |Backlight|Enable|Read/~Write|     RS      )
    i2cWriteByte_UCB1( addr , nibble |   BL    |  0   |    0      | instr0char1 );          // (0x(nibble)9 . instr0char1 = RS, 0 � instru��o, 1 � caractere
    wait(500,us);
    i2cWriteByte_UCB1( addr , nibble |   BL    |  EN  |    0      | instr0char1 );          // (0x(nibble)D . R/~W diz se queremos LER ou ESCREVER no LCD. 0 � escrever
    wait(500,us);
    i2cWriteByte_UCB1( addr , nibble |   BL    |  0   |    0      | instr0char1 );          // (0x(nibble)9
    wait(500,us);
    // ------------------------------------------------------------------
    // "i2cwriteByte (uint8_t addr, uint8_t byte)" � o "port�o de embarque" para os bytes de comunica��o
    // com o LCD, por meio do protocolo i2c. O byte de dados � enviado em duas partes, sempre no MSnibble
    // com o LSnibble sendo reservado para os bits de controle, BL, EN, W/~R, RS
    // tendo como caracter�stica o envio de tr�s "pacotes", o primeiro com EN=0, o segundo EN=1
    // e o �ltimo com EN=0:                __
    //                             EN --> __|  |__
    // POR QUE � assim? � dessa forma porque a leitura dos dados acontece no flanco de descida do enable,
    // ent�o primeiro enviamos os dados para o LCD, depois fazemos um novo envio, agora ativando o enable
    // e por fim, enviamos o mesmo byte de dados s� que com o enable em LOW, ou seja, ocorre um flanco de descida do EN.
}
    // Exemplo, enviando a letra A (0x41) para o LCD na posi��o em que o cursor estiver, ap�s isso o cursor move-se automaticamente
    // para a pr�xima posi��o

    // Uma palavra = 2 bytes = 0x49 = 0100 1001
    // Uma palavra = 2 bytes = 0x4D = 0100 1101
    // Uma palavra = 2 bytes = 0x49 = 0100 1001

    // Uma palavra = 2 bytes = 0x19 = 0001 1001
    // Uma palavra = 2 bytes = 0x1D = 0001 1101
    // Uma palavra = 2 bytes = 0x19 = 0001 1001
//----------------------------------------------------------------------------------------------
//----------------- C�digo para escrever caracteres no LCD -------------------------------------
void lcdWriteByte(uint8_t addr, uint8_t byte, uint8_t instr0char1)  // Fun��o que prepara os dois nibbles que s�o os bits de dados
{

    lcdWriteNibble(addr, byte >> 4,   instr0char1 );        // Manda primeiro o MSNibble, seguindo a logica do protocolo
    lcdWriteNibble(addr, byte & 0x0F, instr0char1 );        // depois o LSNibble
}

void lcdInit(uint8_t addr)
{
    // Inicializa o LCD em modo 8-bits      // D7 D6 D5 D4 D3 D2 D1 D0
    lcdWriteNibble(addr, 0x3, INSTR);             // Garante que o LCD est� em modo 8 bits

    //             ( addc ,nibble|Backlight|Enable|Read/~Write|       RS    )
    // i2cWriteByte( 0x27 , 0011 |   BL    |  0   |    0      | instr0char1 ); => 0011 1000
    // i2cWriteByte( 0x27 , 0011 |   BL    |  1   |    0      | instr0char1 ); => 0011 1100
    // i2cWriteByte( 0x27 , 0011 |   BL    |  0   |    0      | instr0char1 ); => 0011 1000
    lcdWriteNibble(addr, 0x3, INSTR);             // antes de entrarmos em modo
    //             ( addc ,nibble|Backlight|Enable|Read/~Write|       RS    )
    // i2cWriteByte( 0x27 , 0011 |   BL    |  0   |    0      | instr0char1 ); => 0011 1000
    // i2cWriteByte( 0x27 , 0011 |   BL    |  1   |    0      | instr0char1 ); => 0011 1100
    // i2cWriteByte( 0x27 , 0011 |   BL    |  0   |    0      | instr0char1 ); => 0011 1000
    lcdWriteNibble(addr, 0x3, INSTR);             // 4 bits
    //             ( addc ,nibble|Backlight|Enable|Read/~Write|       RS    )
    // i2cWriteByte( 0x27 , 0011 |   BL    |  0   |    0      | instr0char1 ); => 0011 1000
    // i2cWriteByte( 0x27 , 0011 |   BL    |  1   |    0      | instr0char1 ); => 0011 1100
    // i2cWriteByte( 0x27 , 0011 |   BL    |  0   |    0      | instr0char1 ); => 0011 1000


    lcdWriteNibble(addr, 0x2, INSTR);             // Modo 4 bits
    //             ( addc ,nibble|Backlight|Enable|Read/~Write|       RS    )
    // i2cWriteByte( 0x27 , 0010 |   BL    |  0   |    0      | instr0char1 ); => 0010 1000
    // i2cWriteByte( 0x27 , 0010 |   BL    |  1   |    0      | instr0char1 ); => 0010 1100
    // i2cWriteByte( 0x27 , 0010 |   BL    |  0   |    0      | instr0char1 ); => 0010 1000


    lcdWriteByte(addr, 0x06, INSTR);              // Configura o LCD

    lcdWriteByte(addr, 0x0C, INSTR);              // 0  0  0  0  1  D ~C  ~B,
                                                 // desligar o cursor e o blink
    lcdWriteByte(addr, 0x14, INSTR);              // Cursor anda para a direita
    lcdWriteByte(addr, 0x28, INSTR);              // Modo 4 bits, 2 linhas
    lcdWriteByte(addr, 0x01, INSTR);              // Limpa o LCD

    // From "https://www.robotshop.com/community/forum/t/drive-a-standard-hd44780-lcd-using-a-pcf8574-and-i2c/12876"
    // The data sheet warns that under certain conditions, the lcd may fail to initialize properly when power is first applied.
    // This is particulary likely if the Vdd supply does not rise to its correct operating voltage quickly enough. It is recommended
    // that after power is applied a command sequence of 3 bytes of values $30 is sent to the module. This will guarantee that the
    // module is in 8 bit mode and properly initialised.

}




