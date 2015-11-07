#ifndef __LED_H__
#define __LED_H__

#include <stdbool.h>

//#include "stm32f10x_conf.h"

//Led polarity configuration constant
#define LED_POL_POS 1
#define LED_POL_NEG 0

//Hardware configuration
//#define LED_GPIO_PERIF   RCC_APB2Periph_GPIOB
//#define LED_GPIO_PORT    GPIOB
//#define LED_GPIO_GREEN   GPIO_Pin_5
#define LED_POL_GREEN    LED_POL_POS
//#define LED_GPIO_RED     GPIO_Pin_4
#define LED_POL_RED      LED_POL_POS

#define LED_POL_ORANGE LED_POL_POS

#define LED_NUM 3

typedef enum {LED_RED=0, LED_GREEN, LED_ORANGE=2} led_t;

void ledInit();
bool ledTest();

// Procedures to set the status of the LEDs
void ledSet(led_t led, bool value);

void ledTask(void *param);

//Legacy functions
#define ledSetRed(VALUE) ledSet(LED_RED, VALUE)
#define ledSetGreen(VALUE) ledSet(LED_GREEN, VALUE)
#define ledSetOrange(VALUE) ledSet(LED_ORANGE, VALUE)

#endif
