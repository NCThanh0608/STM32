#include "button.h"
#include "delay.h"

void Button_Init(Button *button, GPIO_TypeDef *port, uint16_t pin)
{
	GPIO_InitTypeDef   GPIO_InitStruct;
	
	button->port = port;
	button->pin  = pin;
	
	if(port == GPIOA) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	if(port == GPIOB) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	if(port == GPIOC) RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);
  
	GPIO_InitStruct.GPIO_Pin = pin;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	
	GPIO_Init(port, &GPIO_InitStruct);
}

__inline uint8_t Button_Read(Button *button)
{
	return (button->port->IDR & button->pin) && 1;
}

uint8_t Button_IsPressed(Button *button)
{
    uint8_t ispressed = 0;
    uint8_t status = Button_Read(button);

    if(status != button->pre_status)
    {
				delay_ms(DEBOUNCE_MS);
        status = Button_Read(button);
        if(status != button->pre_status && !status)
        {
            ispressed = 1;
        }
    }

    button->pre_status = status;
    return ispressed;
}
