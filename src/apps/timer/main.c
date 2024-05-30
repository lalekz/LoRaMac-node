#include <stdio.h>
#include "board.h"
#include "gpio.h"
#include "timer.h"

int main(){
    // Target board initialization
    BoardInitMcu();
    BoardInitPeriph();
    while(1){
        printf("Hello, world!\n");
        
    }
}