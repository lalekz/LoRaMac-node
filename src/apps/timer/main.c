#include <stdio.h>
#include "board.h"
#include "gpio.h"
#include "timer.h"
#include "tremo_regs.h"

TimerEvent_t timer;

void callback(){
    printf("Hello world!\n");
    TimerSetValue(&timer, 1000);
    TimerStart(&timer);
}

int main(){
    // Target board initialization
    BoardInitMcu();
    BoardInitPeriph();
    TimerInit(&timer, callback);
    TimerSetValue(&timer, 1000);
    TimerStart(&timer);
    printf("timer set: %ld\n", RTC->CYC_MAX);
    while(1){
    }
}