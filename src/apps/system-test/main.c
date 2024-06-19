#include <stdio.h>
#include "board.h"
#include "gpio.h"
#include "timer.h"
#include "delay.h"
#include "board-periph.h"

TimerEvent_t timer;

void callback()
{
    printf("Woke up. Battery level: %d\n", BoardGetBatteryLevel());
    GpioToggle(&Led1);
    TimerSetValue(&timer, 200);
    TimerStart(&timer);
}

int main()
{
    // Target board initialization
    BoardInitMcu();
    BoardInitPeriph();
    printf("Test began\n");
    DelayMs(1000);
    printf("Delay working\n");
    DelayMs(1000);
    TimerInit(&timer, callback);
    TimerSetValue(&timer, 200);
    TimerStart(&timer);
    while(1) {
        BoardLowPowerHandler();
    }
}
