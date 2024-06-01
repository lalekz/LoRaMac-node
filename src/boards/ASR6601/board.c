#include "board.h"
#include "tremo_cm4.h"
#include "tremo_uart.h"
#include "tremo_gpio.h"
#include "tremo_rcc.h"
#include "tremo_delay.h"
#include "utilities.h"
#include "gpio.h"
#include "board-config.h"
#include "lpm-board.h"
#include "rtc-board.h"
#include "uart.h"

Gpio_t Led1; //PA4
Gpio_t Led2; //PA5

Uart_t Uart0;

void BoardInitMcu() {  
  GpioInit(&Led1, PA_4, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0);
  GpioInit(&Led1, PA_5, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0);
  UartInit(&Uart0, UART_USB_CDC, PB_1, PB_0);
  UartConfig(&Uart0, RX_TX, UART_BAUDRATE_115200, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL);
  RtcInit();
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

