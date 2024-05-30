#include <stdio.h>
#include "board.h"
#include "delay.h"
#include "gpio.h"

/**
 * Main application entry point.
 */
extern Gpio_t Led1;

int main(){
    // Target board initialization
    BoardInitMcu();
    BoardInitPeriph();
    while(1){
        printf("Hello, world!\n");
        GpioToggle(&Led1);
        DelayMs(1000);
    }
}