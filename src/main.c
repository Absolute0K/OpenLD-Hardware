/* STFT with my BioEXG
   DATE Created: 7/19/2014
   DATE Working: 7/30/2014
   DATE STFT: 7/31/2014
   DATE FIR + IIR: 11/22/2014
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
