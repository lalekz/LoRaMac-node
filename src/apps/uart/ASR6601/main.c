#include <string.h>
#include "board.h"
#include "delay.h"

/**
 * Main application entry point.
 */
int main( void )
{
    uint8_t i;

    // Target board initialization
    BoardInitMcu( );
    BoardInitPeriph( );

    while( 1 )
    {
        printf("Hello, world!\n");
        DelayMs( 1000 );
    }
}
