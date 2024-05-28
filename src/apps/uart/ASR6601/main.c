#include <stdio.h>
#include "board.h"
#include "delay-board.h"

/**
 * Main application entry point.
 */
int main(){
    // Target board initialization
    BoardInitMcu();
    BoardInitPeriph();
    while(1){
        printf("Hello, world!\n");
        DelayMsMcu(1000);
    }
}