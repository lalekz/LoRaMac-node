#include <stdio.h>
#include "board.h"
#include "gpio.h"
#include "timer.h"

TimerEvent_t timer;

void callback(){
    printf("Hello world!\n");
    TimerSetValue(&timer, 1000);
    TimerStart(&timer);
    BoardLowPowerHandler();
}

int main(){
    // Target board initialization
    BoardInitMcu();
    BoardInitPeriph();
    TimerInit(&timer, callback);
    TimerSetValue(&timer, 1000);
    TimerStart(&timer);
    BoardLowPowerHandler();
    while(1){
    }
}

