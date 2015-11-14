/**
 *    ||          ____  _ __                           
 * +------+      / __ )(_) /_______________ _____  ___ 
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie control firmware
 *
 * Copyright (C) 2011-2012 Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * uart.c - uart CRTP link and raw access functions
 */
#include <string.h>

/*ST includes */
//#include "stm32f10x.h"
//#include "stm32f10x_dma.h"
//#include "stm32f10x_rcc.h"
//#include "stm32f10x_usart.h"
//#include "stm32f10x_gpio.h"
#include "hw_types.h"
#include "hw_memmap.h"
#include "prcm.h"
#include "uart.h"
#include "rom.h"
#include "rom_map.h"
#include "uart_if.h"
/*FreeRtos includes*/
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#include "uart_hal.h"
#include "crtp.h"
#include "debug.h"
//#include "nvicconf.h"
#include "config.h"

#if defined(USE_FREERTOS) || defined(USE_TI_RTOS)
#include "osi.h"
#endif

#include "uart_if.h"

#ifndef TRUE
#define TRUE true
#endif

#ifndef FALSE
#define FALSE false
#endif

#define UART_DATA_TIMEOUT_MS 1000
#define UART_DATA_TIMEOUT_TICKS (UART_DATA_TIMEOUT_MS / portTICK_PERIOD_MS)
#define CRTP_START_BYTE 0xAA
#define CCR_ENABLE_SET  ((uint32_t)0x00000001)

static bool isInit = false;

SemaphoreHandle_t waitUntilSendDone = NULL;
static uint8_t outBuffer[64];
static uint8_t dataIndex;
static uint8_t dataSize;
static uint8_t crcIndex = 0;
static bool    isUartDmaInitialized;
static enum { notSentSecondStart, sentSecondStart} txState;
static QueueHandle_t packetDelivery;
static QueueHandle_t uartDataDelivery;
//static DMA_InitTypeDef DMA_InitStructureShare;

void uartRxTask(void *param);

/**
  * Configures the UART DMA. Mainly used for FreeRTOS trace
  * data transfer.
  */
void uartDmaInit(void)
{
  isUartDmaInitialized = TRUE;
}

//*****************************************************************************
//
//! Interrupt handler for UART interupt 
//!
//! \param  None
//!
//! \return None
//!
//*****************************************************************************
static void UARTIntHandler()
{
    uartIsr();
}

void uartInit(void)
{
  MAP_UARTConfigSetExpClk(CONSOLE,MAP_PRCMPeripheralClockGet(CONSOLE_PERIPH), 
                  UART_BAUD_RATE, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
                   UART_CONFIG_PAR_NONE));
  
#if defined(UART_OUTPUT_TRACE_DATA) || defined(ADC_OUTPUT_RAW_DATA)
  uartDmaInit();
#else
  // Configure Tx buffer empty interrupt
  MAP_UARTIntRegister(CONSOLE,UARTIntHandler);
  MAP_UARTIntEnable(CONSOLE,UART_INT_RX);
  
  vSemaphoreCreateBinary(waitUntilSendDone);

  xTaskCreate(uartRxTask, (const signed char * const)"UART-Rx",
              configMINIMAL_STACK_SIZE, NULL, /*priority*/2, NULL);

  packetDelivery = xQueueCreate(2, sizeof(CRTPPacket));
  uartDataDelivery = xQueueCreate(40, sizeof(uint8_t));
#endif
  //Enable it
  //USART_Cmd(UART_TYPE, ENABLE);
  
  isInit = true;
}

bool uartTest(void)
{
  return isInit;
}

void uartRxTask(void *param)
{
  enum {waitForFirstStart, waitForSecondStart,
        waitForPort, waitForSize, waitForData, waitForCRC } rxState;

  uint8_t c;
  uint8_t dataIndex = 0;
  uint8_t crc = 0;
  CRTPPacket p;
  rxState = waitForFirstStart;
  uint8_t counter = 0;
  while(1)
  {
    if (xQueueReceive(uartDataDelivery, &c, UART_DATA_TIMEOUT_TICKS) == pdTRUE)
    {
      counter++;
     /* if (counter > 4)
        ledSetRed(1);*/
      switch(rxState)
      {
        case waitForFirstStart:
          rxState = (c == CRTP_START_BYTE) ? waitForSecondStart : waitForFirstStart;
          break;
        case waitForSecondStart:
          rxState = (c == CRTP_START_BYTE) ? waitForPort : waitForFirstStart;
          break;
        case waitForPort:
          p.header = c;
          crc = c;
          rxState = waitForSize;
          break;
        case waitForSize:
          if (c < CRTP_MAX_DATA_SIZE)
          {
            p.size = c;
            crc = (crc + c) % 0xFF;
            dataIndex = 0;
            rxState = (c > 0) ? waitForData : waitForCRC;
          }
          else
          {
            rxState = waitForFirstStart;
          }
          break;
        case waitForData:
          p.data[dataIndex] = c;
          crc = (crc + c) % 0xFF;
          dataIndex++;
          if (dataIndex == p.size)
          {
            rxState = waitForCRC;
          }
          break;
        case waitForCRC:
          if (crc == c)
          {
            xQueueSend(packetDelivery, &p, 0);
          }
          rxState = waitForFirstStart;
          break;
        default:
          ASSERT(0);
          break;
      }
    }
    else
    {
      // Timeout
      rxState = waitForFirstStart;
    }
  }
}

static int uartReceiveCRTPPacket(CRTPPacket *p)
{
  if (xQueueReceive(packetDelivery, p, portMAX_DELAY) == pdTRUE)
  {
    return 0;
  }

  return -1;
}

static portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
static uint8_t rxDataInterrupt;


void uartIsr(void)
{
  if(UART_INT_EOT == UARTIntStatus(CONSOLE, 1))
  {
    if (dataIndex < dataSize)
    {
      //USART_SendData(UART_TYPE, outBuffer[dataIndex] & 0xFF);
      MAP_UARTCharPut(CONSOLE,outBuffer[dataIndex] & 0xFF);
      dataIndex++;
      if (dataIndex < dataSize - 1 && dataIndex > 1)
      {
        outBuffer[crcIndex] = (outBuffer[crcIndex] + outBuffer[dataIndex]) % 0xFF;
      }
    }
    else
    {
      //USART_ITConfig(UART_TYPE, USART_IT_TXE, DISABLE);
      MAP_UARTIntDisable(CONSOLE,UART_INT_EOT);
      xHigherPriorityTaskWoken = pdFALSE;
      xSemaphoreGiveFromISR(waitUntilSendDone, &xHigherPriorityTaskWoken);
    }
  }
  MAP_UARTIntClear(CONSOLE,UART_INT_EOT);
  
  if(UART_INT_RX == UARTIntStatus(CONSOLE, 1))
  {
    //rxDataInterrupt = USART_ReceiveData(UART_TYPE) & 0xFF;
    MAP_UARTCharGet(CONSOLE);
    xQueueSendFromISR(uartDataDelivery, &rxDataInterrupt, &xHigherPriorityTaskWoken);
  }

}

static int uartSendCRTPPacket(CRTPPacket *p)
{
  outBuffer[0] = CRTP_START_BYTE;
  outBuffer[1] = CRTP_START_BYTE;
  outBuffer[2] = p->header;
  outBuffer[3] = p->size;
  memcpy(&outBuffer[4], p->data, p->size);
  dataIndex = 1;
  txState = notSentSecondStart;
  dataSize = p->size + 5;
  crcIndex = dataSize - 1;
  outBuffer[crcIndex] = 0;

  //USART_SendData(UART_TYPE, outBuffer[0] & 0xFF);
  MAP_UARTCharPut(CONSOLE,outBuffer[0] & 0xFF);
  //USART_ITConfig(UART_TYPE, USART_IT_TXE, ENABLE);
  MAP_UARTIntEnable(CONSOLE,UART_INT_EOT);
  xSemaphoreTake(waitUntilSendDone, portMAX_DELAY);
  
  return 0;
}

static int uartSetEnable(bool enable)
{
  return 0;
}

static struct crtpLinkOperations uartOp =
{
  .setEnable         = uartSetEnable,
  .sendPacket        = uartSendCRTPPacket,
  .receivePacket     = uartReceiveCRTPPacket,
};

struct crtpLinkOperations * uartGetLink()
{
  return &uartOp;
}

void uartDmaIsr(void)
{
  
}

void uartSendData(uint32_t size, uint8_t* data)
{
  uint32_t i;

  for(i = 0; i < size; i++)
  {
    MAP_UARTCharPut(CONSOLE,data[i] & 0xFF);
  }
}

int uartPutchar(int ch)
{
    uartSendData(1, (uint8_t *)&ch);
    
    return (unsigned char)ch;
}

void uartSendDataDma(uint32_t size, uint8_t* data)
{
 
}
