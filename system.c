#include <stdbool.h>

/* FreeRtos includes */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "ledseq.h"

#include "system.h"
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
void systemInit(void)
{
  if(isInit)
    return;

  canStartMutex = xSemaphoreCreateMutex();
  xSemaphoreTake(canStartMutex, portMAX_DELAY);
  
  ledseqInit();
  isInit = true;
}
void systemStart()
{
}
void systemWaitStart(void)
{
}
  
void systemSetCanFly(bool val)
{
}
bool systemCanFly(void)
{
}

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
}
