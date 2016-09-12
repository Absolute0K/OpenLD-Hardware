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
