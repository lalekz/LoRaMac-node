#include "gpio-board.h"
#include "uart-board.h"
#include "spi.h"
#include "i2c.h"

extern Gpio_t Led1; //PA4
extern Gpio_t Led2; //PA5
extern Gpio_t LoraRfswCtrl; //PD11
extern Gpio_t LoraRfswVdd; //PA10

extern Uart_t Uart0; //PB0 PB1
extern Spi_t Spi1; //PB10 PB11 PB8 PB9
extern I2c_t I2c1; //PB14 PB15s