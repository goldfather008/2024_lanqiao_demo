#ifndef __I2C_H
#define __I2C_H

#include "main.h"

void I2CStart(void);
void I2CStop(void);
unsigned char I2CWaitAck(void);
void I2CSendAck(void);
void I2CSendNotAck(void);
void I2CSendByte(unsigned char cSendByte);
unsigned char I2CReceiveByte(void);
void I2CInit(void);

//data 读取的数据存放地址
//num 数据大小
//addr 地址
void at24c02_read(uint8_t addr, uint8_t* data, uint8_t num);
//data 要写的数据存放地址
//num 数据大小，最多8Byte
//addr 地址
void at24c02_write(uint8_t addr, uint8_t* data, uint8_t num);

#endif
