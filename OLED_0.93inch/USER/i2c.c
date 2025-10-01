#include<stm32f10x.h>
#include<stm32f10x_i2c.h>
#include"i2c.h"
#include<stdlib.h>

I2C_Handle_t I2C1Handle;
I2C_Handle_t I2C2Handle;

void I2C_Config(I2C_TypeDef* pI2Cx, uint8_t remap, uint32_t I2C_Speed)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	I2C_InitTypeDef I2C_InitStructure;
	if(pI2Cx == I2C1)
	{
		I2C1Handle.pI2Cx = I2C1;
		if(remap == I2C_NO_REMAP)
		{
			/* Enable I2C1 clock */
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

			/* Enable GPIOB clock */
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);

			/* Configure I2C1 SCL pin (PB8) as alternate function open-drain */
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
			GPIO_Init(GPIOB, &GPIO_InitStructure);
			
			/* Configure I2C1 SDA pin (PB9) as alternate function open-drain */
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
			GPIO_Init(GPIOB, &GPIO_InitStructure);
		}
		else if(remap == I2C_REMAP)
		{
			/* Enable I2C1 clock */
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);

			/* Enable GPIOB clock */
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			GPIO_InitTypeDef GPIO_InitStructure;
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
			GPIO_PinRemapConfig(GPIO_Remap_I2C1, ENABLE);

			/* Configure I2C1 SCL pin (PB8) as alternate function open-drain */
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
			GPIO_Init(GPIOB, &GPIO_InitStructure);
			
			/* Configure I2C1 SDA pin (PB9) as alternate function open-drain */
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
			GPIO_Init(GPIOB, &GPIO_InitStructure);
		}

		/* I2C1 configuration */
		I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
		I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2; // For fast mode
		I2C_InitStructure.I2C_OwnAddress1 = 0x00;         // Master mode, this doesn't matter
		I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
		I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
		I2C_InitStructure.I2C_ClockSpeed = I2C_Speed;
		/* Apply I2C1 configuration */
		I2C_Init(I2C1, &I2C_InitStructure);
		/* Enable I2C1 */
		I2C_Cmd(I2C1, ENABLE);

		NVIC_InitTypeDef NVIC_InitStructure;
		
		// Cau hình I2C1 Event interrupt
		NVIC_InitStructure.NVIC_IRQChannel = I2C1_EV_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
		
		// Cau hình I2C1 Error interrupt
		NVIC_InitStructure.NVIC_IRQChannel = I2C1_ER_IRQn;
		NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
		NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
		NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
		NVIC_Init(&NVIC_InitStructure);
	}
	else if (pI2Cx == I2C2)
	{
		// ===============TODO==================
	}
}
// =======================TODO Fix ScanBus to adapt any I2C=======================
uint8_t I2C_ScanBus(void)
{
    for (uint8_t address = 1; address < 127; address++)
    {
        // 1. T?so START condition
        I2C_GenerateSTART(I2C1, ENABLE);

        // 2. Ch? SB du?c set
        while (!I2C_GetFlagStatus(I2C1, I2C_FLAG_SB));

        // 3. G?i d?a ch? (ch? truy?n 7-bit, thu vi?n s? shift trái 1 bit)
        I2C_Send7bitAddress(I2C1, address, I2C_Direction_Transmitter);

        // 4. Ch? s? ki?n ADDR ho?c AF (Acknowledge Failure)
        while (!(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) ||
                 I2C_GetFlagStatus(I2C1, I2C_FLAG_AF)));

        if (I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
        {
            return address;
        }

        // 5. Clear AF flag n?u b? t? ch?i (NACK)
        I2C_ClearFlag(I2C1, I2C_FLAG_AF);

        // 6. G?i STOP
        I2C_GenerateSTOP(I2C1, ENABLE);

        // 7. Delay nh? gi?a các d?a ch? (n?u c?n)
        for (volatile int i = 0; i < 10000; i++);
    }
}

void I2C_ClearAddrFlag(I2C_TypeDef *pI2Cx)
{
	 volatile uint32_t temp;
   temp = pI2Cx->SR1;
   temp = pI2Cx->SR2;
   (void)temp;
}

void I2C_ClearStopfFlag(I2C_TypeDef *pI2Cx)
{
	// Ki?m tra xem STOPF flag có du?c set không
  if (I2C_GetFlagStatus(pI2Cx, I2C_FLAG_STOPF) == SET) {
	// Clear STOPF flag bang cách dcc SR1 roi ghi vào CR1
		I2C_Cmd(pI2Cx, ENABLE);  // Ghi vào CR1 (bat ki giá tri nào)   
  }
}

void I2C_CloseSendData(I2C_Handle_t *pI2CHandle)
{
	// T?t interrupt ITBUFEN (Buffer interrupt)
	I2C_ITConfig(pI2CHandle->pI2Cx, I2C_IT_BUF, DISABLE);
	
	// T?t interrupt ITEVTEN (Event interrupt)
	I2C_ITConfig(pI2CHandle->pI2Cx, I2C_IT_EVT, DISABLE);
	
	// T?t interrupt l?i
	I2C_ITConfig(pI2CHandle->pI2Cx, I2C_IT_ERR, DISABLE);
	
	// Reset tr?ng thái transmission
	pI2CHandle->TxRxState = I2C_IDLE;
	
	// Reset con tr? buffer
	pI2CHandle->pTxBuffer = NULL;
	
	// Reset d? dài d? li?u
	pI2CHandle->TxLen = 0;
	
	// Reset d?a ch? slave
	pI2CHandle->DevAddr = 0;
	
	// Reset c? Repeated Start
	pI2CHandle->Sr = I2C_DISABLE_SR;
}

void I2C_CloseReceiveData(I2C_Handle_t *pI2CHandle)
{
	// T?t interrupt ITBUFEN (Buffer interrupt)
    I2C_ITConfig(pI2CHandle->pI2Cx, I2C_IT_BUF, DISABLE);
    
    // T?t interrupt ITEVTEN (Event interrupt)
    I2C_ITConfig(pI2CHandle->pI2Cx, I2C_IT_EVT, DISABLE);
    
    // T?t interrupt l?i
    I2C_ITConfig(pI2CHandle->pI2Cx, I2C_IT_ERR, DISABLE);
    
    // Reset tr?ng thái reception
    pI2CHandle->TxRxState = I2C_IDLE;
    
    // Reset con tr? buffer
    pI2CHandle->pRxBuffer = NULL;
    
    // Reset d? dài d? li?u
    pI2CHandle->RxLen = 0;
    
    // Reset kích thu?c d? li?u
    pI2CHandle->RxSize = 0;
    
    // Reset d?a ch? slave
    pI2CHandle->DevAddr = 0;
    
    // Reset c? Repeated Start
    pI2CHandle->Sr = I2C_DISABLE_SR;
    
    // Khôi phuc ACK neu can
    if(pI2CHandle->pI2C_InitStructure->I2C_Ack == I2C_Ack_Enable)
    {
        I2C_AcknowledgeConfig(pI2CHandle->pI2Cx, ENABLE);
    }
}

void I2C_MasterSendData(I2C_Handle_t *pI2CHandle, uint8_t *pTxBuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr)
{
	//1. Tao ra START
	I2C_GenerateSTART(pI2CHandle->pI2Cx, ENABLE);
	
	//2. Kiem tra START da duoc tao ra bang cach check SB flag 
	// Cho den khi SB duoc set thi SCL bi keo xuong low (stretched)
	while(I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB) != SET);
	
	//3. Gui 7 bit dia chi va 1 bit RW
	I2C_Send7bitAddress(pI2CHandle->pI2Cx, SlaveAddr << 1, I2C_Direction_Transmitter);
	
	//4. Confirm da hoan thanh gui dung Addr bang cach check ADDR flag
	while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR));
	
	//5. Clear ADDR flag
	// Cho den khi ADDR duoc reset thi SCL bi keo xuong low (stretched)
	I2C_ClearAddrFlag(pI2CHandle->pI2Cx);
	
	//6. Gui data cho den khi Len = 0
	while(Len > 0)
	{
		while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE));
		I2C_SendData(pI2CHandle->pI2Cx, *pTxBuffer);
		pTxBuffer ++;
		Len --;
	}
	
	//7. Khi Len = 0 thi doi den khi TXE = 1 va BTF = 1 -> Tao ra Stop
	// Khi BTF = 1 -> SCL bi keo xuong low (stretched)
	while(I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE) != SET);
	while(I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_BTF) != SET);
	
	//8. Tao ra STOP
	// Tao ra STOP se tu dong clear BTF
	I2C_GenerateSTOP(pI2CHandle->pI2Cx, ENABLE);
}

void I2C_MasterReceiveData(I2C_Handle_t *pI2CHandle, uint8_t *pRxBuffer, uint32_t Len, uint8_t SlaveAddr, uint8_t Sr)
{
	//1. Tao ra START
	I2C_GenerateSTART(pI2CHandle->pI2Cx, ENABLE);
	
	//2. Kiem tra START da duoc tao ra bang cach check SB flag 
	// Cho den khi SB duoc set thi SCL bi keo xuong low (stretched)
	while(I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_SB) != SET);
	
	//3. Gui 7 bit dia chi va 1 bit RW
	I2C_Send7bitAddress(pI2CHandle->pI2Cx, SlaveAddr << 1, I2C_Direction_Receiver);
	
	//4. Confirm da hoan thanh gui dung Addr bang cach check ADDR flag
	while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_ADDR));
	
	//5. Clear ADDR flag
	// Cho den khi ADDR duoc reset thi SCL bi keo xuong low (stretched)
	I2C_ClearAddrFlag(pI2CHandle->pI2Cx);
	
	// Neu chi nhan 1 byte
	if(Len == 1)
	{
		//Disable ACKING
		I2C_AcknowledgeConfig(pI2CHandle->pI2Cx, DISABLE);
		//Clear ADDR flag
		I2C_ClearAddrFlag(pI2CHandle->pI2Cx);
		//doi den khi co RXNE duoc SET
		while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE));
		//Tao ra STOP
		I2C_GenerateSTOP(pI2CHandle->pI2Cx, ENABLE);
		//Doc data tu buffer
		*pRxBuffer = I2C_ReceiveData(pI2CHandle->pI2Cx);
		
		return;
	}
	
	if(Len > 1)
	{
		//Clear ADDR flag
		I2C_ClearAddrFlag(pI2CHandle->pI2Cx);
		//Doc data cho den khi Len = 0
		while(Len --)
		{
			//doi den khi co RXNE duoc SET
			while(!I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE));
			if(Len == 2)
			{
				//Disable ACKING
				I2C_AcknowledgeConfig(pI2CHandle->pI2Cx, DISABLE);
				//Tao ra STOP
				I2C_GenerateSTOP(pI2CHandle->pI2Cx, ENABLE);
			}
			//Doc Data tu buffer:
			*pRxBuffer = I2C_ReceiveData(pI2CHandle->pI2Cx);
			pRxBuffer ++;
		}
	}
	I2C_AcknowledgeConfig(pI2CHandle->pI2Cx, ENABLE);
}

/*********************************************************************
 * @fn      		  - I2C_MasterSendDataIT
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              -  Complete the below code . Also include the function prototype in header file

 */
uint8_t  I2C_MasterSendDataIT(I2C_Handle_t *pI2CHandle,uint8_t *pTxBuffer, uint32_t Len,uint8_t SlaveAddr,uint8_t Sr)
{

	uint8_t busystate = pI2CHandle->TxRxState;

	if( (busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
	{
		pI2CHandle->pTxBuffer = pTxBuffer;
		pI2CHandle->TxLen = Len;
		pI2CHandle->TxRxState = I2C_BUSY_IN_TX;
		pI2CHandle->DevAddr = SlaveAddr;
		pI2CHandle->Sr = Sr;

		//Implement code to Generate START Condition
		I2C_GenerateSTART(pI2CHandle->pI2Cx, ENABLE);

		//Implement the code to enable ITBUFEN Control Bit
		I2C_ITConfig(pI2CHandle->pI2Cx, I2C_IT_BUF, ENABLE);

		//Implement the code to enable ITEVFEN Control Bit
		I2C_ITConfig(pI2CHandle->pI2Cx, I2C_IT_EVT, ENABLE);

		//Implement the code to enable ITERREN Control Bit
		I2C_ITConfig(pI2CHandle->pI2Cx, I2C_IT_ERR, ENABLE);

	}

	return busystate;

}
/*********************************************************************
 * @fn      		  - I2C_MasterReceiveDataIT
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              - Complete the below code . Also include the fn prototype in header file

 */
uint8_t I2C_MasterReceiveDataIT(I2C_Handle_t *pI2CHandle,uint8_t *pRxBuffer, uint32_t Len, uint8_t SlaveAddr,uint8_t Sr)
{

	uint8_t busystate = pI2CHandle->TxRxState;

	if( (busystate != I2C_BUSY_IN_TX) && (busystate != I2C_BUSY_IN_RX))
	{
		pI2CHandle->pRxBuffer = pRxBuffer;
		pI2CHandle->RxLen = Len;
		pI2CHandle->TxRxState = I2C_BUSY_IN_RX;
		pI2CHandle->RxSize = Len; //Rxsize is used in the ISR code to manage the data reception 
		pI2CHandle->DevAddr = SlaveAddr;
		pI2CHandle->Sr = Sr;

		//Implement code to Generate START Condition
		I2C_GenerateSTART(pI2CHandle->pI2Cx, ENABLE);

		//Implement the code to enable ITBUFEN Control Bit
		I2C_ITConfig(pI2CHandle->pI2Cx, I2C_IT_BUF, ENABLE);

		//Implement the code to enable ITEVFEN Control Bit
		I2C_ITConfig(pI2CHandle->pI2Cx, I2C_IT_EVT, ENABLE);

		//Implement the code to enable ITERREN Control Bit
		I2C_ITConfig(pI2CHandle->pI2Cx, I2C_IT_ERR, ENABLE);
		
	}

	return busystate;
}
/*********************************************************************
 * @fn      		  - I2C_EV_IRQHandling
 *
 * @brief             -
 *
 * @param[in]         -
 * @param[in]         -
 * @param[in]         -
 *
 * @return            -
 *
 * @Note              - Complete the below code . Also include the fn prototype in header file

 */
void I2C_EV_IRQHandling(I2C_Handle_t *pI2CHandle)
{
	//1. Handle cho SB event
	// SB flag chi co the dieu khien o master mode
	if(I2C_CheckEvent(pI2CHandle->pI2Cx, I2C_EVENT_MASTER_MODE_SELECT))
	{
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
		{
			I2C_Send7bitAddress(pI2CHandle->pI2Cx, pI2CHandle->DevAddr << 1, I2C_Direction_Transmitter);
		}
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
		{
			I2C_Send7bitAddress(pI2CHandle->pI2Cx, pI2CHandle->DevAddr << 1, I2C_Direction_Receiver);
		}
	}
	
	//2. Handle cho ADDR event
	if(I2C_CheckEvent(pI2CHandle->pI2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) || 
	   I2C_CheckEvent(pI2CHandle->pI2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) ||
	   I2C_CheckEvent(pI2CHandle->pI2Cx, I2C_EVENT_SLAVE_RECEIVER_ADDRESS_MATCHED) ||
	   I2C_CheckEvent(pI2CHandle->pI2Cx, I2C_EVENT_SLAVE_TRANSMITTER_ADDRESS_MATCHED))
	{
		I2C_ClearAddrFlag(pI2CHandle->pI2Cx);
	}
	
	//3. Handle cho BTF event
	if(I2C_CheckEvent(pI2CHandle->pI2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
	{
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
		{
			if(I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE))
			{
				if(pI2CHandle->TxLen == 0)
				{
					//Dam bao TXE cx dc SET
					//1. Tao ra STOP
					if(pI2CHandle->Sr == I2C_DISABLE_SR)
					{
						I2C_GenerateSTOP(pI2CHandle->pI2Cx, ENABLE);
					}
					//2.Reset I2CHanlde
					I2C_CloseSendData(pI2CHandle);
					//3.Call back Application
					I2C_ApplicationEventCallBack(pI2CHandle, I2C_EV_TX_CMPLT);
				}
			}
		}
	}
	
	// Handle cho BTF trong receive mode
	if(I2C_CheckEvent(pI2CHandle->pI2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED))
	{
		if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
		{
			if(pI2CHandle->RxSize == 1)
			{
				*pI2CHandle->pRxBuffer = I2C_ReceiveData(pI2CHandle->pI2Cx);
				pI2CHandle->RxLen--;
			}
			else if(pI2CHandle->RxSize == 2)
			{
				// Disable ACK
				I2C_AcknowledgeConfig(pI2CHandle->pI2Cx, DISABLE);
				// Generate STOP
				if(pI2CHandle->Sr == I2C_DISABLE_SR)
				{
					I2C_GenerateSTOP(pI2CHandle->pI2Cx, ENABLE);
				}
				// Read data
				*pI2CHandle->pRxBuffer = I2C_ReceiveData(pI2CHandle->pI2Cx);
				pI2CHandle->pRxBuffer++;
				pI2CHandle->RxLen--;
				*pI2CHandle->pRxBuffer = I2C_ReceiveData(pI2CHandle->pI2Cx);
				pI2CHandle->RxLen--;
			}
			
			if(pI2CHandle->RxLen == 0)
			{
				// Close I2C reception
				I2C_CloseReceiveData(pI2CHandle);
				// Call application callback
				I2C_ApplicationEventCallBack(pI2CHandle, I2C_EV_RX_CMPLT);
			}
		}
	}
	
	//4. Handle cho STOPF event
	if(I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_STOPF))
	{
		//Clear STOPF flag
		I2C_ClearStopfFlag(pI2CHandle->pI2Cx);
		
		// Callback application
		I2C_ApplicationEventCallBack(pI2CHandle, I2C_EV_STOPF);
	}
	
	//5. Handle cho TXE event
	if(I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_TXE))
	{
		// Kiem tra I2C o mode master hay ko
		if(pI2CHandle->pI2Cx->SR2 & I2C_SR2_MSL)
		{
			// Truyen Data
			if(pI2CHandle->TxRxState == I2C_BUSY_IN_TX)
			{
				if(pI2CHandle->TxLen > 0)
				{
					I2C_SendData(pI2CHandle->pI2Cx, *pI2CHandle->pTxBuffer);
					
					pI2CHandle->TxLen--;
					
					pI2CHandle->pTxBuffer++;
				}
			}
		}
		else
		{
			// Slave mode transmission
			if(pI2CHandle->pI2Cx->SR2 & I2C_SR2_TRA)
			{
				I2C_ApplicationEventCallBack(pI2CHandle, I2C_EV_DATA_REQ);
			}
		}
	}
	
	//6. Handle cho RXNE event
	if(I2C_GetFlagStatus(pI2CHandle->pI2Cx, I2C_FLAG_RXNE))
	{
		// Kiem tra I2C o mode master hay slave
		if(pI2CHandle->pI2Cx->SR2 & I2C_SR2_MSL)
		{
			// Master mode reception
			if(pI2CHandle->TxRxState == I2C_BUSY_IN_RX)
			{
				if(pI2CHandle->RxSize == 1)
				{
					*pI2CHandle->pRxBuffer = I2C_ReceiveData(pI2CHandle->pI2Cx);
					pI2CHandle->RxLen--;
				}
				else if(pI2CHandle->RxLen == 2)
				{
					// Disable ACK before reading second last byte
					I2C_AcknowledgeConfig(pI2CHandle->pI2Cx, DISABLE);
					// Generate STOP condition
					if(pI2CHandle->Sr == I2C_DISABLE_SR)
					{
						I2C_GenerateSTOP(pI2CHandle->pI2Cx, ENABLE);
					}
				}
				
				if(pI2CHandle->RxLen > 0)
				{
					*pI2CHandle->pRxBuffer = I2C_ReceiveData(pI2CHandle->pI2Cx);
					pI2CHandle->pRxBuffer++;
					pI2CHandle->RxLen--;
				}
				
				if(pI2CHandle->RxLen == 0)
				{
					// Reception complete
					I2C_CloseReceiveData(pI2CHandle);
					I2C_ApplicationEventCallBack(pI2CHandle, I2C_EV_RX_CMPLT);
				}
			}
		}
		else
		{
			// Slave mode reception
			if(!(pI2CHandle->pI2Cx->SR2 & I2C_SR2_TRA))
			{
				I2C_ApplicationEventCallBack(pI2CHandle, I2C_EV_DATA_RCV);
			}
		}
	}
}
void I2C_ERR_IRQHandling(I2C_Handle_t *pI2CHandle)
{
	
}

__weak void I2C_ApplicationEventCallBack(I2C_Handle_t *pI2CHandle, uint8_t event)
{
	
}