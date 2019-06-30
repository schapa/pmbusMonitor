/*
 * i2c.c
 *
 *  Created on: Jun 30, 2019
 *      Author: shapa
 */
#include <stm32f10x.h>
#include <stm32f10x_i2c.h>
#include <stm32f10x_rcc.h>
#include <stm32f10x_gpio.h>

#include "i2c.h"

#define Timed(x) {volatile int __Timeout = 0xFF; while (x) { if (__Timeout-- == 0) goto errTout;}}


static int start(uint8_t address, uint8_t dir) {
	// Ждем освобождения интерфейса
	Timed (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

	// Выставляем START на шину
	I2C_GenerateSTART(I2C1, ENABLE);
	Timed (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

	// Отправляем адрес ведомого и бит направления
	I2C_Send7bitAddress(I2C1, address, dir);
	Timed (!I2C_CheckEvent(I2C1, dir == I2C_Direction_Transmitter ? I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED : I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	return 0;
errTout:
	return -1;
}

static int write(uint8_t data) {
	I2C_SendData(I2C1, data);
	Timed (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
errTout:
	return -1;
}

static int read_ack(void) {
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	Timed (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
	return I2C_ReceiveData(I2C1);
errTout:
	return -1;
}

static int read_nack(void){
	I2C_AcknowledgeConfig(I2C1, DISABLE);
	Timed (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
	return I2C_ReceiveData(I2C1);
errTout:
	return -1;
}

static void stop(void) {
	I2C_GenerateSTOP(I2C1, ENABLE);
}

int I2C_burst_read(uint8_t dev, uint8_t addr, uint8_t *buff, size_t size) {

}

int I2C_burst_write(uint8_t dev, uint8_t addr, uint8_t *buff, size_t size) {

}


void I2C_init(void) {
	static const I2C_InitTypeDef config = {
		100 * 1000,
		I2C_Mode_I2C,
		I2C_DutyCycle_2,
		0,
		I2C_Ack_Disable,
		I2C_AcknowledgedAddress_7bit
	};
	I2C_DeInit(I2C1);
	I2C_Init(I2C1, (I2C_InitTypeDef*)&config);
    I2C_Cmd(I2C1, ENABLE);
}



