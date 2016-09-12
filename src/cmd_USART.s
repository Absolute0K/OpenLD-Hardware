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

    .INCLUDE "src/RDP.inc"

    .TEXT
    .SYNTAX	UNIFIED
    .THUMB
    .GLOBAL	CRX
    .GLOBAL	CTX
    .GLOBAL	STX
    .GLOBAL	SRX
    .GLOBAL	PREG
    .GLOBAL	PDEC
    .TYPE	CRX,	%function
    .TYPE	CTX,	%function
    .TYPE	STX,	%function
    .TYPE	SRX,	%function
    .TYPE	PREG,	%function
    .TYPE	PDEC,	%function

CTX:
	MOV	R2,	R0
WAIT_TXE:
	LRV	R3,	APB2,	USART6,	USART_SR
	TST	R3,	#(1 << 7)
	BEQ	WAIT_TXE

SEND_C:
	OSRC	R2,	USART_DR,	B

WAIT_TC:
	LRVC	R3,	USART_SR
	TST	R3,	#(1 << 6)
	BEQ	WAIT_TC
	BX	LR

@ -------------------------------------

CRX:
WAIT_RXNE:
	LRV	R3,	APB2,	USART6,	USART_SR
	TST	R3,	#(1 << 5)
	BEQ	WAIT_RXNE

READ_C:
	LRVC	R0,	USART_DR,	B
	BX	LR

@ -------------------------------------

SRX:
	PUSH	{LR}
	MOV	R1,	R0

RX_UNTIL_NEWLINE:
	BL	CRX

	TEQ	R0,	#0xA	@ Get out when R0 = 0xA (NOT 0x10 WTF) aka \n
	BEQ	EXIT_SRX

	TEQ	R0,	#0xD	@ Get out when R0 = 0xD (NOT 0x10 WTF) aka \r
	BEQ	EXIT_SRX	@ THIS IS VERY IMPORTANT FOR LINUX

	STRB	R0,	[R1],	#0x1

	B	RX_UNTIL_NEWLINE

EXIT_SRX:
	POP	{PC}

@ -------------------------------------

STX:
	PUSH	{LR}
	MOV	R1,	R0

RC_SEND:
	LDRB	R0,	[R1],	#0x1

	TST	R0,	#0xFF	@ Get out when R2 = 0x0
	BEQ	EXIT_STR

	BL	CTX
	B	RC_SEND

EXIT_STR:
	POP	{PC}

@ -------------------------------------

PREG:
	PUSH	{R4, LR}
	MOV	R1,	R0	@ Save Register
	MOV	R4, #8		@ Counter
/*
	MOV	R2,	#'0'
	BL	CTX

	MOV	R2,	#'x'
	BL	CTX
*/
HEX_SEND:
	ROR	R1, R1, #28
	AND	R0, R1,	#0xF	@ Extract 1 digit

	CMP	R0, #0xA
	ITE	GE

	ADDGE	R0, #55		@ Convert to ASCII (Capital letters)
	ADDLT	R0, #48 	@ Just use number 0-9
	BL	CTX

	SUBS	R4, #0x1
	BNE	HEX_SEND

	POP	{R4, PC}
@ -------------------------------------
PDEC:
@ Send Register value in Decimal
@ R0: SP; R1: Temp; R2: Register value;
@ R3: R2 / 10; R4: R2 - (R3 * 10) => Remainder


	PUSH	{R4, R5, LR}
	MOV	R2,	R0
	MOV	R0,	SP
	SUB	SP,	SP,	#12
	MOV	R1,	#0x0
	STRB	R1,	[R0, #-1]!

	MOV	R5, #10

DEC_LOOP:
	UDIV	R3,	R2,	R5
	MUL	R1,	R3,	R5
	SUB	R4,	R2,	R1
	ADD	R4,	R4,	#48
	STRB	R4,	[R0, #-1]!

	MOVS	R2,	R3
	BNE	DEC_LOOP

	BL	STX
	ADD	SP,	SP,	#12

	@ MOV	R0,	#'\n'
	@ BL	CTX

	POP	{R4, R5, PC}
@ -------------------------------------

.END
