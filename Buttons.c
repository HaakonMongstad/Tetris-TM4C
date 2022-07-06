#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
void EnableInterrupts(void);


void buttonInit(void){
	uint32_t delay;
	SYSCTL_RCGCGPIO_R |= 0x10;
	delay = SYSCTL_RCGCGPIO_R;
	GPIO_PORTE_DIR_R &= (~0x0F);
	GPIO_PORTE_DEN_R |= 0x0F;
}
	
uint8_t flag3 = 0;
uint8_t flag4 = 0;

volatile uint32_t FallingEdges = 0;

void EdgeCounter_Init(void){
	int delay;
  SYSCTL_RCGCGPIO_R |= 0x00000010; // (a) activate clock for port E
	delay = SYSCTL_RCGCGPIO_R;
//  risingEdges = 0;             // (b) initialize count and wait for clock
  GPIO_PORTE_DIR_R &= ~0x0C;    // (c) make PE0 input
  GPIO_PORTE_DEN_R |= 0x0C;     //     enable digital I/O on PE0
  GPIO_PORTE_IS_R &= ~0x0C;     // (d) PE0 is edge-sensitive
  GPIO_PORTE_IBE_R &= ~0x0C;    //     PE0 is not both edges
  GPIO_PORTE_IEV_R &= 0x0C;    //     PE0 rising edge event
  GPIO_PORTE_ICR_R = 0x0C;      // (e) clear flag0
  GPIO_PORTE_IM_R |= 0x0C;      // (f) arm interrupt on PE0
  NVIC_PRI1_R = (NVIC_PRI1_R & 0xFFFFFF1F)|0x00000040; // (g) priority 2 
  NVIC_EN0_R = 0x00000010;      // (h) enable interrupt 4 in NVIC
  EnableInterrupts();           // (i) Enable global Interrupt flag (I)

}

void GPIOPortE_Handler(void){

 // GPIO_PORTF_ICR_R = 0x10;      // acknowledge flag4
	if ((GPIO_PORTE_RIS_R & 0x08) == 0x08){
	flag3 = 1;
	}
	else if ((GPIO_PORTE_RIS_R & 0x04) == 0x04){
		flag4 = 1;
	}
	GPIO_PORTE_ICR_R = 0x0C;      // acknowledge flag4
}
	