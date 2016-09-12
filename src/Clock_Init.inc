.MACRO	CLOCK_INIT

CLK_PWR_MODE:
    MOV		R2,	#(1 << 28)
    LMS		R2,	AHB1,	RCC,	RCC_APB1ENR
	
CLK_BUS_PRESET:
    LDR		R2,	=(5 << 10 | 4 << 13)	@ APB1 = AHB/4, APB2 = AHB/2
    LMSC	R2,	RCC_CFGR
	
CLK_HSE_ON:
    MOV		R2,	#(1 << 16)					@ HSE ON
    LMSC	R2,	RCC_CR
	
CLK_HSE_RDY:
    LRVC	R2,	RCC_CR
    TST		R2,	#(1 << 17)
    BEQ		CLK_HSE_RDY

CLK_PLL_PRESET:
    /*
    FCLK     = EXTCLK * (PLLN / PLLM)
    PLL_OUT  = FCLK / PLLP
    SDIO_CLK = FCLK / PLLQ
    */

    @ EXTCLK = 16Mhz; PLLM = 4; PLLN = 168; PLLP = 2; PLLQ = 7
    LDR		R2,	=(8 | 168 << 6 | 0 << 16 | 1 << 22 | 7 << 24 | 2 << 28)
    OSRC	R2,	RCC_PLLCFGR
	
CLK_PLL_ON:
    MOV		R2,	#(1 << 24)
    LMSC	R2,	RCC_CR

CLK_PLL_RDY:
    LRVC	R2,	RCC_CR
    TST		R2,	#(1 << 25)
    BEQ		CLK_PLL_RDY
	
CLK_FLASH_LATENCY:
    LDR		R2,	=(5 << 0 | 1 << 9 | 1 << 10)
    LMS		R2,	AHB1,	FLASH,	FLASH_ACR

CLK_ACTIVATE_PLL:
    MOV		R2,	#(1 << 1)
    LMS		R2,	AHB1,	RCC,	RCC_CFGR

CLK_PLL_STATUS:
    LRVC	R2,	RCC_CFGR
    AND		R2,	R2,	#0xF
    TST		R2,	#0xA
    BEQ		CLK_PLL_STATUS

.ENDM
