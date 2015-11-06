
#include "led.h"

//#include "motors.h"

#include <stdbool.h>
//#include "stm32f10x_conf.h"

/*FreeRtos includes*/
#include "FreeRTOS.h"
#include "task.h"

#include "gpio_if.h"

static bool isInit=false;
static unsigned int led_pin[] = {
  [LED_GREEN] = MCU_GREEN_LED_GPIO, 
  [LED_RED]   = MCU_RED_LED_GPIO,
};
static int led_polarity[] = {
  [LED_GREEN] = LED_POL_GREEN, 
  [LED_RED] = LED_POL_RED,
};

//Initialize the green led pin as output
void ledInit()
{
  if(isInit)
    return;
  GPIO_IF_LedConfigure(LED1|LED2|LED3);
  GPIO_IF_LedOff(MCU_ALL_LED_IND);
}

bool ledTest(void)
{
  return isInit;
}

void ledSet(led_t led, bool value) 
{
  if (led>LED_NUM)
    return;

  if (led_polarity[led]==LED_POL_NEG)
    value = !value;
  if(value)
    GPIO_IF_LedOn(led_pin[led]);
  else
    GPIO_IF_LedOff(led_pin[led]);


#ifdef MOTORS_TEST    
  if(led == LED_RED) 
  {
    static int step = 0;
    
    if(!value)
    {
      motorsSetRatio(step, 0x3FFF);
      
      step++;
      if(step>3) step=0;
    }
    else
    {
      motorsSetRatio(0, 0x0000);
      motorsSetRatio(1, 0x0000);
      motorsSetRatio(2, 0x0000);
      motorsSetRatio(3, 0x0000);
    }
  }
#endif
}