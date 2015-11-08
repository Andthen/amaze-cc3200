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
 */

/*
 * ledseq.c - LED sequence handler
 */

#include <stdbool.h>

#include "ledseq.h"

#include "FreeRTOS.h"
#include "timers.h"
#include "semphr.h"

#include <stdbool.h>
#include "led.h"

/* Led sequence priority */
static ledseq_t * sequences[] = {
  seq_testPassed,
  seq_lowbat,
  seq_charged,
  seq_charging,
  seq_chargingMax,
  seq_bootloader,
  seq_armed,
  seq_calibrated,
  seq_alive,
  seq_linkup,
  seq_0shot,
  seq_1shot,
  seq_2shot,
  seq_3shot,
  seq_4shot,
  seq_5shot,
  seq_6shot,
  seq_7shot,
  seq_8shot,
  seq_9shot,
  seq_10shot,
  seq_11shot,
  seq_12shot,
};

/* Led sequences */
ledseq_t seq_lowbat[] = {
  { true, LEDSEQ_WAITMS(1000)},
  {    0, LEDSEQ_LOOP},
};

ledseq_t seq_armed[] = {
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(250)},
  {    0, LEDSEQ_LOOP},
};

ledseq_t seq_calibrated[] = {
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(450)},
  {    0, LEDSEQ_LOOP},
};

ledseq_t seq_alive[] = {
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(1950)},
  {    0, LEDSEQ_LOOP},
};


//TODO: Change, right now is called so fast it looks like seq_lowbat
ledseq_t seq_altHold[] = {
  { true, LEDSEQ_WAITMS(1)},
  {false, LEDSEQ_WAITMS(50)},
  {    0, LEDSEQ_STOP},
};

ledseq_t seq_linkup[] = {
  { true, LEDSEQ_WAITMS(1)},
  {false, LEDSEQ_WAITMS(0)},
  {    0, LEDSEQ_STOP},
};


ledseq_t seq_charged[] = {
  { true, LEDSEQ_WAITMS(1000)},
  {    0, LEDSEQ_LOOP},
};

ledseq_t seq_charging[] = {
  { true, LEDSEQ_WAITMS(200)},
  {false, LEDSEQ_WAITMS(800)},
  {    0, LEDSEQ_LOOP},
};

ledseq_t seq_chargingMax[] = {
  { true, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_WAITMS(400)},
  {    0, LEDSEQ_LOOP},
};

ledseq_t seq_bootloader[] = {
  { true, LEDSEQ_WAITMS(500)},
  {false, LEDSEQ_WAITMS(500)},
  {    0, LEDSEQ_LOOP},
};

ledseq_t seq_testPassed[] = {
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(50)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(50)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(50)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(50)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(50)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(50)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_STOP},
};

ledseq_t seq_0shot[] = {
  { true, LEDSEQ_WAITMS(500)},
  {false, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_STOP},
};

ledseq_t seq_1shot[] = {
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_STOP},
};
ledseq_t seq_2shot[] = {
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_STOP},
};
ledseq_t seq_3shot[] = {
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_STOP},
};
ledseq_t seq_4shot[] = {
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_STOP},
};
ledseq_t seq_5shot[] = {
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_STOP},
};
ledseq_t seq_6shot[] = {
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_STOP},
};
ledseq_t seq_7shot[] = {
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_STOP},
};
ledseq_t seq_8shot[] = {
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_STOP},
};
ledseq_t seq_9shot[] = {
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_STOP},
};
ledseq_t seq_10shot[] = {
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_STOP},
};
ledseq_t seq_11shot[] = {
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_STOP},
};
ledseq_t seq_12shot[] = {
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  { true, LEDSEQ_WAITMS(50)},
  {false, LEDSEQ_WAITMS(100)},
  {false, LEDSEQ_STOP},
};

/* Led sequence handling machine implementation */
#define SEQ_NUM (sizeof(sequences)/sizeof(sequences[0]))

static void runLedseq(TimerHandle_t xTimer);
static int getPrio(ledseq_t *seq);
static void updateActive(led_t led);

//State of every sequence for every led: LEDSEQ_STOP if stopped or the current 
//step
static int state[LED_NUM][SEQ_NUM];
//Active sequence for each led
static int activeSeq[LED_NUM];

static TimerHandle_t timer[LED_NUM];

static SemaphoreHandle_t ledseqSem;

static bool isInit = false;

void ledseqInit()
{
  int i,j;
  
  if(isInit)
    return;
  
  ledInit();
  
  //Initialise the sequences state
  for(i=0; i<LED_NUM; i++) {
    activeSeq[i] = LEDSEQ_STOP;
    for(j=0; j<SEQ_NUM; j++)
      state[i][j] = LEDSEQ_STOP;
  }
  
  //Init the soft timers that runs the led sequences for each leds
  for(i=0; i<LED_NUM; i++)
    timer[i] = xTimerCreate((const signed char *)"ledseqTimer", M2T(1000), pdFALSE, (void*)i, runLedseq);

  vSemaphoreCreateBinary(ledseqSem);
  
  isInit = true;
}

bool ledseqTest(void)
{
  return isInit & ledTest();
}

void ledseqRun(led_t led, ledseq_t *sequence)
{
  int prio = getPrio(sequence);
  
  if(prio<0) return;
  
  xSemaphoreTake(ledseqSem, portMAX_DELAY);
  state[led][prio] = 0;  //Reset the seq. to its first step
  updateActive(led);
  xSemaphoreGive(ledseqSem);
  
  //Run the first step if the new seq is the active sequence
  if(activeSeq[led] == prio)
    runLedseq(timer[led]);
}

void ledseqSetTimes(ledseq_t *sequence, uint32_t onTime, uint32_t offTime)
{
  sequence[0].action = onTime;
  sequence[1].action = offTime;
}

void ledseqStop(led_t led, ledseq_t *sequence)
{
  int prio = getPrio(sequence);
  
  if(prio<0) return;
  
  xSemaphoreTake(ledseqSem, portMAX_DELAY);
  state[led][prio] = LEDSEQ_STOP;  //Stop the seq.
  updateActive(led);
  xSemaphoreGive(ledseqSem);
  
  //Run the next active sequence (if any...)
  runLedseq(timer[led]);
}

/* Center of the led sequence machine. This function is executed by the FreeRTOS
 * timer and runs the sequences
 */
static void runLedseq( TimerHandle_t xTimer )
{
  extern int state[LED_NUM][SEQ_NUM];
  led_t led = (led_t)pvTimerGetTimerID(xTimer);
  ledseq_t *step;
  bool leave=false;

  while(!leave) {
    int prio = activeSeq[led];
  
    if (prio == LEDSEQ_STOP)
      return;
    
    step = &sequences[prio][state[led][prio]];
    
    state[led][prio]++;
    
    xSemaphoreTake(ledseqSem, portMAX_DELAY);
    switch(step->action)
    {
      case LEDSEQ_LOOP:
        state[led][prio] = 0;
        break;
      case LEDSEQ_STOP:
        state[led][prio] = LEDSEQ_STOP;
        updateActive(led);
        break;
      default:  //The step is a LED action and a time
        ledSet(led, step->value);
        if (step->action == 0)
          break;
        xTimerChangePeriod(xTimer, M2T(step->action), 0);
        xTimerStart(xTimer, 0);
        leave=true;
        break;
    }
    xSemaphoreGive(ledseqSem);
  }
}

//Utility functions
static int getPrio(ledseq_t *seq)
{
  int prio;

  //Find the priority of the sequence
  for(prio=0; prio<SEQ_NUM; prio++)
    if(sequences[prio]==seq) return prio;
  
  return -1; //Invalid sequence
}

static void updateActive(led_t led)
{
  int prio;
  
  activeSeq[led]=LEDSEQ_STOP;
  ledSet(led, false);
  
  for(prio=0;prio<SEQ_NUM;prio++)
  {
    if (state[led][prio] != LEDSEQ_STOP)
    {
      activeSeq[led]=prio;
      break;
    }
  }
}

//******************************************************************************
//
//! 3rd test task
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
  uint64_t Timestamp;
  extern uint32_t useconds;
  static uint32_t seq;
  ledseqInit();
  for( ;; )
  {
    seq = (useconds/60)%60;
    switch( seq/10 )
    {
    case 0:ledseqRun(LED_GREEN, seq_testPassed);break;  
    case 1:ledseqRun(LED_GREEN, seq_1shot);break;
    case 2:ledseqRun(LED_GREEN, seq_2shot);break;
    case 3:ledseqRun(LED_GREEN, seq_3shot);break;
    case 4:ledseqRun(LED_GREEN, seq_4shot);break;
    case 5:ledseqRun(LED_GREEN, seq_5shot);break;
    case 6:ledseqRun(LED_GREEN, seq_6shot);break;
    case 7:ledseqRun(LED_GREEN, seq_7shot);break;
    case 8:ledseqRun(LED_GREEN, seq_8shot);break;
    case 9:ledseqRun(LED_GREEN, seq_9shot);break;
    case 10:ledseqRun(LED_GREEN, seq_10shot);break;
    default:break;
    }
    switch( seq%10 )
    {
    //case 0:ledseqRun(LED_ORANGE, seq_0shot);break;
    case 1:ledseqRun(LED_ORANGE, seq_1shot);break;
    case 2:ledseqRun(LED_ORANGE, seq_2shot);break;
    case 3:ledseqRun(LED_ORANGE, seq_3shot);break;
    case 4:ledseqRun(LED_ORANGE, seq_4shot);break;
    case 5:ledseqRun(LED_ORANGE, seq_5shot);break;
    case 6:ledseqRun(LED_ORANGE, seq_6shot);break;
    case 7:ledseqRun(LED_ORANGE, seq_7shot);break;
    case 8:ledseqRun(LED_ORANGE, seq_8shot);break;
    case 9:ledseqRun(LED_ORANGE, seq_9shot);break;
    case 10:ledseqRun(LED_ORANGE, seq_10shot);break;
    default:break;
    }
    seq = (useconds/3600)%12;
     switch( seq )
    {
    //case 0:ledseqRun(LED_RED, seq_0shot);break;
    case 1:ledseqRun(LED_RED, seq_1shot);break;
    case 2:ledseqRun(LED_RED, seq_2shot);break;
    case 3:ledseqRun(LED_RED, seq_3shot);break;
    case 4:ledseqRun(LED_RED, seq_4shot);break;
    case 5:ledseqRun(LED_RED, seq_5shot);break;
    case 6:ledseqRun(LED_RED, seq_6shot);break;
    case 7:ledseqRun(LED_RED, seq_7shot);break;
    case 8:ledseqRun(LED_RED, seq_8shot);break;
    case 9:ledseqRun(LED_RED, seq_9shot);break;
    case 10:ledseqRun(LED_RED, seq_10shot);break;
    case 11:ledseqRun(LED_RED, seq_11shot);break;
    case 12:ledseqRun(LED_RED, seq_12shot);break;
    default:break;
    }
    //Timestamp = usecTimestamp();
    //UART_PRINT("Timestamp = %lld\n\r", Timestamp);
    vTaskDelay(M2T(5000));
//    osi_Sleep(2000);
  }
  
}


