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
 * ledseq.h - LED sequence handler
 */

#ifndef __LEDSEQ_H__
#define __LEDSEQ_H__

/* A LED sequence is made of a list of actions. Each action contains the new 
 * state of the LED and either a time to wait before executing the next action 
 * or a command LOOP or STOP.
 *
 * The sequences are stored in a list by priority order ([0] is the highest
 * priority). The list ordered by priority is defined at the beginning of
 * ledseq.c
 *
 * Each sequence effects only one LED. For each LED only the runnable sequence
 * with the highest priority is run.
 */

#include <stdbool.h>
#include <led.h>
#include <stdint.h>
#define LEDSEQ_CHARGE_CYCLE_TIME_500MA  1000
#define LEDSEQ_CHARGE_CYCLE_TIME_MAX    500
//Led sequence action
#define LEDSEQ_WAITMS(X) (X)
#define LEDSEQ_STOP      -1
#define LEDSEQ_LOOP      -2
typedef struct {
  bool value;
  int action;
} ledseq_t;

//Public API
void ledseqInit(void);
bool ledseqTest(void);

void ledseqRun(led_t led, ledseq_t * sequence);
void ledseqStop(led_t led, ledseq_t * sequence);
void ledseqSetTimes(ledseq_t *sequence, uint32_t onTime, uint32_t offTime);

//Existing led sequences
extern ledseq_t seq_armed[];
extern ledseq_t seq_calibrated[];
extern ledseq_t seq_alive[];
extern ledseq_t seq_lowbat[];
extern ledseq_t seq_linkup[];
extern ledseq_t seq_altHold[];
extern ledseq_t seq_charged[];
extern ledseq_t seq_charging[];
extern ledseq_t seq_chargingMax[];
extern ledseq_t seq_bootloader[];
extern ledseq_t seq_testPassed[];\
extern ledseq_t seq_0shot[];
extern ledseq_t seq_1shot[];
extern ledseq_t seq_2shot[];
extern ledseq_t seq_3shot[];
extern ledseq_t seq_4shot[];
extern ledseq_t seq_5shot[];
extern ledseq_t seq_6shot[];
extern ledseq_t seq_7shot[];
extern ledseq_t seq_8shot[];
extern ledseq_t seq_9shot[];
extern ledseq_t seq_10shot[];
extern ledseq_t seq_11shot[];
extern ledseq_t seq_12shot[];

#endif

