#include "delay-board.h"
#include "tremo_delay.h"

//stop program for duration ms 
void DelayMsMcu(uint32_t ms){
  delay_ms(ms);
}