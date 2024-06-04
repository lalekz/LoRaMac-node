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
#include "sx126x-board.h"
#include "uart.h"
#include <stdio.h>
Gpio_t Led1; //PA4
Gpio_t Led2; //PA5
Uart_t Uart0; //PB0 PB1
Gpio_t LoraRfswCtrl; //PD11
Gpio_t LoraRfswVdd; //PA10

void LoracInit() {
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_LORA, false);
    rcc_rst_peripheral(RCC_PERIPHERAL_LORA, true);
    rcc_rst_peripheral(RCC_PERIPHERAL_LORA, false);
    rcc_enable_peripheral_clk(RCC_PERIPHERAL_LORA, true);

    LORAC->CR0 = 0x00000200; //pins for RF TRx from internal SSP 

    LORAC->SSP_CR0 = 0x07; //8 bit data width
    LORAC->SSP_CPSR = 0x02; //Fsspclkout prescaler = 2

    //wakeup lora 
    //avoid always waiting busy after main reset or soft reset
    if(LORAC->CR1 != 0x80) //select / deselect if not POR_BAT
    {
        delay_us(20);
        LORAC->NSS_CR = 0;
        delay_us(110);
        LORAC->NSS_CR = 1;
    }
  
    LORAC->SSP_CR1 = 0x02;

    NVIC_EnableIRQ(LORA_IRQn);
    SX126xIoInit();
}
void BoardInitMcu() {  
  GpioInit(&Led1, PA_4, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0);
  GpioInit(&Led1, PA_5, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0);
  UartInit(&Uart0, UART_USB_CDC, PB_1, PB_0);
  UartConfig(&Uart0, RX_TX, UART_BAUDRATE_115200, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL);
  rcc_enable_peripheral_clk(RCC_PERIPHERAL_PWR, true);
  rcc_enable_peripheral_clk(RCC_PERIPHERAL_SAC, true);
  LoracInit();
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

