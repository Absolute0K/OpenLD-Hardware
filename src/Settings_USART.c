/*
    This file is part of the project OpenLD.

    OpenLD is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    OpenLD is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with OpenLD.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "main.h"

// #define SETTINGS_BIT_HPFILTER(x) 1 << (4 * (x - '0' - 1) + 1)
// #define SETTINGS_BIT_LPFILTER(x) 1 << (4 * (x - '0' - 1) + 2)
// #define SETTINGS_BIT_IMP(x)      1 << (4 * (x - '0' - 1) + 3)

void setting_mode()
{
    // Disable IRQ to prevent idiotic stuff from happening
    NVIC_DisableIRQ(USART6_IRQn);

    STX("*********************************************\n");
    STX("*          SETTINGS MODE                    *\n");
    STX("*===========================================*\n");
    STX("* OPTIONS:                                  *\n");
    STX("*  - C_CHANNEL + OPT: SET CHANNEL           *\n");
    STX("*  > OPT: NO, T-EST, S-HORT, TE-MP, SU-PPLY *\n");
    STX("*  - I: ENABLE IMPEDANCE MEASUREMENT        *\n");
    STX("*  - B: TOGGLE GPIO INPUT/OUTPUT SETTING    *\n");
    STX("*  - S: PRINT ALL SETTINGS                  *\n");
    STX("*  - P: PRINT CHANNEL CONFIGS               *\n");
    STX("*  - E: Quit                                *\n");
    STX("*********************************************\n");

    while (1) {

        // Signal end of transmission: 0x17 + \n:
        CTX(0x17);
        CTX('\n');

        // STX(">> ");

        // Initialize and clear input_str_cmd contents
        char input_str_cmd[50];
        for (int i = 0; i < 50; i++) input_str_cmd[i] = 0x0;

        SRX(input_str_cmd);

        // Echo the input string
        STX(input_str_cmd);
        CTX('\n');


        // Check if the second argument/character is a valid number 1 - 9
        // NOTE: THIS IF STATEMENT CONTINUES ON FOR TWO IF STATEMENTS
        if (input_str_cmd[1] > '0' && input_str_cmd[1] < '9') {

            /*************************
             *  SET UP A NEW CHANNEL *
             *************************/
            if (input_str_cmd[0] == 'C') {

                STX("CHANNEL_");
                // Check if the input is a number[1:8]

                // Echo channel number
                CTX(input_str_cmd[1]);

                uint8_t input_opt = 0x0;

                if      (input_str_cmd[2] == 'N') {
                    input_opt = ADS1299_INPUT_NORMAL;
                    CTX('N');
                } else if (input_str_cmd[2] == 'T') {
                    input_opt = ADS1299_INPUT_TESTSIGNAL;
                    CTX('T');
                } else if (input_str_cmd[2] == 'S') {
                    input_opt = ADS1299_INPUT_SHORTED;
                    CTX('S');
                } else if (input_str_cmd[2] == 'E') {
                    input_opt = ADS1299_INPUT_TEMP;
                    CTX('E');
                } else if (input_str_cmd[2] == 'U') {
                    input_opt = ADS1299_INPUT_SUPPLY;
                    CTX('U');
                } else
                    CTX('N');

                BIOEXG_SETTINGS ^= SETTINGS_BIT_CHANNEL(input_str_cmd[1] - '1');

                // Determine whether or not the Channel is ON or OFF
                if (BIOEXG_SETTINGS & SETTINGS_BIT_CHANNEL(input_str_cmd[1] - '1')) {
                    // Magic happens underneath (input_str_cmd[1] - '1' + CH1SET), don't touch
                    ads1299_write_reg((uint8_t) input_str_cmd[1] - '1' + CH1SET, ADS1299_INPUT_PWR_UP
                                      | ADS1299_PGA_GAIN24 | input_opt);
                    STX(": ON\n");

                } else {
                    ads1299_write_reg((uint8_t) input_str_cmd[1] - '1' + CH1SET, ADS1299_INPUT_PWR_DOWN
                                      | ADS1299_PGA_GAIN24 | input_opt);
                    STX(": OFF\n");
                }

                /**********************************************
                 *  SET UP IMPEDANCE MEASUREMENT ON A CHANNEL *
                 **********************************************/

            } else if (input_str_cmd[0] == 'I') {

                STX("CHANNEL_");
                CTX(input_str_cmd[1]);
                STX(" IMPEDANCE MEASUREMENT");

                // Set bit in BIOEXG SETTINGS and use that bitfield to correctly set LOFF_SENSEP in ADS1299
                BIOEXG_SETTINGS ^= SETTINGS_BIT_IMP(input_str_cmd[1] - '1');
                ads1299_write_reg(LOFF_SENSP, (uint8_t) 0xFF & BIOEXG_SETTINGS >> 12);

                if (BIOEXG_SETTINGS & SETTINGS_BIT_IMP(input_str_cmd[1] - '1')) STX(": ON\n");
                else                                                      STX(": OFF\n");
            }

            // END IF STATEMENT ON CHANNEL NUMBER CHECKING

        } else if (input_str_cmd[0] == 'P') {
            for (int i = 0; i < 8; i++) {
                STX("Channel ");
                CTX(i + '1');
                STX(": ");
                PREG(ads1299_read_reg(CH1SET + i));
                CTX('\n');
            }

            /**************************
             *  CHANGE BUTTON SETTING *
             **************************/
        } else if (input_str_cmd[0] == 'B') {

            STX("GPIOD ODR Register (BEFORE): ");
            PREG(ads1299_read_reg(GPIOD->MODER));

            uint32_t setting_GPIO = GPIOD->MODER & GPIO_MODER_MODER9_0;
            setting_GPIO ^= GPIO_MODER_MODER9_0;
            GPIOD->MODER |= setting_GPIO;


            STX("GPIOD ODR Register (AFTER): ");
            PREG(ads1299_read_reg(GPIOD->MODER));

            /**********************
             *  PRINT ALL CONFIGS *
             **********************/

        } else if (input_str_cmd[0] == 'S') {
            STX("Channel + Impedance_M Status: ");
            PREG(BIOEXG_SETTINGS);

            STX("\nConfiguration Register 1: ");
            PREG(ads1299_read_reg(CONFIG1));

            STX("\nConfiguration Register 2: ");
            PREG(ads1299_read_reg(CONFIG2));

            STX("\nConfiguration Register 3: ");
            PREG(ads1299_read_reg(CONFIG3));

            STX("\nConfiguration Register 4: ");
            PREG(ads1299_read_reg(CONFIG4));

            STX("\nLOFF: ");
            PREG(ads1299_read_reg(LOFF));

            STX("\nLOFF_SENSEP: ");
            PREG(ads1299_read_reg(LOFF_SENSP));

            STX("\nLOFF_SENSEN: ");
            PREG(ads1299_read_reg(LOFF_SENSN));

            CTX('\n');
            /*********
             *  QUIT *
             *********/

        } else if (input_str_cmd[0] == 'E') break;

        else  STX("WTF\n");
    }

    // Enable NVIC again
    NVIC_EnableIRQ(USART6_IRQn);

}
