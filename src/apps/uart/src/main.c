#include <stdio.h>
#include "board.h"
#include "delay.h"

/**
 * Main application entry point.
 */
int main(){
    // Target board initialization
    boardInitMcu();
    boardInitPeriph(UART_BAUDRATE_115200);
    while(1){
        printf("Hello, world!\n");
        delayMs(1000);
    }
}