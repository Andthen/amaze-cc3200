/*
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
 * system.c - Top level module implementation
 */
#define DEBUG_MODULE "SYS"

#include <stdbool.h>

/* FreeRtos includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#include "debug.h"
#include "version.h"
#include "config.h"
#include "param.h"
//#include "log.h"
#include "ledseq.h"
//#include "pm.h"

#include "system.h"
#include "configblock.h"
//#include "worker.h"
//#include "freeRTOSdebug.h"
#include "uart_if.h"
#include "uart_hal.h"
#include "comm.h"
#include "stabilizer.h"
#include "commander.h"
#include "common.h"

//#include "console.h"

#define APPLICATION_VERSION              "1.1.1"
#define APP_NAME                         "IMU application"

/* Private variable */
static bool canFly;

static bool isInit;

/* System wide synchronisation */
SemaphoreHandle_t canStartMutex;

/* Private functions */
static void systemTask(void *arg);

/* Public functions */
void systemLaunch(void)
{
  xTaskCreate(systemTask, (const signed char * const)"SYSTEM",
              2*configMINIMAL_STACK_SIZE, NULL, /*Piority*/2, NULL);

}

//This must be the first module to be initialized!
void systemInit(void)
{
  if(isInit)
    return;

  canStartMutex = xSemaphoreCreateMutex();
  xSemaphoreTake(canStartMutex, portMAX_DELAY);

//  configblockInit();
//  workerInit();
//  adcInit();
  ledseqInit();
//  pmInit();
    
  isInit = true;
}

bool systemTest()
{
  bool pass=isInit;
  
  //pass &= adcTest();
  pass &= ledseqTest();
  //pass &= pmTest();
  //pass &= workerTest();
  
  return pass;
}

//*****************************************************************************
//
//! Application startup display on UART
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
static void
DisplayBanner(char * AppName)
{
    UART_PRINT("\n\n\n\r");
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\t\t     CC3200 %s Application       \n\r", AppName);
    UART_PRINT("\t\t *************************************************\n\r");
    UART_PRINT("\n\n\n\r");
}

/* Private functions implementation */

extern int paramsLen;

void systemTask(void *arg)
{
  bool pass = true;
  
  //Init the high-levels modules
  systemInit();

#ifndef USE_UART_CRTP
#ifdef UART_OUTPUT_TRACE_DATA
  debugInitTrace();
#endif
#ifdef ENABLE_UART
  uartInit();
#endif
#endif //ndef USE_UART_CRTP
  uartInit();
  ClearTerm();
  DisplayBanner(APP_NAME);
  commInit();

  DEBUG_PRINT("Crazyflie is up and running!\n\r");
  DEBUG_PRINT("Build %s:%s (%s) %s\n\r", V_SLOCAL_REVISION,
              V_SREVISION, V_STAG, (V_MODIFIED)?"MODIFIED":"CLEAN");
  //DEBUG_PRINT("I am 0x%X%X%X and I have %dKB of flash!\n",
              //*((int*)(0x1FFFF7E8+8)), *((int*)(0x1FFFF7E8+4)),
              //*((int*)(0x1FFFF7E8+0)), *((short*)(0x1FFFF7E0)));
  DEBUG_PRINT("Revision Number: 0x%X \n\rImplementer Code:0x%X\n\rCPUID:0x%X", 
                             *((int*)(0xE000ED00+0)) & 0x07, 
                             *((int*)(0xE000ED00+3)) & 0xFF,
                             *((int*)(0xE000ED00+0)));
  //0xE000.E000:Core Peripherals base address
  //offset D00h :CPUID CPU ID Base

  //commanderInit();
  //stabilizerInit();
  
  //Test the modules
  pass &= systemTest();
  pass &= commTest();
  //pass &= commanderTest();
  //pass &= stabilizerTest();
  
  //Start the firmware
  if(pass)
  {
    systemStart();
    ledseqRun(LED_RED, seq_alive);
    ledseqRun(LED_GREEN, seq_testPassed);
  }
  else
  {
    if (systemTest())
    {
      while(1)
      {
        ledseqRun(LED_RED, seq_testPassed); //Red passed == not passed!
        vTaskDelay(M2T(2000));
      }
    }
    else
    {
      ledInit();
      ledSet(LED_RED, true);
    }
  }
  
  //workerLoop();
  
  //Should never reach this point!
  while(1)
    vTaskDelay(portMAX_DELAY);
}


/* Global system variables */
void systemStart()
{
  xSemaphoreGive(canStartMutex);
}

void systemWaitStart(void)
{
  //This permits to guarantee that the system task is initialized before other
  //tasks waits for the start event.
  while(!isInit)
    vTaskDelay(2);

  xSemaphoreTake(canStartMutex, portMAX_DELAY);
  xSemaphoreGive(canStartMutex);
}

void systemSetCanFly(bool val)
{
  canFly = val;
}

bool systemCanFly(void)
{
  return canFly;
}

/*System parameters (mostly for test, should be removed from here) */
#if defined(gcc) 
PARAM_GROUP_START(cpu)
PARAM_ADD(PARAM_UINT16 | PARAM_RONLY, flash, 0x1FFFF7E0)
PARAM_ADD(PARAM_UINT32 | PARAM_RONLY, id0, 0x1FFFF7E8+0)
PARAM_ADD(PARAM_UINT32 | PARAM_RONLY, id1, 0x1FFFF7E8+4)
PARAM_ADD(PARAM_UINT32 | PARAM_RONLY, id2, 0x1FFFF7E8+8)
PARAM_GROUP_STOP(cpu)

/* Loggable variables */
LOG_GROUP_START(sys)
LOG_ADD(LOG_INT8, canfly, &canFly)
LOG_GROUP_STOP(sys)
#endif 

