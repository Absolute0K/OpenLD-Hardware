// Define to prevent recursive inclusion
#ifndef __MAIN_H
#define __MAIN_H

// Header Files
#include "stm32f4xx.h"
#include "core_cm4.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_spi.h"

// DSP FFT Header Files
#include "arm_math.h"
#include "arm_const_structs.h"

#define ADS1299_CHANNELS 8

// Function Prototypes
    // Initialization Routines
    void init_USART6();
    void init_SPI1();
    void init_GPIO();
    void init_EXTI();
    void FPU_INIT();
    // Other things
    void check_RN42_RTS();
    void EXTI0_IRQHandler();
    void EXTI9_5_IRQHandler();
    void USART6_IRQHandler();
    // Delays
    void __DELAY(uint32_t cycles);
    // External ASM functions
    extern void STX(const char *X0);
    extern void SRX(char *X0);
    extern void CTX(char X0);
    extern char CRX();
    extern void PREG(uint32_t R0);
    extern void PDEC(uint32_t R0);

    // Analysis Functions & Definitions & Variables
    #define ADS1299_SIGNAL_WINDOW 250

    // IIR Structures
    #define BIQUAD_STAGES_HP 39
    #define BIQUAD_STAGES_BP 21
    extern arm_biquad_cascade_df2T_instance_f32 biquad_HP_Struct[ADS1299_CHANNELS];
    extern float32_t biquad_HP_State[ADS1299_CHANNELS][2 * BIQUAD_STAGES_HP];
    extern float32_t biquad_HP_Coeffs[5 * BIQUAD_STAGES_HP];
    extern const float32_t biquad_HP_Output_Gain;

    extern arm_biquad_cascade_df2T_instance_f32 biquad_BP_Struct[ADS1299_CHANNELS];
    extern float32_t biquad_BP_State[ADS1299_CHANNELS][2 * BIQUAD_STAGES_BP];
    extern float32_t biquad_BP_Coeffs[5 * BIQUAD_STAGES_BP];
    extern const float32_t biquad_BP_Output_Gain;

    // BioEXG Settings variable + Function
    extern uint32_t BIOEXG_SETTINGS;
    void setting_mode();

    // Data counter
    extern uint32_t counterData;

// BIOEXG_SETTINGS Bit field:
// [24:16 - IMPEDANCE] [7:0 - CHANNELS]
    #define SETTINGS_BIT_CHANNEL(x)  1 << (x + 0)
    #define SETTINGS_BIT_IMP(x)      1 << (x + 12)

    // ADS1299 Functions
    void ads1299_init();
    void ads1299_pwr_up_seq();
    void ads1299_read_data(uint32_t *STATUS, int32_t *DATA);
    void ads1299_stop_dataread();
    uint8_t ads1299_read_reg(uint8_t ADDR);
    void ads1299_write_reg(uint8_t ADDR, uint8_t VAL);
    uint8_t SPI_TX(uint8_t DATA);
    uint8_t SPI_NO_DELAY_TX(uint8_t DATA);


// Macro Definitions
#define MILI_S(x)  x * 168000/4
#define MICRO_S(x) x * 168/4

// Thanks to OpenBCI for the definitions :D

// SPI Command Definitions (Datasheet, 35)
#define _WAKEUP  0x02 // Wake-up from standby mode
#define _STANDBY 0x04 // Enter Standby mode
#define _RESET   0x06 // Reset the device registers to default
#define _START   0x08 // Start and restart (synchronize) conversions
#define _STOP    0x0A // Stop conversion
#define _RDATAC  0x10 // Enable Read Data Continuous mode (default mode at power-up)
#define _SDATAC  0x11 // Stop Read Data Continuous mode
#define _RDATA   0x12 // Read data by command; supports multiple read back
#define _RREG    0x20 // Read Register
#define _WREG    0x40 // Write to Register

// Register Addresses
#define ID         0x00
#define CONFIG1    0x01
#define CONFIG2    0x02
#define CONFIG3    0x03
#define LOFF       0x04
#define CH1SET     0x05
#define CH2SET     0x06
#define CH3SET     0x07
#define CH4SET     0x08
#define CH5SET     0x09
#define CH6SET     0x0A
#define CH7SET     0x0B
#define CH8SET     0x0C
#define BIAS_SENSP 0x0D
#define BIAS_SENSN 0x0E
#define LOFF_SENSP 0x0F
#define LOFF_SENSN 0x10
#define LOFF_FLIP  0x11
#define LOFF_STATP 0x12
#define LOFF_STATN 0x13
#define GPIO       0x14
#define MISC1      0x15
#define MISC2      0x16
#define CONFIG4    0x17

// Gains
#define ADS1299_PGA_GAIN01 (0b00000000)
#define ADS1299_PGA_GAIN02 (0b00010000)
#define ADS1299_PGA_GAIN04 (0b00100000)
#define ADS1299_PGA_GAIN06 (0b00110000)
#define ADS1299_PGA_GAIN08 (0b01000000)
#define ADS1299_PGA_GAIN12 (0b01010000)
#define ADS1299_PGA_GAIN24 (0b01100000)

// Input Modes - Channels

#define ADS1299_INPUT_PWR_DOWN   (0b10000000)
#define ADS1299_INPUT_PWR_UP     (0b00000000)

#define ADS1299_INPUT_NORMAL     (0b00000000)
#define ADS1299_INPUT_SHORTED    (0b00000001)
#define ADS1299_INPUT_MEAS_BIAS  (0b00000010)
#define ADS1299_INPUT_SUPPLY     (0b00000011)
#define ADS1299_INPUT_TEMP       (0b00000100)
#define ADS1299_INPUT_TESTSIGNAL (0b00000101)
#define ADS1299_INPUT_SET_BIASP  (0b00000110)
#define ADS1299_INPUT_SET_BIASN  (0b00000111)

// Test Signal Choices - p41
#define ADS1299_TEST_INT              (0b00010000)
#define ADS1299_TESTSIGNAL_AMP_1X     (0b00000000)
#define ADS1299_TESTSIGNAL_AMP_2X     (0b00000100)
#define ADS1299_TESTSIGNAL_PULSE_SLOW (0b00000000)
#define ADS1299_TESTSIGNAL_PULSE_FAST (0b00000001)
#define ADS1299_TESTSIGNAL_DCSIG      (0b00000011)
#define ADS1299_TESTSIGNAL_NOCHANGE   (0b11111111)

//Lead-off Signal Choices
#define LOFF_MAG_6NA (0b00000000)
#define LOFF_MAG_24NA (0b00000100)
#define LOFF_MAG_6UA (0b00001000)
#define LOFF_MAG_24UA (0b00001100)
#define LOFF_FREQ_DC (0b00000000)
#define LOFF_FREQ_7p8HZ (0b00000001)
#define LOFF_FREQ_31p2HZ (0b00000010)
#define LOFF_FREQ_FS_4 (0b00000011)
#define PCHAN (1)
#define NCHAN (2)
#define BOTHCHAN (3)

#define OFF (0)
#define ON (1)

#endif // __MAIN_H