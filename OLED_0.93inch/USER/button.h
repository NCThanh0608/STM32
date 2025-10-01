#ifndef __BUTTON__
#define __BUTTON__
#ifdef __cplusplus
extern "C"{
#endif

#include "stm32f10x.h"                  // Device header

typedef struct
{
	GPIO_TypeDef *port;
	uint16_t pin;
	uint8_t pre_status;
	
} Button;

void Button_Init(Button *button, GPIO_TypeDef *port, uint16_t pin);
extern __inline uint8_t Button_Read(Button *button);
uint8_t Button_IsPressed(Button *button);

#ifdef __cplusplus
}
#endif
#endif