#include "board.h"
#include "tremo_uart.h"
#include "tremo_gpio.h"
#include "tremo_rcc.h"
#include "tremo_delay.h"

void BoardInitMcu(){ 
  //used default system configuration
}
void BoardInitPeriph(void){
  uart_config_t uart_config_var;
  //init GPIO
  rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOB, true); 
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