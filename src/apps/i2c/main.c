#include "i2c-board.h"
#include "board.h"
#include "delay.h"
#include "board-periph.h"
#include <stdio.h>

int main()
{
    uint8_t cmd[] = {0x24, 0x00};
    uint8_t buff[] = {0, 0, 0, 0, 0};
    uint8_t i;
    BoardInitMcu();
    BoardInitPeriph();
    DelayMs(10);
    I2cMcuWriteBuffer(&I2c1, 0x44, cmd, 2);
    DelayMs(20);
    I2cMcuReadBuffer(&I2c1, 0x44, buff, 5);
    for(i = 0; i < 2; i++)
        printf("%d ", (buff[i] << 8) | buff[i+1]);
    printf("\n");
}
