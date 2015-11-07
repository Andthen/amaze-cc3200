//*****************************************************************************
//
// Copyright (C) 2014 Texas Instruments Incorporated - http://www.ti.com/ 
// 
// 
//  Redistribution and use in source and binary forms, with or without 
//  modification, are permitted provided that the following conditions 
//  are met:
//
//    Redistributions of source code must retain the above copyright 
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the 
//    documentation and/or other materials provided with the   
//    distribution.
//
//    Neither the name of Texas Instruments Incorporated nor the names of
//    its contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
//  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
//  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
//  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
//  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
//  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
//  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
//  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
//  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//*****************************************************************************

//*****************************************************************************
//
// Application Name     - PWM Application
// Application Overview - The general purpose timers (GPTs) support a 16-bit 
//                        pulse-width modulation (PWM) mode with software-
//                        programmable output inversion of the PWM signal.
//                        The brightness of the LEDs are varied from Off to On 
//                        as PWM output varies.
// Application Details  -
// http://processors.wiki.ti.com/index.php/CC32xx_PWM
// or
// docs\examples\CC32xx_PWM.pdf
//
//*****************************************************************************

//****************************************************************************
//
//! \addtogroup pwm
//! @{
//
//****************************************************************************

// Standard includes.
//#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/* Personal configs */
#include "FreeRTOSConfig.h"

//freertos interface includes
#include "FreeRTOS.h"
#include "task.h"
#include "osi.h"

/* Project includes */
#include "config.h"
#include "system.h"
#include "led.h"

// Driverlib includes
#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_apps_rcm.h"
#include "hw_common_reg.h"
#include "interrupt.h"
#include "rom.h"
#include "rom_map.h"
#include "uart.h"
#include "timer.h"
#include "utils.h"
#include "prcm.h"

// Common interface includes
#include "common.h"
#include "pinmux.h"
#include "i2c_if.h"
#include "uart_if.h"
#include "gpio_if.h"
#include "timer_if.h"

//drivers interface includes
#include "mpu6050.h"
#include "led.h"
#include "ledseq.h"
//hal interface includes
#include "imu.h" //zadd 08311708

//*****************************************************************************
//                      MACRO DEFINITIONS
//*****************************************************************************
#define APPLICATION_VERSION     "1.1.1"
#define DBG_PRINT               Report
#define SPAWN_TASK_PRIORITY     9
#define OSI_STACK_SIZE          2048
#define MAX_MSG_LENGTH			16
#define APPLICATION_VERSION              "1.1.1"
#define APP_NAME                         "cc3200 IMU application"

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

//*****************************************************************************
//                 GLOBAL VARIABLES -- Start
//*****************************************************************************
// The queue used to send strings to the task1.
OsiMsgQ_t MsgQ;

#ifndef USE_TIRTOS
/* in case of TI-RTOS don't include startup_*.c in app project */
#if defined(gcc) || defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif
#if defined(ewarm)
extern uVectorEntry __vector_table;
#endif
#endif
//*****************************************************************************
//                 GLOBAL VARIABLES -- End
//*****************************************************************************



/****************************************************************************/
/*                      LOCAL FUNCTION DEFINITIONS                          */
/****************************************************************************/
static void vTestTask1( void *pvParameters );
static void vTestTask2( void *pvParameters );
static void vTestTask3( void *pvParameters );
static void BoardInit();

#ifdef USE_FREERTOS
//*****************************************************************************
// FreeRTOS User Hook Functions enabled in FreeRTOSConfig.h
//*****************************************************************************

//*****************************************************************************
//
//! \brief Application defined hook (or callback) function - assert
//!
//! \param[in]  pcFile - Pointer to the File Name
//! \param[in]  ulLine - Line Number
//!
//! \return none
//!
//*****************************************************************************
void
vAssertCalled( const char *pcFile, unsigned long ulLine )
{
    //Handle Assert here
    while(1)
    {
    }
}

//*****************************************************************************
//
//! \brief Application defined idle task hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
/*
void
vApplicationIdleHook( void)
{
    //Handle Idle Hook for Profiling, Power Management etc
}
*/
//*****************************************************************************
//
//! \brief Application defined malloc failed hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationMallocFailedHook()
{
    //Handle Memory Allocation Errors
    while(1)
    {
    }
}

//*****************************************************************************
//
//! \brief Application defined stack overflow hook
//!
//! \param  none
//!
//! \return none
//!
//*****************************************************************************
void vApplicationStackOverflowHook( OsiTaskHandle *pxTask,
                                   signed char *pcTaskName)
{
    //Handle FreeRTOS Stack Overflow
    while(1)
    {
    }
}
#endif //USE_FREERTOS

//******************************************************************************
//
//! First test task
//!
//! \param pvParameters is the parameter passed to the task while creating it.
//!
//!    This Function
//!        1. Receive message from the Queue and display it on the terminal.
//!
//! \return none
//
//******************************************************************************
void vTestTask1( void *pvParameters )
{
	char pcMessage[MAX_MSG_LENGTH];
    for( ;; )
    {
    	/* Wait for a message to arrive. */
    	osi_MsgQRead(&MsgQ, pcMessage, OSI_WAIT_FOREVER);

		UART_PRINT("message = ");
		UART_PRINT(pcMessage);
		UART_PRINT("\n\r");
		osi_Sleep(200);
    }
}

//******************************************************************************
//
//! Second test task
//!
//! \param pvParameters is the parameter passed to the task while creating it.
//!
//!    This Function
//!        1. Creates a message and send it to the queue.
//!
//! \return none
//
//******************************************************************************
void vTestTask2( void *pvParameters )
{
   unsigned long ul_2;
   const char *pcInterruptMessage[4] = {"Welcome","to","CC32xx"
           ,"development !\n"};

   ul_2 =0;

   for( ;; )
     {
       /* Queue a message for the print task to display on the UART CONSOLE. */
	   osi_MsgQWrite(&MsgQ, (void*) pcInterruptMessage[ul_2 % 4], OSI_NO_WAIT);
	   ul_2++;        
	   osi_Sleep(200);
     }
}

//******************************************************************************
//
//! Second test task
//!
//! \param pvParameters is the parameter passed to the task while creating it.
//!
//!    This Function
//!        1. Creates a message and send it to the queue.
//!
//! \return none
//
//******************************************************************************

void vTestTask3( void *pvParameters )
{
  
  ledseqInit();
  for( ;; )
  {
//    ledseqRun(LED_ORANGE, seq_charging);
//    ledseqRun(LED_RED, seq_armed);
    ledseqRun(LED_GREEN, seq_testPassed);
    vTaskDelay(M2T(2000));
//    osi_Sleep(2000);
  }
  
}

//*****************************************************************************
//
// Globals used by the timer interrupt handler.
//
//*****************************************************************************
static volatile unsigned long g_ulSysTickValue;
static volatile unsigned long g_ulBase;
static volatile unsigned long g_ulRefBase;
static volatile unsigned long g_ulRefTimerInts = 0;
static volatile unsigned long g_ulIntClearVector;
unsigned long g_ulTimerInts;

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

    g_ulTimerInts ++;
    GPIO_IF_LedToggle(MCU_ORANGE_LED_GPIO);
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
    //
    // Clear the timer interrupt.
    //
    Timer_IF_InterruptClear(g_ulRefBase);

    g_ulRefTimerInts ++;
    //GPIO_IF_LedToggle(MCU_RED_LED_GPIO);
    //imu9Read(&gyro, &acc, &mag);
}
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

void InitTimer(void)
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
    Timer_IF_Start(g_ulBase, TIMER_A, 50);
    Timer_IF_Start(g_ulRefBase, TIMER_A, 2);
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

//*****************************************************************************
//
//! Board Initialization & Configuration
//!
//! \param  None
//!
//! \return None
//
//*****************************************************************************
static void
BoardInit(void)
{
/* In case of TI-RTOS vector table is initialize by OS itself */
#ifndef USE_TIRTOS
  //
  // Set vector table base
  //
#if defined(ccs) || defined(gcc)
    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
#endif
#if defined(ewarm)
    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
#endif
#endif
    //
    // Enable Processor
    //
    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}
//****************************************************************************
//
//! Demonstrates the controlling of LED brightness using PWM
//!
//! \param none
//! 
//! This function  
//!    1. Pinmux the GPIOs that drive LEDs to PWM mode.
//!    2. Initializes the timer as PWM.
//!    3. Loops to continuously change the PWM value and hence brightness 
//!       of LEDs.
//!
//! \return None.
//
//****************************************************************************
void main()
{
    int iLoopCnt;
    long lRetVal = -1;
    int tmpvalue;
    //zadd 08311744
    static Axis3f gyro; // Gyro axis data in deg/s
    static Axis3f acc;  // Accelerometer axis data in mG
    static Axis3f mag;  // Magnetometer axis data in testla
//
    //
    // Board Initialisation
    //
    BoardInit();
    
    //
    // Configure the pinmux settings for the peripherals exercised
    //
    PinMuxConfig();  
    
    ledInit();
    //
    InitTimer();
    //
    // Initialize the PWMs used for driving the LEDs
    //
//    InitPWMModules();

    InitTerm();
   
    //
    // Clearing the terminal
    //
    ClearTerm();
    
    //
    // Diasplay Banner
    //
    DisplayBanner(APP_NAME);
    
    //
    // Creating a queue for 10 elements.
    //
    OsiReturnVal_e osi_retVal;
    osi_retVal = osi_MsgQCreate(&MsgQ, "MSGQ", MAX_MSG_LENGTH, 10);
    if(osi_retVal != OSI_OK)
    {
    	// Queue was not created and must not be used.
    	while(1);
    }

    //
    // Create the Queue Receive task
    //
    osi_TaskCreate( vTestTask1, "TASK1",\
    							OSI_STACK_SIZE, NULL, 1, NULL );

    //
    // Create the Queue Send task
    //
    osi_TaskCreate( vTestTask2, "TASK2",\
    							OSI_STACK_SIZE,NULL, 1, NULL );
    //
    // Create the Queue Send task
    //
    osi_TaskCreate( vTestTask3, "TASK3",\
    							OSI_STACK_SIZE,NULL, 1, NULL );
    //
    // Start the task scheduler
    //
    osi_start();
    
    
    //
    // I2C Init
    //
    lRetVal = I2C_IF_Open(I2C_MASTER_MODE_FST);
    if(lRetVal < 0)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }    
    
    //del08311649 mpu6050Init(I2C1);
    imu6Init();
     if (imu6Test() == !TRUE)
    {
        ERR_PRINT(lRetVal);
        LOOP_FOREVER();
    }
    while(1)
    {
      MAP_UtilsDelay(26666);
      imu9Read(&gyro, &acc, &mag);
      if (imu6IsCalibrated())
      {
        //commanderGetRPY(&eulerRollDesired, &eulerPitchDesired, &eulerYawDesired);
      }
      tmpvalue = 1000 * acc.x;
      if(g_ulRefTimerInts ++ > 50)
      {
        UART_PRINT("accx = %d\n\r", tmpvalue);
        g_ulRefTimerInts = 0;
      }
    }
    
    //never run to this location
    {
      UpdateDutyCycle(TIMERA3_BASE, TIMER_A, (int)abs(tmpvalue)/4);
        //
        // RYB - Update the duty cycle of the corresponding timers.
        // This changes the brightness of the LEDs appropriately.
        // The timers used are as per LP schematics.
        //
        for(iLoopCnt = 0; iLoopCnt < 255; iLoopCnt++)
        {
            UpdateDutyCycle(TIMERA2_BASE, TIMER_B, iLoopCnt);
            UpdateDutyCycle(TIMERA3_BASE, TIMER_B, iLoopCnt);
            
            MAP_UtilsDelay(4000);
        }
        
        for(iLoopCnt = 255; iLoopCnt > 0; iLoopCnt--)
        {
            UpdateDutyCycle(TIMERA2_BASE, TIMER_B, iLoopCnt);
            UpdateDutyCycle(TIMERA3_BASE, TIMER_B, iLoopCnt);
            
            MAP_UtilsDelay(4000);//UtilsDelay(40000) = 3mS
        }    
            
//        vTaskDelay(M2T(50));
    }

    //
    // De-Init peripherals - will not reach here...
    //
    //DeInitPWMModules();
}

//*****************************************************************************
//
// Close the Doxygen group.
//! @}
//
//*****************************************************************************
