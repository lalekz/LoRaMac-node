#include "delay.h"
#include <stdio.h> 

typedef struct DelayDataStruct {
  uint16_t global_delay_time;
  uint8_t global_delay_interrupt_occured;
} DelayData;

DelayData delay_data;
//stop program for duration ms 
void delayMs(uint16_t duration) {
  delay_data.global_delay_time = duration;
  delay_data.global_delay_interrupt_occured = 0;
  while(delay_data.global_delay_time > 0) {
    BSTIMER0->CR1 |= BSTIMER_CR1_CEN; //enable timer
    while(!delay_data.global_delay_interrupt_occured) {
      asm("nop"); //do nothing
    }
    delay_data.global_delay_interrupt_occured = 0;
  }
}

void BSTIMER0_IRQHandler(void) {
  if(BSTIMER0->SR & BSTIMER_SR_UIF) { //update interrupt occured
    delay_data.global_delay_time--;
    delay_data.global_delay_interrupt_occured = 1;
  }
}