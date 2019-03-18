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


#include <stdint.h>

void I2C_init(void);

void I2C_single_write(uint8_t HW_address, uint8_t addr, uint8_t data);
uint8_t I2C_single_read(uint8_t HW_address, uint8_t addr);

void I2C_burst_write(uint8_t HW_address, uint8_t addr, uint8_t *data, uint8_t n_data);
void I2C_burst_read(uint8_t HW_address, uint8_t addr, uint8_t *data, uint8_t n_data);


void SMBus_ReadWord(uint8_t slaveAddr, uint16_t* data, uint8_t ReadAddr);

#ifdef __cplusplus
}
#endif
