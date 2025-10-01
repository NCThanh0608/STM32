#ifndef OLED_128x64_H
#define OLED_128x64_H
#include "i2c.h"
#include <stdarg.h>
void OLED_SendCommand(uint8_t cmd);
void OLED_SendData(uint8_t data);
void OLED_Init(I2C_Handle_t* pI2CxHandle);
void OLED_Fill(uint8_t data, uint8_t offset);
void OLED_WriteChar(char c);
void OLED_WriteString(const char *str);
void OLED_SetCursor(uint8_t x, uint8_t y, uint8_t offset) ;
void OLED_IntToString(int num, char *str);
void OLED_WriteInt(int num);
void OLED_Printf(const char *format, ...);
void OLED_PrintfAt(uint8_t x, uint8_t y, uint8_t offset, const char *format, ...);
#endif