#include "board.h"
#include "board-periph.h"
#include "i2c-board.h"
#include "lpm-board.h"
#include "rtc-board.h"
#include "sx126x-board.h"
#include "tremo_cm4.h"
#include "tremo_uart.h"
#include "tremo_gpio.h"
#include "tremo_rcc.h"
#include "tremo_adc.h"
#include "tremo_delay.h"
#include "utilities.h"
#include "gpio.h"
#include "uart.h"

#define ADC_GAIN (1.000 + 0.002 * (((*(uint32_t *)0x10002030) & 0x1fe0) >> 5))
#define ADC_DCO (-0.256 + 0.001 * (((*(uint32_t *)0x10002034) & 0x1ff0) >> 4))
#define BAT_LOW_LEVEL 1.8

Gpio_t Led1; //PA4
Gpio_t Led2; //PA5
Gpio_t Led3; //PA8
Gpio_t LoraRfswCtrl; //PD11
Gpio_t LoraRfswVdd; //PA10
Gpio_t EepromRw; //PC4

Uart_t Uart0; //PB0 PB1
Spi_t Spi1; //PB10 PB11 PB8 PB9
I2c_t I2c1; //PB14 PB15s
I2c_t I2c2; //PC2 PC3

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
  GpioInit(&Led2, PA_5, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0);
  GpioInit(&Led3, PA_8, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0);
  GpioInit(&EepromRw, PC_4, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0);

  UartInit(&Uart0, UART_USB_CDC, PB_1, PB_0);
  UartConfig(&Uart0, RX_TX, UART_BAUDRATE_115200, UART_8_BIT, UART_1_STOP_BIT, NO_PARITY, NO_FLOW_CTRL);

  rcc_enable_peripheral_clk(RCC_PERIPHERAL_PWR, true);
  rcc_enable_peripheral_clk(RCC_PERIPHERAL_SAC, true);
  LoracInit();

  RtcInit();

  SpiInit(&Spi1, SPI_1, PB_10, PB_11, PB_8, PB_9);
  SpiFormat(&Spi1, 8, 0, 0, 0);
  SpiFrequency(&Spi1, 100000);

  delay_init();

  I2cMcuInit(&I2c1, I2C_1, PB_14, PB_15);
  I2cMcuFormat(&I2c1, MODE_I2C, I2C_DUTY_CYCLE_2, true, I2C_ACK_ADD_7_BIT, 100000);
  I2cMcuInit(&I2c2, I2C_2, PC_2, PC_3);
  I2cMcuFormat(&I2c2, MODE_I2C, I2C_DUTY_CYCLE_2, true, I2C_ACK_ADD_7_BIT, 400000);

  rcc_set_adc_clk_source(RCC_ADC_CLK_SOURCE_RCO48M);
  rcc_enable_peripheral_clk(RCC_PERIPHERAL_ADC, true);
  adc_enable_vbat31(true);
  adc_init();
  adc_config_clock_division(20); //sample frequence 150K
  adc_config_sample_sequence(0, 15);
  adc_config_conv_mode(ADC_CONV_MODE_SINGLE);
  adc_enable(true);
}

void BoardInitPeriph() {
 // not used
}

void BoardLowPowerHandler() {
  LpmEnterLowPower();
}

void BoardCriticalSectionBegin(uint32_t *mask) {
  *mask = __get_PRIMASK( );
  __disable_irq( );
}

void BoardCriticalSectionEnd(uint32_t *mask) {
  __set_PRIMASK( *mask );
}

void BoardResetMcu() {NVIC_SystemReset();}

uint8_t BoardGetBatteryLevel() {
  uint8_t bat, i;
  uint16_t data;
  for(i = 0; i < 3; i++) {
    adc_start(true);
    while(!(adc_get_interrupt_status(ADC_ISR_EOC)));
    data = adc_get_data();
  }
  adc_start(false);
  bat = 
    (3.06 * ((1.2 / 4096 * data - ADC_DCO) / ADC_GAIN) - BAT_LOW_LEVEL) / 
    (3.3 - BAT_LOW_LEVEL) * 254;
  return bat ? bat : 1;
}

uint32_t BoardGetRandomSeed() {return (EFC->SN_L) ^ (EFC->SN_H);}

void BoardGetUniqueId(uint8_t *id) {
  uint32_t snl = EFC->SN_L;
  uint32_t snh = EFC->SN_H;
  id[7] = (snl + snl) >> 24;
  id[6] = (snl + snh) >> 16;
  id[5] = (snl + snh) >> 8;
  id[4] = (snl + snh);
  id[3] = snl >> 24;
  id[2] = snl >> 16;
  id[1] = snl >> 8;
  id[0] = snl;
}

