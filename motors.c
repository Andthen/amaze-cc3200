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
 * motors.c - Motor driver
 *
 * This code mainly interfacing the PWM peripheral lib of ST.
 */

#include <stdbool.h>

#include "motors.h"

// ST lib includes
//#include "stm32f10x_conf.h"

//FreeRTOS includes
#include "FreeRTOS.h"
#include "task.h"

#include "hw_types.h"
#include "rom.h"
#include "rom_map.h"
#include "prcm.h"
#include "hw_memmap.h"
#include "timer.h"

// HW defines
#define MOTORS_GPIO_TIM_PERIF     RCC_APB1Periph_TIM3
#define MOTORS_GPIO_TIM_M1_2      TIM3
#define MOTORS_GPIO_TIM_M1_2_DBG  DBGMCU_TIM3_STOP
#define MOTORS_REMAP              GPIO_PartialRemap_TIM3

#define MOTORS_GPIO_TIM_M3_4_PERIF  RCC_APB1Periph_TIM4
#define MOTORS_GPIO_TIM_M3_4        TIM4
#define MOTORS_GPIO_TIM_M3_4_DBG    DBGMCU_TIM4_STOP

#define MOTORS_GPIO_PERIF         RCC_APB2Periph_GPIOB
#define MOTORS_GPIO_PORT          GPIOB
#define MOTORS_GPIO_M1            GPIO_Pin_1 // T3_CH4
#define MOTORS_GPIO_M2            GPIO_Pin_0 // T3_CH3
#define MOTORS_GPIO_M3            GPIO_Pin_9 // T4_CH4
#define MOTORS_GPIO_M4            GPIO_Pin_8 // T4_CH3

/* Utils Conversion macro */
#ifdef BRUSHLESS_MOTORCONTROLLER
  #define C_BITS_TO_16(X) (0xFFFF * (X - MOTORS_PWM_CNT_FOR_1MS) / MOTORS_PWM_CNT_FOR_1MS)
  #define C_16_TO_BITS(X) (MOTORS_PWM_CNT_FOR_1MS + ((X * MOTORS_PWM_CNT_FOR_1MS) / 0xFFFF))
#else
  #define C_BITS_TO_16(X) ((X)<<(16-MOTORS_PWM_BITS))
  #define C_16_TO_BITS(X) ((X)>>(16-MOTORS_PWM_BITS)&((1<<MOTORS_PWM_BITS)-1))
#endif

const int MOTORS[] = { MOTOR_M1, MOTOR_M2, MOTOR_M3, MOTOR_M4 };
static bool isInit = false;

//
// The PWM works based on the following settings:
//     Timer reload interval -> determines the time period of one cycle
//     Timer match value -> determines the duty cycle 
//                          range [0, timer reload interval]
// The computation of the timer reload interval and dutycycle granularity
// is as described below:
// Timer tick frequency = 80 Mhz = 80000000 cycles/sec
// For a time period of 0.5 ms, 
//      Timer reload interval = 80000000/2000 = 40000 cycles
// To support steps of duty cycle update from [0, 255]
//      duty cycle granularity = ceil(40000/255) = 157
// Based on duty cycle granularity,
//      New Timer reload interval = 255*157 = 40035
//      New time period = 0.5004375 ms
//      Timer match value = (update[0, 255] * duty cycle granularity)
//
#define TIMER_INTERVAL_RELOAD   40035 /* =(255*157) */
#define DUTYCYCLE_GRANULARITY   157

/* Public functions */

//****************************************************************************
//
//! Update the dutycycle of the PWM timer
//!
//! \param ulBase is the base address of the timer to be configured
//! \param ulTimer is the timer to be setup (TIMER_A or  TIMER_B)
//! \param ucLevel translates to duty cycle settings (0:255)
//! 
//! This function  
//!    1. The specified timer is setup to operate as PWM
//!
//! \return None.
//
//****************************************************************************
void UpdateDutyCycle(unsigned long ulBase, unsigned long ulTimer,
                     unsigned char ucLevel)
{
    //
    // Match value is updated to reflect the new dutycycle settings
    //
    MAP_TimerMatchSet(ulBase,ulTimer,(ucLevel*DUTYCYCLE_GRANULARITY));
}

void GetDutyCycle(unsigned long ulBase, unsigned long ulTimer)
{
    //
    // Match value is updated to reflect the new dutycycle settings
    //
    MAP_TimerMatchGet(ulBase,ulTimer);
}

//****************************************************************************
//
//! Setup the timer in PWM mode
//!
//! \param ulBase is the base address of the timer to be configured
//! \param ulTimer is the timer to be setup (TIMER_A or  TIMER_B)
//! \param ulConfig is the timer configuration setting
//! \param ucInvert is to select the inversion of the output
//! 
//! This function  
//!    1. The specified timer is setup to operate as PWM
//!
//! \return None.
//
//****************************************************************************
void SetupTimerPWMMode(unsigned long ulBase, unsigned long ulTimer,
                       unsigned long ulConfig, unsigned char ucInvert)
{
    //
    // Set GPT - Configured Timer in PWM mode.
    //
    MAP_TimerConfigure(ulBase,ulConfig);
    MAP_TimerPrescaleSet(ulBase,ulTimer,0);
    
    //
    // Inverting the timer output if required
    //
    MAP_TimerControlLevel(ulBase,ulTimer,ucInvert);
    
    //
    // Load value set to ~0.5 ms time period
    //
    MAP_TimerLoadSet(ulBase,ulTimer,TIMER_INTERVAL_RELOAD);
    
    //
    // Match value set so as to output level 0
    //
    MAP_TimerMatchSet(ulBase,ulTimer,TIMER_INTERVAL_RELOAD);

}

//****************************************************************************
//
//! Sets up the identified timers as PWM to drive the peripherals
//!
//! \param none
//! 
//! This function sets up the folowing 
//!    1. TIMERA2 (TIMER B) as RED of RGB light
//!    2. TIMERA3 (TIMER B) as YELLOW of RGB light
//!    3. TIMERA3 (TIMER A) as GREEN of RGB light
//!
//! \return None.
//
//****************************************************************************
void InitPWMModules()
{
    //
    // Initialization of timers to generate PWM output
    //
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkEnable(PRCM_TIMERA3, PRCM_RUN_MODE_CLK);

    //
    // TIMERA2 (TIMER B) as RED of RGB light. GPIO 9 --> PWM_5
    //
    SetupTimerPWMMode(TIMERA2_BASE, TIMER_B,
            (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_B_PWM), 1);
    //
    // TIMERA3 (TIMER B) as YELLOW of RGB light. GPIO 10 --> PWM_6
    //
    SetupTimerPWMMode(TIMERA3_BASE, TIMER_A, 
            (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM | TIMER_CFG_B_PWM), 1);
    //
    // TIMERA3 (TIMER A) as GREEN of RGB light. GPIO 11 --> PWM_7
    //
    SetupTimerPWMMode(TIMERA3_BASE, TIMER_B, 
            (TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PWM | TIMER_CFG_B_PWM), 1);

    MAP_TimerEnable(TIMERA2_BASE,TIMER_B);
    MAP_TimerEnable(TIMERA3_BASE,TIMER_A);
    MAP_TimerEnable(TIMERA3_BASE,TIMER_B);
}

//****************************************************************************
//
//! Disables the timer PWMs
//!
//! \param none
//! 
//! This function disables the timers used
//!
//! \return None.
//
//****************************************************************************
void DeInitPWMModules()
{
    //
    // Disable the peripherals
    //
    MAP_TimerDisable(TIMERA2_BASE, TIMER_B);
    MAP_TimerDisable(TIMERA3_BASE, TIMER_A);
    MAP_TimerDisable(TIMERA3_BASE, TIMER_B);
    MAP_PRCMPeripheralClkDisable(PRCM_TIMERA2, PRCM_RUN_MODE_CLK);
    MAP_PRCMPeripheralClkDisable(PRCM_TIMERA3, PRCM_RUN_MODE_CLK);
}


//Initialization. Will set all motors ratio to 0%
void motorsInit()
{
  if (isInit)
    return;

  //Init pwm
  InitPWMModules();
  
  
  isInit = true;
}

bool motorsTest(void)
{
#ifndef BRUSHLESS_MOTORCONTROLLER
  int i;

  for (i = 0; i < sizeof(MOTORS) / sizeof(*MOTORS); i++)
  {
    motorsSetRatio(MOTORS[i], MOTORS_TEST_RATIO);
    vTaskDelay(M2T(MOTORS_TEST_ON_TIME_MS));
    motorsSetRatio(MOTORS[i], 0);
    vTaskDelay(M2T(MOTORS_TEST_DELAY_TIME_MS));
  }
#endif

  return isInit;
}


void motorsSetRatio(int id, uint16_t ratio)
{
  switch(id)
  {
    case MOTOR_M1:
      UpdateDutyCycle(TIMERA2_BASE, TIMER_B, ratio);
//      TIM_SetCompare4(MOTORS_GPIO_TIM_M1_2, C_16_TO_BITS(ratio));
      break;
    case MOTOR_M2:
      UpdateDutyCycle(TIMERA3_BASE, TIMER_B, ratio);  
//      TIM_SetCompare3(MOTORS_GPIO_TIM_M1_2, C_16_TO_BITS(ratio));
      break;
    case MOTOR_M3:
      UpdateDutyCycle(TIMERA3_BASE, TIMER_A, ratio);
//      TIM_SetCompare4(MOTORS_GPIO_TIM_M3_4, C_16_TO_BITS(ratio));
      break;
    case MOTOR_M4:
//      TIM_SetCompare3(MOTORS_GPIO_TIM_M3_4, C_16_TO_BITS(ratio));
      break;
  }
}

int motorsGetRatio(int id)
{
  switch(id)
  {
    case MOTOR_M1:
      GetDutyCycle(TIMERA2_BASE, TIMER_B);
//      return C_BITS_TO_16(TIM_GetCapture4(MOTORS_GPIO_TIM_M1_2));
    case MOTOR_M2:
      GetDutyCycle(TIMERA3_BASE, TIMER_B);
//      return C_BITS_TO_16(TIM_GetCapture3(MOTORS_GPIO_TIM_M1_2));
    case MOTOR_M3:
      GetDutyCycle(TIMERA3_BASE, TIMER_A);
 //     return C_BITS_TO_16(TIM_GetCapture4(MOTORS_GPIO_TIM_M3_4));
    case MOTOR_M4:
//      return C_BITS_TO_16(TIM_GetCapture3(MOTORS_GPIO_TIM_M3_4));
  }

  return -1;
}

#ifdef MOTOR_RAMPUP_TEST
// FreeRTOS Task to test the Motors driver with a rampup of each motor alone.
void motorsTestTask(void* params)
{
  int step=0;
  float rampup = 0.01;

  motorsSetRatio(MOTOR_M4, 1*(1<<8) * 0.0);
  motorsSetRatio(MOTOR_M3, 1*(1<<8) * 0.0);
  motorsSetRatio(MOTOR_M2, 1*(1<<8) * 0.0);
  motorsSetRatio(MOTOR_M1, 1*(1<<8) * 0.0);
  vTaskDelay(M2T(1000));

  while(1)
  {
    vTaskDelay(M2T(100));

    motorsSetRatio(MOTOR_M4, 1*(1<<8) * rampup);
    motorsSetRatio(MOTOR_M3, 1*(1<<8) * rampup);
    motorsSetRatio(MOTOR_M2, 1*(1<<8) * rampup);
    motorsSetRatio(MOTOR_M1, 1*(1<<8) * rampup);

    rampup += 0.001;
    if (rampup >= 0.1)
    {
      if(++step>3) step=0;
      rampup = 0.01;
    }
  }
}
#else
// FreeRTOS Task to test the Motors driver
void motorsTestTask(void* params)
{
  static const int sequence[] = {0.1*(1<<8), 0.15*(1<<8), 0.2*(1<<8), 0.25*(1<<8)};
  int step=0;

  //Wait 3 seconds before starting the motors
  vTaskDelay(M2T(3000));

  while(1)
  {
    motorsSetRatio(MOTOR_M4, sequence[step%4]);
    motorsSetRatio(MOTOR_M3, sequence[(step+1)%4]);
    motorsSetRatio(MOTOR_M2, sequence[(step+2)%4]);
    motorsSetRatio(MOTOR_M1, sequence[(step+3)%4]);

    if(++step>3) step=0;

    vTaskDelay(M2T(1000));
  }
}
#endif

