#include "board.h"
#include <stdio.h>
void boardInitMcu(){ 
  //used default system configuration
}
//init GPIOB
void boardInitGpio(){
  rcc_enable_peripheral_clk(RCC_PERIPHERAL_GPIOB, true);
  gpio_set_iomux(GPIOB, GPIO_PIN_0, 1);
  gpio_set_iomux(GPIOB, GPIO_PIN_1, 1);
}
//init Uart0 
void boardInitUart(uint32_t baudrate){
  uart_config_t uart_config_var;
  rcc_enable_peripheral_clk(RCC_PERIPHERAL_UART0, true);
  uart_config_init(&uart_config_var);
  uart_config_var.baudrate = baudrate;
  uart_init(UART0, &uart_config_var);
  uart_cmd(UART0, ENABLE);
}
//init Basic Timer0 1ms period 1-pulse
void boardInitTimer(){
  bstimer_init_t bstimer_init_var;
  bstimer_init_var.prescaler = 2399;
  bstimer_init_var.autoreload_preload = false;
  bstimer_init_var.period = 9;
  bstimer_init_var.bstimer_mms = BSTIMER_MMS_ENABLE;
  rcc_enable_peripheral_clk(RCC_PERIPHERAL_BSTIMER0, true);
  bstimer_init(BSTIMER0, &bstimer_init_var);
  BSTIMER0->CR1 |= BSTIMER_CR1_OPM | BSTIMER_CR1_URS; //one pulse and overflow update
  BSTIMER0->EGR |= BSTIMER_EGR_UG; //force update
  BSTIMER0->DIER |= BSTIMER_DIER_UIE; //enable interrupt
  NVIC_EnableIRQ(BSTIMER0_IRQn);
  NVIC_SetPriority(BSTIMER0_IRQn, 2);
}
//init all
void boardInitPeriph(uint32_t baudrate){
  boardInitGpio();
  boardInitUart(baudrate);
  boardInitTimer();
}