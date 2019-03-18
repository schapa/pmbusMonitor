/*
 * i2c.c
 *
 *  Created on: Mar 18, 2019
 *      Author: shapa
 */

#include "i2c.h"
#include "bsp.h"
#include "system.h"

#include "stm32f10x_i2c.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "dbg_base.h"
#if 01
#include "dbg_trace.h"
#endif

#define I2C_TOUT 0xFF

static struct {
    uint8_t adr;
    uint8_t *buff;
    size_t size;
    size_t curPos;
    _Bool busy;
    _Bool err;
} s_i2c;


static void initI2cNvic(void) {
	static const NVIC_InitTypeDef event = {
		I2C1_EV_IRQn,
		8, 0,
		ENABLE
	};
	static const NVIC_InitTypeDef error = {
		I2C1_ER_IRQn,
		8, 0,
		ENABLE
	};
	NVIC_Init((NVIC_InitTypeDef*)&event);
	NVIC_Init((NVIC_InitTypeDef*)&error);
}

void I2C_init(void) {
	static const I2C_InitTypeDef config = {
		100 * 1000,
		I2C_Mode_I2C,
		I2C_DutyCycle_2,
		0x0,
		I2C_Ack_Disable,
		I2C_AcknowledgedAddress_7bit
	};
	I2C_DeInit(I2C1);
	I2C_Init(I2C1, (I2C_InitTypeDef*)&config);
    I2C_Cmd(I2C1, ENABLE);
    initI2cNvic();

//	I2C_ITConfig(I2C1, I2C_IT_BUF, ENABLE);
//	I2C_ITConfig(I2C1, I2C_IT_EVT, ENABLE);
//	I2C_ITConfig(I2C1, I2C_IT_ERR, ENABLE);
}
_Bool BSP_i2c_test(int devAdr, uint8_t *buff, size_t size) {
//    if (s_i2c.busy)
//        return false;
    DBGMSG_L("0x%02X", devAdr >> 1);
    memset(&s_i2c, 0, sizeof(s_i2c));
    s_i2c.adr = devAdr;
    s_i2c.buff = buff;
    s_i2c.size = size;
    s_i2c.busy = true;
	_Bool res = false;
	do {
	    int tout = I2C_TOUT;
	    while (--tout && I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
        if (!tout)
            break;
		I2C_GenerateSTART(I2C1, ENABLE);
		while (s_i2c.busy)
		    __WFI();
//		tout = I2C_TOUT;
//		I2C_Send7bitAddress(I2C1, devAdr << 1, I2C_Direction_Transmitter);
//		while (--tout && !I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
//		if (!tout)
//			break;
//	    tout = I2C_TOUT;
//	    I2C_GenerateSTOP(I2C1, ENABLE);
//	    while (--tout && I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
//		res = true;
		res = !s_i2c.err;
	} while (0);

	return res;
}

void I2C1_EV_IRQHandler(void) {
    uint32_t sr = I2C1->SR1;
    sr |= I2C1->SR2 << 16;
    if ((sr & I2C_EVENT_MASTER_MODE_SELECT) == I2C_EVENT_MASTER_MODE_SELECT) { // EV5
        I2C1->DR = s_i2c.adr;
        return;
    }
    if ((sr & I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) == I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) { // EV6
        if (s_i2c.curPos < s_i2c.size)
            I2C1->DR = s_i2c.buff[s_i2c.curPos++];
        else {
            I2C_GenerateSTOP(I2C1, ENABLE);
            s_i2c.busy = false;
        }
        return;
    }

    if ((sr & I2C_EVENT_MASTER_BYTE_TRANSMITTED) == I2C_EVENT_MASTER_BYTE_TRANSMITTED) { // EV8_2
        DBGMSG_L("TRD");
//        return;
    }
    if ((sr & I2C_EVENT_MASTER_BYTE_TRANSMITTING) == I2C_EVENT_MASTER_BYTE_TRANSMITTING) { // EV8
        if (s_i2c.curPos < s_i2c.size) {
            I2C1->DR = s_i2c.buff[s_i2c.curPos++];
            DBGMSG_L("TRS");
        } else if (s_i2c.curPos == s_i2c.size) {
            s_i2c.curPos = s_i2c.size +1;
            DBGMSG_L("TRS STP");
            I2C_GenerateSTOP(I2C1, ENABLE);
            s_i2c.busy = false;
        }
        return;
    }
	char flags[128];
	int occup = 0;
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_DUALF))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "DUALF ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_SMBHOST))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "SMBHOST ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_SMBDEFAULT))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "SMBDEFAULT ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_GENCALL))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "GENCALL ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_TRA))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "TRA ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "BUSY ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_MSL)) {
		occup += snprintf(flags + occup, sizeof(flags) - occup, "MSL ");
//		I2C_ClearFlag(I2C1, I2C_FLAG_MSL);
	}
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_SMBALERT))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "SMBALERT ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_TIMEOUT))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "TIMEOUT ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_PECERR))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "PECERR ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_OVR))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "OVR ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_AF))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "AF ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_ARLO))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "ARLO ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_BERR))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "BERR ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "TXE ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "RXNE ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "STOPF ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_ADD10))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "ADR10 ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "BTF ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "ADDR ");
	if (occup)
	    DBGMSG_L(flags);
	else
	    DBGMSG_L("sr 0x%08X", sr);
}

void I2C1_ER_IRQHandler(void) {
    uint32_t sr = I2C1->SR1;
    sr |= I2C1->SR2 << 16;
    if ((sr & I2C_EVENT_SLAVE_ACK_FAILURE) == I2C_EVENT_SLAVE_ACK_FAILURE) { // EV3_2
        I2C_ClearFlag(I2C1, I2C_FLAG_AF);
        I2C_GenerateSTOP(I2C1, ENABLE);
        s_i2c.err = true;
        s_i2c.busy = false;
        return;
    }
	char flags[128];
	int occup = 0;
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_DUALF))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "DUALF ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_SMBHOST))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "SMBHOST ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_SMBDEFAULT))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "SMBDEFAULT ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_GENCALL))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "GENCALL ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_TRA))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "TRA ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "BUSY ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_MSL))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "MSL ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_SMBALERT))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "SMBALERT ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_TIMEOUT))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "TIMEOUT ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_PECERR))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "PECERR ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_OVR))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "OVR ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_AF))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "AF ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_ARLO))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "ARLO ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_BERR))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "BERR ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_TXE))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "TXE ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_RXNE))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "RXNE ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_STOPF))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "STOPF ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_ADD10))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "ADR10 ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_BTF))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "BTF ");
	if (I2C_GetFlagStatus(I2C1, I2C_FLAG_ADDR))
		occup += snprintf(flags + occup, sizeof(flags) - occup, "ADDR ");
	DBGMSG_WARN(flags);
}

void I2C_single_write(uint8_t HW_address, uint8_t addr, uint8_t data) {
	I2C_GenerateSTART(I2C1, ENABLE);
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	I2C_Send7bitAddress(I2C1, HW_address, I2C_Direction_Transmitter);
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	I2C_SendData(I2C1, addr);
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	I2C_SendData(I2C1, data);
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	I2C_GenerateSTOP(I2C1, ENABLE);
		while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
}

uint8_t I2C_single_read(uint8_t HW_address, uint8_t addr) {
	uint8_t data;
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	I2C_GenerateSTART(I2C1, ENABLE);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	I2C_Send7bitAddress(I2C1, HW_address, I2C_Direction_Transmitter);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	I2C_SendData(I2C1, addr);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	I2C_GenerateSTART(I2C1, ENABLE);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	I2C_Send7bitAddress(I2C1, HW_address, I2C_Direction_Receiver);
	while(!I2C_CheckEvent(I2C1,I2C_EVENT_MASTER_BYTE_RECEIVED));
	data = I2C_ReceiveData(I2C1);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
	I2C_AcknowledgeConfig(I2C1, DISABLE);
	I2C_GenerateSTOP(I2C1, ENABLE);
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	return data;
}


void I2C_burst_write(uint8_t HW_address, uint8_t addr, uint8_t *data, uint8_t n_data) {
	I2C_GenerateSTART(I2C1, ENABLE);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	I2C_Send7bitAddress(I2C1, HW_address, I2C_Direction_Transmitter);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	I2C_SendData(I2C1, addr);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	while(n_data--) {
		I2C_SendData(I2C1, *data++);
			while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	}
	I2C_GenerateSTOP(I2C1, ENABLE);
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
}
void I2C_burst_read(uint8_t HW_address, uint8_t addr, uint8_t *data, uint8_t n_data) {
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
	I2C_GenerateSTART(I2C1, ENABLE);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	I2C_Send7bitAddress(I2C1, HW_address, I2C_Direction_Transmitter);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
	I2C_SendData(I2C1, addr);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	I2C_GenerateSTOP(I2C1, ENABLE);
	I2C_GenerateSTART(I2C1, ENABLE);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	I2C_Send7bitAddress(I2C1, HW_address, I2C_Direction_Receiver);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
	I2C_AcknowledgeConfig(I2C1, ENABLE);
	while(n_data--) {
		if(!n_data)
			I2C_AcknowledgeConfig(I2C1, DISABLE);
		while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
		*data++ = I2C_ReceiveData(I2C1);
	}
	I2C_AcknowledgeConfig(I2C1, DISABLE);
	I2C_GenerateSTOP(I2C1, ENABLE);
	while(I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
}

#define SMBus_Max_Delay_Cycles 0xFF
void SMBus_ReadWord(uint8_t slaveAddr, uint16_t* data, uint8_t ReadAddr)
{
	uint8_t buff[2];
	uint32_t counter = SMBus_Max_Delay_Cycles;

    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY) && counter) --counter;
    if(counter == 0) {
	    return;
    }

    I2C_GenerateSTART(I2C1, ENABLE);

    counter = SMBus_Max_Delay_Cycles;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) && counter) --counter;
    if(counter == 0) {
	    return;
    }
    I2C_Send7bitAddress(I2C1, slaveAddr, I2C_Direction_Transmitter);
    counter = SMBus_Max_Delay_Cycles;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) && counter) --counter;
    if(counter == 0) {
	    return;
    }

    I2C_Cmd(I2C1, ENABLE);

    I2C_SendData(I2C1, ReadAddr);
    counter = SMBus_Max_Delay_Cycles;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED) && counter) --counter;
    if(counter == 0) {
	    return;
    }

    I2C_GenerateSTART(I2C1, ENABLE);
    counter = SMBus_Max_Delay_Cycles;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT) && counter) --counter;
    if(counter == 0) {
	    return;
    }

    I2C_Send7bitAddress(I2C1, slaveAddr, I2C_Direction_Receiver);
    counter = SMBus_Max_Delay_Cycles;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) && counter) --counter;
    if(counter == 0) {
	    return;
    }

    counter = SMBus_Max_Delay_Cycles;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) && counter) --counter;
    if(counter == 0) {
	    return;
    }
    buff[0] = I2C_ReceiveData(I2C1);
    counter = SMBus_Max_Delay_Cycles;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) && counter) --counter;
    if(counter == 0) {
	    return;
    }
    buff[1] = I2C_ReceiveData(I2C1);
    counter = SMBus_Max_Delay_Cycles;
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED) && counter) --counter;
    if(counter == 0) {
	    return;
    }
    I2C_ReceiveData(I2C1);
    I2C_GenerateSTOP(I2C1, ENABLE);
    *data = ((uint16_t)buff[1] << 8) | buff[0];
}

