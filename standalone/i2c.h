#ifndef __I2C_H__
#define __I2C_H__

#include "stdint.h"
#include "unistd.h"

void I2C_Start(volatile uint32_t *mapped);
void I2C_Stop(volatile uint32_t *mapped);
int I2C_Slave_Ack(volatile uint32_t *mapped);
void I2C_Master_Ack(volatile uint32_t *mapped);
void I2C_Master_No_Ack(volatile uint32_t *mapped);
void I2C_Byte_Send(volatile uint32_t *mapped, uint8_t data);
void Enable_Rj45(volatile uint32_t *mapped, uint32_t index, uint8_t enable);

#endif 					// __I2C_H__		