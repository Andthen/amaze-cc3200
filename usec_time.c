/**
 *    ||          ____  _ __                           
 * +------+      / __ )(_) /_______________ _____  ___ 
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie control firmware
 *
 * Copyright (C) 2011-2013 Bitcraze AB
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
 *
 * usec_time.c - microsecond-resolution timer and timestamps.
 */

#include "stdint.h"
#include "hw_types.h"
#include "prcm.h"
#include "rom_map.h"
#include "hw_memmap.h"
#include "timer.h"
#include "common.h"
#include "timer_if.h"
#include "gpio_if.h"
#include "usec_time.h"

//#include "nvicconf.h"
//#include "stm32f10x.h"

uint32_t usecTimerHighCount;

static volatile unsigned long g_ulSysTickValue;
static volatile unsigned long g_ulBase;
static volatile unsigned long g_ulRefBase;
static volatile unsigned long g_ulRefTimerInts = 0;
static volatile unsigned long g_ulIntClearVector;
unsigned long g_ulTimerInts;
uint32_t useconds;
uint32_t uminutes;
uint32_t uhours;

//*****************************************************************************
//
//! The interrupt handler for the first timer interrupt.
//!
//! \param  None
//!
//! \return none
//
//*****************************************************************************
void
TimerBaseIntHandler(void)
{
    //
    // Clear the timer interrupt.
    //
    Timer_IF_InterruptClear(g_ulBase);
    usecTimerHighCount ++;
    //g_ulTimerInts ++;
}

//*****************************************************************************
//
//! The interrupt handler for the second timer interrupt.
//!
//! \param  None
//!
//! \return none
//
//*****************************************************************************
void
TimerRefIntHandler(void)
{
    static uint64_t Timestamp;
    //
    // Clear the timer interrupt.
    //
    Timer_IF_InterruptClear(g_ulRefBase);

    //g_ulRefTimerInts ++;
    useconds++;
    //useconds = useconds%60;
    uminutes = (useconds/60)%60;
    uhours = (useconds/3600)%24;
    Timestamp = usecTimestamp();
    UART_PRINT("Timestamp = %lld\n\r", Timestamp);
    //GPIO_IF_LedToggle(MCU_RED_LED_GPIO);
    //imu9Read(&gyro, &acc, &mag);
}
void initUsecTimer(void)
{
    //
    // Base address for first timer
    //
    g_ulBase = TIMERA0_BASE;
    //
    // Base address for second timer
    //
    g_ulRefBase = TIMERA1_BASE;
    //
    // Configuring the timers
    //
    
    //
    usecTimerHighCount = 0;
    Timer_IF_Init(PRCM_TIMERA0, g_ulBase, TIMER_CFG_PERIODIC, TIMER_A, 0);
    Timer_IF_Init(PRCM_TIMERA1, g_ulRefBase, TIMER_CFG_PERIODIC, TIMER_A, 0);

    //
    // Setup the interrupts for the timer timeouts.
    //
    Timer_IF_IntSetup(g_ulBase, TIMER_A, TimerBaseIntHandler);
    Timer_IF_IntSetup(g_ulRefBase, TIMER_A, TimerRefIntHandler);

    //
    // Turn on the timers feeding values in mSec
    //
    //Timer_IF_Start(g_ulBase, TIMER_A, 1);
    /*******/
    MAP_TimerLoadSet(g_ulBase,TIMER_A,0xFFFFFFFF);//53 second
    //
    // Enable the GPT 
    //
    MAP_TimerEnable(g_ulBase,TIMER_A);
    /*******/
    Timer_IF_Start(g_ulRefBase, TIMER_A, 1000);
  /*
  usecTimerHighCount = 0;

  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  //Enable the Timer
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

  //Timer configuration
  TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
  TIM_TimeBaseStructure.TIM_Prescaler = 72;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

  NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = NVIC_TRACE_TIM_PRI;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  DBGMCU_Config(DBGMCU_TIM1_STOP, ENABLE);
  TIM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM1, ENABLE);
  */
}

uint64_t usecTimestamp(void)
{
  uint32_t high0;
//  __atomic_load(&usecTimerHighCount, &high0, __ATOMIC_SEQ_CST);
  high0 = usecTimerHighCount;
  uint32_t low = Timer_IF_GetCount(g_ulBase, TIMER_A);
  uint32_t high;
//  __atomic_load(&usecTimerHighCount, &high, __ATOMIC_SEQ_CST);
  high = usecTimerHighCount;
  // There was no increment in between
  if (high == high0)
  {
    return (((uint64_t)high) << 32) + low;
  }
  // There was an increment, but we don't expect another one soon
  return (((uint64_t)high) << 32) + Timer_IF_GetCount(g_ulBase, TIMER_A);
}
