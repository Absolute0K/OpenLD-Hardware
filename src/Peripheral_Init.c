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
#define  AFR_Pin(x) x * 4

void init_GPIO()
{
    // Enable buses GPIOB, GPIOC, GPIOD and GPIOE
    RCC->AHB1ENR |= (RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOEEN);

    // GPIOB
    // Set Alternate Function mode for ADS1299 SPI's - PB3, PB4, PB5; PB6 and PB7 as GPIO Output
    GPIOB->MODER |= (GPIO_MODER_MODER3_1 | GPIO_MODER_MODER4_1 |
                     GPIO_MODER_MODER5_1 | GPIO_MODER_MODER6_0 | GPIO_MODER_MODER7_0);
    // Set fast speed for PB3, PB4, PB5
    GPIOB->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR3_1 | GPIO_OSPEEDER_OSPEEDR4_1 |
                       GPIO_OSPEEDER_OSPEEDR5_1 | GPIO_OSPEEDER_OSPEEDR7_1);
    // WTF PB4 IS PULLDOWN
    GPIOB->PUPDR = 0x0;
    // WITH THAT SAID, LETS PULL PB6 DOWN
    GPIOB->BSRRH |= GPIO_Pin_6;
    // Set SPI1 for PB3, PB4, PB5
    GPIOB->AFR[0] |= (GPIO_AF_SPI1 << AFR_Pin(3) | GPIO_AF_SPI1 << AFR_Pin(4) |
                      GPIO_AF_SPI1 << AFR_Pin(5));

    // GPIOC
    // Set Alternate Function mode for RN42 USART's - PC6, PC7
    GPIOC->MODER   |= (GPIO_MODER_MODER6_1 | GPIO_MODER_MODER7_1);
    // Set fast speed for PC6, PC7
    GPIOC->OSPEEDR |= (GPIO_OSPEEDER_OSPEEDR6_1 | GPIO_OSPEEDER_OSPEEDR7_1);
    // Set USART6 for PC6, PC7
    GPIOC->AFR[0]  |= (GPIO_AF_USART6 << AFR_Pin(6) | GPIO_AF_USART6 << AFR_Pin(7));

    // GPIOD
    // Set Output mode for CS (PD7), CTS (PD14), and Input mode for RTS
    // PD9 - CS_5 (PD9) is the External Button pin, therefore Input mode
    // PD9 - CS_4 (PD10) is the External Trigger pin, therefore Output
    // GPIOD->MODER   |= (GPIO_MODER_MODER7_0 | GPIO_MODER_MODER14_0 | GPIO_MODER_MODER9_0);
    GPIOD->MODER   |= (GPIO_MODER_MODER7_0 | GPIO_MODER_MODER14_0 | GPIO_MODER_MODER10_0);
    GPIOD->OSPEEDR |= GPIO_OSPEEDER_OSPEEDR7_1;
    GPIOD->BSRRL   |= GPIO_Pin_7;
    GPIOD->BSRRH   |= GPIO_Pin_14;

    // Set internal pullup for the External Button/Stimulus Trigger (PD9)
    GPIOD->PUPDR   |= GPIO_PUPDR_PUPDR9_0;

    // GPIOE
    // Set Output mode for the LED's - PE7, PE8, PE10
    GPIOE->MODER |= (GPIO_MODER_MODER7_0 | GPIO_MODER_MODER8_0 | GPIO_MODER_MODER10_0);
}

void init_USART6()
{
    // Enable USART6 Bus
    RCC->APB2ENR |= RCC_APB2ENR_USART6EN;
    // Set USART6 Baudrate
    // 115Kbps approx.
    //USART6->BRR = 0x2D9;
    // 922Kbps approx 5.6875 = 5B
    //USART6->BRR = 0x5B;
    // 230Kbps approx.
    USART6->BRR = 0x16D;

    // Set Priority VERY LOW
    NVIC_SetPriority(USART6_IRQn, 10);
    // Clear Pending
    NVIC_ClearPendingIRQ(USART6_IRQn);
    // Interrupt line is EXTI6 - PD6
    NVIC_EnableIRQ(USART6_IRQn);

    // Enable Transmit & Recieve & Recieve Interrupt then Enable USART6
    USART6->CR1 |= (USART_CR1_TE | USART_CR1_RE | USART_CR1_RXNEIE);
    USART6->CR1 |= USART_CR1_UE;
}

void check_RN42_RTS()
{
    // If the RN42_RTS signal is high (RN42 is too busy)
    // then wait and turn off the green LED
    // when it is no longer high, turn green LED back on

    while (GPIOD->IDR & GPIO_IDR_IDR_15) {
        GPIOE->ODR |= GPIO_Pin_8;
    }

    GPIOE->ODR &= ~GPIO_Pin_8;
}

void init_SPI1()
{
    // Enable SPI1 Bus
    RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
    // Configure & Enable SPI1
    // CPHA = 1; CPOL = 0; MASTER; BR = 84Mhz/8 = 10.5Mhz
    SPI1->CR1 = (SPI_CR1_CPHA | SPI_CR1_MSTR | SPI_CR1_BR_1 | SPI_CR1_BR_0 |
                 SPI_CR1_SPE | SPI_CR1_SSI  | SPI_CR1_SSM);
}

void init_EXTI()
{
    // Enable SysCFG
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    // Connect PD6 -> EXTI6
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI6_PD;
    // Remove mask on EXTI6 line
    EXTI->IMR |= EXTI_IMR_MR6;
    // Remove mask on EXTI0 line
    EXTI->IMR |= EXTI_IMR_MR0;
    // Trigger EXTI6 on Falling Edge
    EXTI->FTSR |= EXTI_FTSR_TR6;
    // Enable Interrupts
    // __enable_irq();

    // Disable any customization going on
    NVIC_SetPriorityGrouping(0);

    // Set Priority
    NVIC_SetPriority(EXTI0_IRQn, 5);
    NVIC_SetPriority(EXTI9_5_IRQn, 4);

    // Clear Pending
    NVIC_ClearPendingIRQ(EXTI0_IRQn);
    NVIC_ClearPendingIRQ(EXTI9_5_IRQn);

    // Interrupt line is EXTI6 - PD6
    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI9_5_IRQn);
}
