#include <stdint.h>

#include "../inc/tm4c123gh6pm.h"

void Timer0A_Init(void(*task)(void), uint32_t period);

void Timer0A_Handler2(void);

