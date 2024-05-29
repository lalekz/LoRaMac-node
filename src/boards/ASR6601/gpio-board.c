#include "gpio-board.h"
#include "tremo_gpio.h"
#include "tremo_rcc.h"
//ACHTUNG
//NO SUPPORT FOR AF
//BSR IS ACTUALLY BSRR

gpio_t* get_gpio_address(uint8_t port_index) {return GPIO_BASE + port_index * 0x400;}
void GpioMcuInit(Gpio_t *obj, PinNames pin, PinModes mode, PinConfigs config, PinTypes type, uint32_t value) {
  
  uint8_t port_index = pin / 15;
  uint8_t pin_index = pin % 15;
  gpio_t* port = get_gpio_address(port_index);
  
  obj->pinIndex = pin_index;
  obj->portIndex = port_index;
  obj->port = port;
  obj->pin = pin;
  obj->pull = type;
  
  if(!(RCC->CGR0 & (RCC_CGR0_IOM0_CLK_EN_MASK >> port_index)))
    RCC->CGR0 |= (RCC_CGR0_IOM0_CLK_EN_MASK >> port_index);
  
  switch(mode){
    case PIN_INPUT:
      port->IER |= 1 << pin_index;
      gpio_set_iomux(port, pin_index, 0);
      
      break;
    case PIN_OUTPUT:
      port->OER |= 1 << pin_index;
      gpio_set_iomux(port, pin_index, 0);
      break;

    case PIN_ALTERNATE_FCT:
      gpio_set_iomux(port, pin_index, value);
      break;
    case PIN_ANALOGIC:
        port->OER |= 1 << pin_index;
        port->IER &= ~(1 << pin_index);
        port->PER &= ~(1 << pin_index);
      break;
  }

  switch(config) {
    case PIN_PUSH_PULL:
      break;
    case PIN_OPEN_DRAIN:
      port->OTYPER |= 1 << pin_index;
      break;
  }

  switch(type) {
    case PIN_NO_PULL:
      break;
    case PIN_PULL_UP:
      port->PSR |= 1 << pin_index;
      port->PER |= 1 << pin_index;
      break;
    case PIN_PULL_DOWN:
      port->PER |= 1 << pin_index;
      break;
  }
}

void GpioMcuToggle(Gpio_t *obj) {
  gpio_toggle(get_gpio_address(obj->portIndex), obj->pinIndex);
}

void GpioMcuWrite(Gpio_t *obj, uint32_t value) {
  gpio_write(get_gpio_address(obj->portIndex), obj->pinIndex, value ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}

uint32_t GpioMcuRead(Gpio_t *obj) {
  return (uint32_t)gpio_read(get_gpio_address(obj->portIndex), obj->pinIndex);
}
