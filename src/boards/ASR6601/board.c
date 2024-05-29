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

Gpio_t Led1;    // Active Low
Gpio_t Led2; // Active Low

void BoardInitMcu() { 
  uart_config_t uart_config_var;
  //init GPIO
  rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOA, true); 
  rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOB, true); 
  rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOD, true); 
  gpio_set_iomux(GPIOB, GPIO_PIN_0, 1);
  gpio_set_iomux(GPIOB, GPIO_PIN_1, 1);
  //init UART0
  rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
  uart_config_init(&uart_config_var);
  uart_config_var.baudrate = UART_BAUDRATE_115200;
  uart_init(UART0, &uart_config_var);
  uart_cmd(UART0, ENABLE);
  //init SysTick for delay
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