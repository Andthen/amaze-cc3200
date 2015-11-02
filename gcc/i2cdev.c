#include <stdint.h>
#include <stdbool.h>

#include "i2cdev.h"
#include "i2c_if.h"
/*
void mpu6050Init(I2C_TypeDef *i2cPort)
{
  if (isInit)
    return;
  
  I2Cx = i2cPort;
  devAddr = MPU6050_ADDRESS_AD0_HIGH;
  
  isInit = TRUE;
}*/



bool i2cdevRead(I2C_TypeDef *I2Cx, uint8_t devAddress, uint8_t memAddress,
               uint16_t len, uint8_t *data)
{
  bool status = TRUE;

  if (memAddress != I2CDEV_NO_MEM_ADDR)
  {
    //status = I2C_Master_BufferWrite(I2Cx, &memAddress,  1, INTERRUPT, devAddress << 1, I2C_TIMEOUT);
    status =  1 + I2C_IF_Write(devAddress, &memAddress, 1, 0);
  }
  if (status)
  {
    //TODO: Fix DMA transfer if more then 3 bytes
    //status = I2C_Master_BufferRead(I2Cx, (uint8_t*)data,  len, INTERRUPT, devAddress << 1, I2C_TIMEOUT);
    status =  1 + I2C_IF_Read(devAddress, data, len);
  }

  return status;
}




bool i2cdevReadByte(I2C_TypeDef *I2Cx, uint8_t devAddress, uint8_t memAddress,
                    uint8_t *data)
{
  return i2cdevRead(I2Cx, devAddress, memAddress, 1, data);
}

bool i2cdevReadBit(I2C_TypeDef *I2Cx, uint8_t devAddress, uint8_t memAddress,
                     uint8_t bitNum, uint8_t *data)
{
  uint8_t byte;
  bool status;
  
  status = i2cdevRead(I2Cx, devAddress, memAddress, 1, &byte);
  *data = byte & (1 << bitNum);

  return status;
}

bool i2cdevReadBits(I2C_TypeDef *I2Cx, uint8_t devAddress, uint8_t memAddress,
                    uint8_t bitStart, uint8_t length, uint8_t *data)
{
  bool status;
  uint8_t byte;

  if ((status = i2cdevReadByte(I2Cx, devAddress, memAddress, &byte)) == TRUE)
  {
      uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
      byte &= mask;
      byte >>= (bitStart - length + 1);
      *data = byte;
  }
  return status;
}



bool i2cdevWrite(I2C_TypeDef *I2Cx, uint8_t devAddress, uint8_t memAddress,
                uint16_t len, uint8_t *data)
{
  bool status;
  static uint8_t buffer[17];
  int i;

  if (memAddress != I2CDEV_NO_MEM_ADDR)
  {
    // Sorry ...
    if (len > 16) len = 16;

    if(len == 0) return 0;

    buffer[0] = memAddress;
    for(i = 0; i < len ; i++)
      buffer[i + 1] = data[i];

//    status = I2C_Master_BufferWrite(I2Cx, buffer,  len + 1, INTERRUPT, devAddress << 1, I2C_TIMEOUT);
      status = 1 + I2C_IF_Write(devAddress, buffer, len + 1, 1);
  }
  else
  {
//    status = I2C_Master_BufferWrite(I2Cx, data,  len, INTERRUPT, devAddress << 1, I2C_TIMEOUT);
    status = 1 + I2C_IF_Write(devAddress, data, len, 1);
  }

  return status;
}

bool i2cdevWriteByte(I2C_TypeDef *I2Cx, uint8_t devAddress, uint8_t memAddress,
                    uint8_t data)
{
  return i2cdevWrite(I2Cx, devAddress, memAddress, 1, &data);
}

bool i2cdevWriteBit(I2C_TypeDef *I2Cx, uint8_t devAddress, uint8_t memAddress,
                    uint8_t bitNum, uint8_t data)
{
    uint8_t byte;
    i2cdevReadByte(I2Cx, devAddress, memAddress, &byte);
    byte = (data != 0) ? (byte | (1 << bitNum)) : (byte & ~(1 << bitNum));
    return i2cdevWriteByte(I2Cx, devAddress, memAddress, byte);
}

bool i2cdevWriteBits(I2C_TypeDef *I2Cx, uint8_t devAddress, uint8_t memAddress,
                     uint8_t bitStart, uint8_t length, uint8_t data)
{
  bool status;
  uint8_t byte;

  if ((status = i2cdevReadByte(I2Cx, devAddress, memAddress, &byte)) == TRUE)
  {
      uint8_t mask = ((1 << length) - 1) << (bitStart - length + 1);
      data <<= (bitStart - length + 1); // shift data into correct position
      data &= mask; // zero all non-important bits in data
      byte &= ~(mask); // zero all important bits in existing byte
      byte |= data; // combine data with existing byte
      status = i2cdevWriteByte(I2Cx, devAddress, memAddress, byte);
  }

  return status;
}
