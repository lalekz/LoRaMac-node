#include "gpio-board.h"
#include "tremo_gpio.h"
#include "tremo_rcc.h"
//ACHTUNG
//NO SUPPORT FOR AF
//BSR IS ACTUALLY BSRR

gpio_t* get_gpio_address(Gpio_t *obj) {return GPIO_BASE + (obj->portIndex) * 0x400;}

void GpioMcuInit(Gpio_t *obj, PinNames pin, PinModes mode, PinConfigs config, PinTypes type, uint32_t value) {
  gpio_t* gpiox = get_gpio_address(obj);
  uint16_t gpio_pin = 1 << (obj->pinIndex);

  
  switch(mode){
    case PIN_INPUT:
      gpiox->IER |= gpio_pin;
    case PIN_OUTPUT:
      gpiox->OER |= gpio_pin;

    case PIN_ALTERNATE_FCT:
      break;
    case PIN_ANALOGIC:
        gpiox->OER |= gpio_pin;
        gpiox->IER &= ~gpio_pin;
        gpiox->PER &= ~gpio_pin;
      break;
  }
  switch(config) {
    case PIN_PUSH_PULL:
      break;
    case PIN_OPEN_DRAIN:
      gpiox->OTYPER |= gpio_pin;
  }
  switch(type) {
    case PIN_NO_PULL:
      break;
    case PIN_PULL_UP:
      gpiox->PSR |= gpio_pin;
    case PIN_PULL_DOWN:
      gpiox->PER |= gpio_pin;
  }
}

void GpioMcuToggle(Gpio_t *obj) {
  gpio_t* gpiox = get_gpio_address(obj);
  uint16_t gpio_pin = 1 << (obj->pinIndex);
  gpiox->BSR = 0;
  gpiox->BSR |= gpio_pin << ((gpiox->ODR) & gpio_pin ? 16 : 0);
}