#include <stdio.h>
#include "board.h"
#include "delay.h"
#include "gpio.h"
#include "board-periph.h"

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