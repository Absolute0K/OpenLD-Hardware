/* MIT License

   Copyright (c) [2016] [Jae Choi]

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
 */


#include "main.h"

// Initalize counter + BioEXG settings
uint32_t counterData = 0;
uint32_t BIOEXG_SETTINGS = 0x0;

// IIR Filter Output
float32_t output_IIR[ADS1299_CHANNELS][ADS1299_SIGNAL_WINDOW];
float32_t output_IIR_IMPEDANCE[ADS1299_CHANNELS][ADS1299_SIGNAL_WINDOW];

// Impedance calculation
float32_t sum_Impedance[ADS1299_CHANNELS];
float32_t rms_Impedance[ADS1299_CHANNELS];
int32_t rms_Impedance_Integer[ADS1299_CHANNELS];

// These are 2D-arrays (integer and float): Array[DATA_POINTS][CHANNELS]
float32_t data_Signal_Float[ADS1299_CHANNELS][ADS1299_SIGNAL_WINDOW];
int32_t data_Signal_Integer[ADS1299_CHANNELS][ADS1299_SIGNAL_WINDOW];

void EXTI9_5_IRQHandler()
{
    int32_t REALS[8];
    uint32_t RDATA_STATUS;
    uint32_t i;

    ads1299_read_data(&RDATA_STATUS, REALS);


    // Send data character
    //CTX('D'); // SEND DATA TYPE - RAW DATA

    for (i = 0; i < 8; i++) {
        if (BIOEXG_SETTINGS & SETTINGS_BIT_CHANNEL(i)) {
            // Add to uint data buffer
            data_Signal_Integer[i][counterData] = REALS[i];

            // Send data
            // check_RN42_RTS();

            // if (data_Signal_Integer[i][counterData] < 0) {
            //     CTX('-');
            //     data_Signal_Integer[i][counterData] *= -1;
            // }
            // PDEC((uint32_t)data_Signal_Integer[i][counterData]);
            // CTX(' ');

            // Converting Integer to Scaled Floating Point for Impedance checking
            // Range/(2^23 - 1) = 2.2351744...E-5
            data_Signal_Float[i][counterData] = ((float32_t)REALS[i]) * 2.235174445E-5;
        }
    }

    // Send ENDLINE character
    //CTX('\n');

    counterData++;

    if (counterData == ADS1299_SIGNAL_WINDOW) {
        counterData = 0;
        NVIC_SetPendingIRQ(EXTI0_IRQn);
    }

    // Forgot this:
    EXTI->PR |= EXTI_PR_PR6;
    // Clear pending bit
    NVIC_ClearPendingIRQ(EXTI9_5_IRQn);
}

void EXTI0_IRQHandler()
{
    // LED Indicator - Orange for IRQ calls, Blue for Data processing duration
    GPIOE->ODR ^= GPIO_Pin_10;
    // GPIOE->ODR |= GPIO_Pin_7;
    // GPIOE->ODR &= ~GPIO_Pin_8;


    uint32_t i, j;

    for (j = 0; j < ADS1299_SIGNAL_WINDOW; j++) {
        CTX('D'); // SEND DATA TYPE - RAW DATA
        for (i = 0; i < 8; i++) {
            if (BIOEXG_SETTINGS & SETTINGS_BIT_CHANNEL(i)) {
                check_RN42_RTS();

                if (data_Signal_Integer[i][j] < 0) {
                    CTX('-');
                    data_Signal_Integer[i][j] *= -1;
                }
                PDEC((uint32_t)data_Signal_Integer[i][j]);
                CTX(' ');
            }
        }
        CTX('\n'); // SEND ENDLINE
    }

    for (i = 0; i < 8; i++) {
        if (BIOEXG_SETTINGS & SETTINGS_BIT_IMP(i)) {
            /***************************
            * IIR DC REMOVAL HP FILTER *
            ***************************/
            /************************************
            * IIR IMPEDENCE WAVEFORM EXTRACTION *
            *************************************/

            arm_biquad_cascade_df2T_f32(&biquad_HP_Struct[i], data_Signal_Float[i], output_IIR_IMPEDANCE[i], ADS1299_SIGNAL_WINDOW);
            // Mutiply biquad SOS output by output coeff
            for (j = 0; j < ADS1299_SIGNAL_WINDOW; j++) output_IIR_IMPEDANCE[i][j] *= biquad_HP_Output_Gain;

            arm_biquad_cascade_df2T_f32(&biquad_BP_Struct[i], output_IIR_IMPEDANCE[i], output_IIR[i], ADS1299_SIGNAL_WINDOW);
            // Mutiply biquad SOS output by output coeff, also compute amplitude

            sum_Impedance[i] = 0;
            rms_Impedance[i] = 0;

            for (j = 0; j < ADS1299_SIGNAL_WINDOW; j++) {
                output_IIR[i][j] *= biquad_BP_Output_Gain;
                sum_Impedance[i] += output_IIR[i][j] * output_IIR[i][j];
            }

            // Calculate Amplitude
            sum_Impedance[i] = (float)sum_Impedance[i] / (float)ADS1299_SIGNAL_WINDOW;
            arm_sqrt_f32(sum_Impedance[i], &rms_Impedance[i]);
            rms_Impedance_Integer[i] = rms_Impedance[i] * 10000.0f;

            /*
                        float32_t cos_fs4_table[] = {1, 0, -1, 0};
                        float32_t sin_fs4_table[] = {0, 1, 0, -1};

                        float32_t sin_accum = 0.0f, cos_accum = 0.0f, block_avg = 0.0f;

                        for (int j = 0; j < ADS1299_SIGNAL_WINDOW; j++) block_avg += data_Signal_Float[i][j];

                        block_avg /= ADS1299_SIGNAL_WINDOW;

                        for (int j = 0; j < ADS1299_SIGNAL_WINDOW; j++) {
                            cos_accum += (data_Signal_Float[i][j] - block_avg) * cos_fs4_table[j & 0x3];
                            sin_accum += (data_Signal_Float[i][j] - block_avg) * sin_fs4_table[j & 0x3];
                        }

                        // Calculate Amplitude
                        arm_sqrt_f32((float32_t)(sin_accum * sin_accum + cos_accum * cos_accum), &rms_Impedance[i]);
                        // rms_Impedance[i] *= 1.414f / (float32_t)ADS1299_SIGNAL_WINDOW / 6.0f;
                        // Print raw output so that fudge can be applied at the other end
                        rms_Impedance_Integer[i] = rms_Impedance[i] * 10000.0f;
                        */
        }
    }

    CTX('I'); // SEND DATA TYPE - IMPEDANCE MEASUREMENT
    for (i = 0; i < 8; i++) {
        if (BIOEXG_SETTINGS & SETTINGS_BIT_IMP(i)) {

            if (rms_Impedance_Integer[i] < 0) {
                CTX('-');
                rms_Impedance_Integer[i] *= -1;
            }
            PDEC((uint32_t)rms_Impedance_Integer[i]);
            CTX(' ');
        }
    }
    CTX('\n'); // SEND ENDLINE


    /* Print status for the External Interface */
    // Print out value for inverted PD9, shifted 9 (cause of pullup resistor)
    uint32_t status_button = !((GPIOD->IDR & GPIO_IDR_IDR_9) >> 9);
    if (status_button) GPIOE->ODR |= GPIO_Pin_7;
    else               GPIOE->ODR &= ~GPIO_Pin_7;


    CTX('B');
    PDEC((uint32_t)status_button);
    CTX('\n'); // SEND ENDLINE

// Append semicolon to the text to indicate end of 1 second packet
    STX("; We're good:\n");

// Forgot this:
    EXTI->PR |= EXTI_PR_PR0;
// Clear pending bit
    NVIC_ClearPendingIRQ(EXTI0_IRQn);
}

void USART6_IRQHandler()
{
    char recieved_char = CRX();

    if (recieved_char == 'S') {

        // Turn on Amber LED to indicate a Keypress then stop ADS1299 conversion
        GPIOE->BSRRL |= GPIO_Pin_10;
        GPIOB->BSRRH |= GPIO_Pin_6;  // STOP

        // If S was pressed, then enter Settings mode
        setting_mode();

        // Turn off Amber LED then restart conversion
        GPIOE->BSRRH |= GPIO_Pin_10;
        GPIOB->BSRRL |= GPIO_Pin_6;  // START

        // Reset EVERYTHING
        counterData = 0;
        NVIC_ClearPendingIRQ(EXTI0_IRQn);
        NVIC_SetPendingIRQ(EXTI0_IRQn);

    } else if (recieved_char == 'O') {

        GPIOD->ODR ^= GPIO_Pin_10;
        /*/ Stimulation - Send SOS
        GPIOD->ODR |= GPIO_Pin_10;  __DELAY(MILI_S(100));
        GPIOD->ODR &= ~GPIO_Pin_10; __DELAY(MILI_S(100));
        GPIOD->ODR |= GPIO_Pin_10;  __DELAY(MILI_S(100));
        GPIOD->ODR &= ~GPIO_Pin_10; __DELAY(MILI_S(100));
        GPIOD->ODR |= GPIO_Pin_10;  __DELAY(MILI_S(100));
        GPIOD->ODR &= ~GPIO_Pin_10; __DELAY(MILI_S(700));

        GPIOD->ODR |= GPIO_Pin_10;  __DELAY(MILI_S(100));
        GPIOD->ODR &= ~GPIO_Pin_10; __DELAY(MILI_S(700));
        GPIOD->ODR |= GPIO_Pin_10;  __DELAY(MILI_S(100));
        GPIOD->ODR &= ~GPIO_Pin_10; __DELAY(MILI_S(700));
        GPIOD->ODR |= GPIO_Pin_10;  __DELAY(MILI_S(100));
        GPIOD->ODR &= ~GPIO_Pin_10; __DELAY(MILI_S(700));

        GPIOD->ODR |= GPIO_Pin_10;  __DELAY(MILI_S(100));
        GPIOD->ODR &= ~GPIO_Pin_10; __DELAY(MILI_S(100));
        GPIOD->ODR |= GPIO_Pin_10;  __DELAY(MILI_S(100));
        GPIOD->ODR &= ~GPIO_Pin_10; __DELAY(MILI_S(100));
        GPIOD->ODR |= GPIO_Pin_10;  __DELAY(MILI_S(100));
        GPIOD->ODR &= ~GPIO_Pin_10; __DELAY(MILI_S(100));
        */
    }

    NVIC_ClearPendingIRQ(USART6_IRQn);
}
