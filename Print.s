; Print.s
; Haakon Mongstad
; Runs on TM4C123
; EE319K lab 7 device driver for any LCD
;
; As part of Lab 7, students need to implement these LCD_OutDec and LCD_OutFix
; This driver assumes two low-level LCD functions
; ST7735_OutChar   outputs a single 8-bit ASCII character
; ST7735_OutString outputs a null-terminated string 

FP 		RN	 11
Count 	EQU	 0
Num 	EQU	 4


    IMPORT   ST7735_OutChar
    IMPORT   ST7735_OutString
    EXPORT   LCD_OutDec
    EXPORT   LCD_OutFix

    AREA    |.text|, CODE, READONLY, ALIGN=2
    THUMB

 PRESERVE8 

;-----------------------LCD_OutDec-----------------------
; Output a 32-bit number in unsigned decimal format
; Input: R0 (call by value) 32-bit unsigned number
; Output: none
; Invariables: This function must not permanently modify registers R4 to R11
; R0=0,    then output "0"
; R0=3,    then output "3"
; R0=89,   then output "89"
; R0=123,  then output "123"
; R0=9999, then output "9999"
; R0=4294967295, then output "4294967295"
LCD_OutDec
	PUSH {R4-R9, R11, LR}
	SUB SP, #8		; Allocate
	MOV FP, SP
	MOV R4, #0
	STR R4, [FP, #Count]
	STR R0, [FP, #Num]
	CMP R0, #0
	BEQ PRINTZERO
	MOV R5, #10
loop
	LDR R4, [FP, #Num]
	CMP R4, #0
	BEQ NEXT
	UDIV R6, R4, R5
	MUL R7, R6, R5
	SUB R8, R4, R7
	PUSH{R8}
	STR R6, [FP, #Num]
	LDR R6, [FP, #Count]
	ADD R6, #1
	STR R6, [FP, #Count]
	B loop
NEXT
	LDR R4, [FP, #Count]
	CMP R4, #0
	BEQ DONE
	POP{R0}
	ADD R0, #0x30		; ASCII offset
	BL ST7735_OutChar
	SUB R4, #1
	STR R4, [FP, #Count]
	B NEXT
PRINTZERO
	ADD R0, #0x30
	BL ST7735_OutChar
DONE
	ADD SP, #8		;DeAllocate
	POP {R4-R9, R11, LR}
	

    BX  LR
;* * * * * * * * End of LCD_OutDec * * * * * * * *

; -----------------------LCD _OutFix----------------------
; Output characters to LCD display in fixed-point format
; unsigned decimal, resolution 0.001, range 0.000 to 9.999
; Inputs:  R0 is an unsigned 32-bit number
; Outputs: none
; E.g., R0=0,    then output "0.000"
;       R0=3,    then output "0.003"
;       R0=89,   then output "0.089"
;       R0=123,  then output "0.123"
;       R0=9999, then output "9.999"
;       R0>9999, then output "*.***"
; Invariables: This function must not permanently modify registers R4 to R11

LCD_OutFix

	PUSH {R4-R9, R11, LR}
	SUB SP, #12		; Allocate
	MOV FP, SP
	MOV R4, #0
	STR R4, [FP, #Count]
	STR R0, [FP, #Num]
	MOV R5, #10
loopF
	LDR R4, [FP, #Num]
	CMP R4, #0
	BEQ NEXTF
	UDIV R6, R4, R5
	MUL R7, R6, R5
	SUB R8, R4, R7
	PUSH{R8}
	STR R6, [FP, #Num]
	LDR R6, [FP, #Count]
	ADD R6, #1
	STR R6, [FP, #Count]
	B loopF
NEXTF
	LDR R4, [FP, #Count]
	CMP R4, #4
	BHI OutOfRange
	CMP R4, #4
	BEQ PRINTNUM
	MOV R1, #0
	PUSH{R1}
	ADD R4, #1
	STR R4, [FP, #Count]
	B NEXTF
PRINTD
	MOV R0, #0x2E
	BL ST7735_OutChar
	B PRINTNEXT
PRINTNUM
	LDR R4, [FP, #Count]
	CMP R4, #3
	BEQ PRINTD
	CMP R4, #0
	BEQ DONEF
PRINTNEXT
	POP{R0}
	ADD R0, #0x30		; ASCII offset
	BL ST7735_OutChar
	SUB R4, #1
	STR R4, [FP, #Count]
	B PRINTNUM
OutOfRange
	LDR R4, [FP, #Count]
DECREMENT
	CMP R4, #0
	BEQ NEXTO
	POP{R1}
	SUBS R4, #1
	B DECREMENT
NEXTO
	MOV R0, #0x2A
	BL ST7735_OutChar
	MOV R0, #0x2E
	BL ST7735_OutChar
	MOV R0, #0x2A
	BL ST7735_OutChar
	MOV R0, #0x2A
	BL ST7735_OutChar
	MOV R0, #0x2A
	BL ST7735_OutChar

DONEF
	ADD SP, #12	;DeAllocate
	POP {R4-R9, R11, LR}
	

    BX   LR
 
     ALIGN
;* * * * * * * * End of LCD_OutFix * * * * * * * *

     ALIGN                           ; make sure the end of this section is aligned
     END                             ; end of file
