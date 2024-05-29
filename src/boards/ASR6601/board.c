#include "board.h"
#include "utilities.h"
#include "core_cm4.h"
#include "core_cmFunc.h"
#include "gpio.h"
#include "uart.h"
#include "board-config.h"
#include "pinName-board.h"
#include "lpm-board.h"
#include "tremo_delay.h"

Gpio_t Led1; //PA4
Gpio_t Led2; //PA5
Gpio_t Uart0Rx; //PB0
Gpio_t Uart0Tx; //PB1

Uart_t Uart0;

void BoardInitMcu() {  
  GpioInit(&Led1, PA_4, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0);
  GpioInit(&Led1, PA_5, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0);
  UartInit(&Uart0, UART_USB_CDC, PB_1, PB_0);
  UartConfig(&Uart0, RX_TX, 115200, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL);
  delay_init();
}

void BoardInitPeriph() {
 // not used
}

void BoardLowPowerHandler() {
  __disable_irq( );
  LpmEnterLowPower( );
  __enable_irq( );
}

void BoardCriticalSectionBegin(uint32_t *mask) {
  *mask = __get_PRIMASK( );
  __disable_irq( );
}

void BoardCriticalSectionEnd(uint32_t *mask) {
  __set_PRIMASK( *mask );
}