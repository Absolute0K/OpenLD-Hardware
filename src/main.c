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

// Initialize global variables
// float32_t HM_Window[ADS1299_SIGNAL_WINDOW];

// Global IIR variables
float32_t biquad_HP_State[ADS1299_CHANNELS][2 * BIQUAD_STAGES_HP];
arm_biquad_cascade_df2T_instance_f32 biquad_HP_Struct[ADS1299_CHANNELS];

float32_t biquad_BP_State[ADS1299_CHANNELS][2 * BIQUAD_STAGES_BP];
arm_biquad_cascade_df2T_instance_f32 biquad_BP_Struct[ADS1299_CHANNELS];

// Global FIR variable
// arm_fir_instance_f32 FIR_Struct[ADS1299_CHANNELS];
// uint32_t blockSize, numBlocks;

int main()
{

    // Peripheral Configs
    init_GPIO();
    __enable_irq();
    init_USART6();

    // Initialize IIR filters for Real Time Impedance Calculation
    for (int i = 0; i < ADS1299_CHANNELS; i++) {
        arm_biquad_cascade_df2T_init_f32(&biquad_HP_Struct[i], BIQUAD_STAGES_HP, biquad_HP_Coeffs, biquad_HP_State[i]);
        arm_biquad_cascade_df2T_init_f32(&biquad_BP_Struct[i], BIQUAD_STAGES_BP, biquad_BP_Coeffs, biquad_BP_State[i]);
    }

    init_SPI1();
    ads1299_init();
    init_EXTI();
    while (1); //__WFI();
}
