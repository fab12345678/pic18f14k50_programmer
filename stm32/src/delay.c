#include "stm32f30x_conf.h"
#include "delay.h"

void delay_us(uint32_t time_us){
  SysTick->LOAD = 8 * time_us-1;
  SysTick->VAL = 0; /* Load the SysTick Counter Value */
  SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; /* Enable SysTick Timer */
  SysTick->CTRL |= (1 << 0) | (1 << 2);

  do {  } while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk)==0);
  SysTick->CTRL  &= ~SysTick_CTRL_ENABLE_Msk; /* Disable SysTick Timer */
  SysTick->VAL = 0; /* Load the SysTick Counter Value */
}

void delay_ms(uint16_t time_ms){
  delay_us(time_ms * 1000);
}