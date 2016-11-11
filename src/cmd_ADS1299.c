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

void ads1299_init()
{
    // ADS1299 Power up
    STX("Starting Power up sequence...\n");
    ads1299_pwr_up_seq();
    __DELAY(MILI_S(1000));
    ads1299_pwr_up_seq();
    STX("Sequence completed\n");

    // Stop Conversion for Configuration
    STX("Sending Stop Data CMD...\n");
    ads1299_stop_dataread();

    // WHO AM I?
    if (ads1299_read_reg(ID) == 0x3E) STX("Life is good\n");
    else {
        STX("SHIT. I DON'T KNOW WHO I AM\n");
        while (1);
    }

    // Some Configs:
    // Turn on Reference buffer and Bias buffer in CONFIG3
    // Set Test signal configs in CONFIG2
    ads1299_write_reg(CONFIG3, 0x60 | 1 << 7 | 1 << 2);
    ads1299_write_reg(CONFIG2, 0xC0 | ADS1299_TEST_INT | ADS1299_TESTSIGNAL_PULSE_FAST);

    // All Channels are PGA = 24 | Normal Input | Powered down except Channel 8
    ads1299_write_reg(CH1SET, ADS1299_PGA_GAIN24 | ADS1299_INPUT_NORMAL | ADS1299_INPUT_PWR_DOWN);
    ads1299_write_reg(CH2SET, ADS1299_PGA_GAIN24 | ADS1299_INPUT_NORMAL | ADS1299_INPUT_PWR_DOWN);
    ads1299_write_reg(CH3SET, ADS1299_PGA_GAIN24 | ADS1299_INPUT_NORMAL | ADS1299_INPUT_PWR_DOWN);
    ads1299_write_reg(CH4SET, ADS1299_PGA_GAIN24 | ADS1299_INPUT_NORMAL | ADS1299_INPUT_PWR_DOWN);
    ads1299_write_reg(CH5SET, ADS1299_PGA_GAIN24 | ADS1299_INPUT_NORMAL | ADS1299_INPUT_PWR_DOWN);
    ads1299_write_reg(CH6SET, ADS1299_PGA_GAIN24 | ADS1299_INPUT_NORMAL | ADS1299_INPUT_PWR_DOWN);
    ads1299_write_reg(CH7SET, ADS1299_PGA_GAIN24 | ADS1299_INPUT_NORMAL | ADS1299_INPUT_PWR_DOWN);
    ads1299_write_reg(CH8SET, ADS1299_PGA_GAIN24 | ADS1299_INPUT_NORMAL | ADS1299_INPUT_PWR_DOWN);

    // Configure Lead Off Options in LOFF: Set AC lead-off at 62.5hz (f_DR/4)
    ads1299_write_reg(LOFF, LOFF_FREQ_FS_4);
    // ads1299_write_reg(LOFF_SENSP, 0xFF);

    // Connect SRB1 to all inverting outputs
    ads1299_write_reg(MISC1, 1 << 5);

#ifdef DEBUG_ADS1299
    STX("CONFIG3 - should be 0xE0: ");
    PREG(ads1299_read_reg(CONFIG3));
    STX("CONFIG2: ");
    PREG(ads1299_read_reg(CONFIG2));
    STX("CH8SET: ");
    PREG(ads1299_read_reg(CH8SET));
#endif

    // Gimme time to check settings
    //__DELAY(MILI_S(2000));

    // Start the conversion
    GPIOB->BSRRL |= GPIO_Pin_6;
}



void ads1299_pwr_up_seq()
{
    // WAIT 40ms
    __DELAY(MILI_S(40));
    // PULL RESET LOW
    GPIOB->BSRRH |= GPIO_Pin_7;
    // WAIT 2us
    __DELAY(MICRO_S(2));
    // PULL RESET HIGH
    GPIOB->BSRRL |= GPIO_Pin_7;
    // WAIT 10us
    __DELAY(MICRO_S(10));
}

void ads1299_stop_dataread()
{
    // PULL CS LOW
    GPIOD->BSRRH |= GPIO_Pin_7;
    // SEND BYTE: 0x11
    SPI_TX(_SDATAC);
    // PULL CS HIGH
    GPIOD->BSRRL |= GPIO_Pin_7;
    // WAIT 2*TCLK'S = 888ns = 1us
    __DELAY(MICRO_S(1));
}

void ads1299_read_data(uint32_t *STATUS, int32_t *DATA)
{

    // PULL CS LOW
    GPIOD->BSRRH |= GPIO_Pin_7;

    // SEND BYTE: 0x11
    SPI_NO_DELAY_TX(_RDATA);
    // READ STATUS
    *STATUS = SPI_NO_DELAY_TX(0x0) << 16;
    *STATUS |= SPI_NO_DELAY_TX(0x0) << 8;
    *STATUS |= SPI_NO_DELAY_TX(0x0);

    // READ DATA 0 - 7
    int i = 0;
    for (i = 0; i < 8; i++) {
        DATA[i] = SPI_NO_DELAY_TX(0x0) << 16;
        DATA[i] |= SPI_NO_DELAY_TX(0x0) << 8;
        DATA[i] |= SPI_NO_DELAY_TX(0x0);

        // Take care of the sign
        if (DATA[i] & 1 << 23) {
            DATA[i] ^= 0x00FFFFFF;
            DATA[i]++;
            DATA[i] &= 0x00FFFFFF;
            DATA[i] *= -1;
        }
    }

    // PULL CS HIGH
    GPIOD->BSRRL |= GPIO_Pin_7;
    // WAIT 2*TCLK'S = 888ns = 1us
    __DELAY(MICRO_S(1));
}

void ads1299_write_reg(uint8_t ADDR, uint8_t VAL)
{
    // PULL CS LOW
    GPIOD->BSRRH |= GPIO_Pin_7;

    // SEND FIRST BYTE: 0x2 | ADDR
    SPI_TX(_WREG | ADDR);
    // SEND SECOND BYTE: NUMBER_TO_WRITE
    SPI_TX(0x00);
    // SEND VALUE TO WRITE
    SPI_TX(VAL);

    // PULL CS HIGH
    GPIOD->BSRRL |= GPIO_Pin_7;
    // WAIT 2*TCLK'S = 888ns = 1us
    __DELAY(MICRO_S(1));
}


uint8_t ads1299_read_reg(uint8_t ADDR)
{
    // PULL CS LOW
    GPIOD->BSRRH |= GPIO_Pin_7;

    // SEND FIRST BYTE: 0x2 | ADDR
    SPI_TX(_RREG | ADDR);
    // SEND SECOND BYTE: NUMBER_TO_READ -- pretty much always 0x0
    SPI_TX(0x00);
    // SEND A DUMMY BYTE TO RECIEVE DATA
    uint8_t RESP = SPI_TX(0x00);

    // PULL CS HIGH
    GPIOD->BSRRL |= GPIO_Pin_7;
    // WAIT 2*TCLK'S = 888ns = 1us
    __DELAY(MICRO_S(1));
    // Return read value
    return (RESP);
}

uint8_t SPI_TX(uint8_t DATA)
{
    // SEND DATA
    SPI1->DR = (uint8_t)DATA;

    // WAIT UNTIL TRANSMIT IS COMPLETED
    while (!(SPI1->SR & SPI_I2S_FLAG_TXE));
    // WAIT UNTIL DATA RECIEVE IS COMPLETED
    while (!(SPI1->SR & SPI_I2S_FLAG_RXNE));
    // WAIT UNTIL SPI IS NOT BUSY
    while (SPI1->SR & SPI_I2S_FLAG_BSY);
    // WAIT 2uS - Give the chip some time 2 sort things out
    __DELAY(MICRO_S(2));

    // RETURN RECIEVED DATA
    return (SPI1->DR);
}


uint8_t SPI_NO_DELAY_TX(uint8_t DATA)
{
    // SEND DATA
    SPI1->DR = (uint8_t)DATA;

    // WAIT UNTIL TRANSMIT IS COMPLETED
    while (!(SPI1->SR & SPI_I2S_FLAG_TXE));
    // WAIT UNTIL DATA RECIEVE IS COMPLETED
    while (!(SPI1->SR & SPI_I2S_FLAG_RXNE));
    // WAIT UNTIL SPI IS NOT BUSY
    while (SPI1->SR & SPI_I2S_FLAG_BSY);

    // RETURN RECIEVED DATA
    return (SPI1->DR);
}
