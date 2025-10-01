#ifndef STM32_I2C_H_
#define STM32_I2C_H_
#include<stm32f10x.h>
#include<stm32f10x_i2c.h>

#define I2C_NO_REMAP		0
#define I2C_REMAP				1	

#define I2C_IDLE				0
#define I2C_BUSY_IN_RX 	1
#define I2C_BUSY_IN_TX 	2

#define I2C_EV_TX_CMPLT	1
#define I2C_EV_RX_CMPLT	2
#define	I2C_EV_STOPF		3
#define I2C_EV_DATA_REQ	4
#define I2C_EV_DATA_RCV 5

#define I2C_DISABLE_SR 	0
#define I2C_ENABLE_SR 	1


typedef struct I2C_Handle_t{
	I2C_TypeDef *pI2Cx;
	I2C_InitTypeDef *pI2C_InitStructure;
	uint8_t *pTxBuffer;
	uint8_t *pRxBuffer;
	uint32_t TxLen;
	uint32_t RxLen;
	uint8_t TxRxState;
	uint8_t DevAddr;
	uint32_t RxSize;
	uint8_t Sr;// luu repeated start value
}I2C_Handle_t;

extern I2C_Handle_t I2C1Handle;
extern I2C_Handle_t I2C2Handle;

void I2C_Config(I2C_TypeDef* pI2Cx, uint8_t remap, uint32_t I2C_Speed);
uint8_t I2C_ScanBus(void);
void I2C_MasterSendData(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr);
void I2C_MasterReceiveData(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr);
uint8_t I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr);
uint8_t I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr);
void I2C_EV_IRQHandling(I2C_Handle_t *pI2CHandle);
void I2C_ERR_IRQHandling(I2C_Handle_t *pI2CHandle);
void I2C_CloseSendData(I2C_Handle_t *pI2CHandle);
__weak void I2C_ApplicationEventCallBack(I2C_Handle_t *pI2CHandle, uint8_t event);
#endif