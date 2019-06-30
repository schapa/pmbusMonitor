/*
 * i2c.h
 *
 *  Created on: Mar 18, 2019
 *      Author: shapa
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif


#include <stddef.h>
#include <stdint.h>

void I2C_init(void);

int I2C_single_write(uint8_t dev, uint8_t addr, uint8_t value);
int I2C_single_read(uint8_t dev, uint8_t addr);

int I2C_burst_write(uint8_t dev, uint8_t addr, uint8_t *buff, size_t size);
int I2C_burst_read(uint8_t dev, uint8_t addr, uint8_t *buff, size_t size);

void SMBus_ReadWord(uint8_t slaveAddr, uint16_t* data, uint8_t ReadAddr);

#ifdef __cplusplus
}
#endif
