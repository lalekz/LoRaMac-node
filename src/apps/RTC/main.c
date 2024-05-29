#include <stdio.h>
#include "board.h"
#include "rtc.h"

/**
 * Main application entry point.
 */
int main(){
    // Target board initialization
    BoardInitMcu();
    BoardInitPeriph();
}