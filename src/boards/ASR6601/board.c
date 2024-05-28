#include "board.h"
#include "tremo_uart.h"
#include "tremo_gpio.h"
#include "tremo_rcc.h"
#include "tremo_delay.h"

void BoardInitMcu(){ 
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

void BoardResetMcu( void );

/*!
 * \brief Initializes the boards peripherals.
 */
void BoardInitPeriph( void );

/*!
 * \brief De-initializes the target board peripherals to decrease power
 *        consumption.
 */
void BoardDeInitMcu( void );

/*!
 * \brief Gets the current potentiometer level value
 *
 * \retval value  Potentiometer level ( value in percent )
 */
uint8_t BoardGetPotiLevel( void );

/*!
 * \brief Measure the Battery voltage
 *
 * \retval value  battery voltage in volts
 */
uint32_t BoardGetBatteryVoltage( void );

/*!
 * \brief Get the current battery level
 *
 * \retval value  battery level [  0: USB,
 *                                 1: Min level,
 *                                 x: level
 *                               254: fully charged,
 *                               255: Error]
 */
uint8_t BoardGetBatteryLevel( void );

/*!
 * \brief Get the current MCU temperature in degree celcius * 256
 *
 * \retval temperature * 256
 */
int16_t BoardGetTemperature( void );

/*!
 * Returns a pseudo random seed generated using the MCU Unique ID
 *
 * \retval seed Generated pseudo random seed
 */
uint32_t BoardGetRandomSeed( void );

/*!
 * \brief Gets the board 64 bits unique ID
 *
 * \param [IN] id Pointer to an array that will contain the Unique ID
 */
void BoardGetUniqueId( uint8_t *id );

/*!
 * \brief Manages the entry into ARM cortex deep-sleep mode
 */
void BoardLowPowerHandler( void );

/*!
 * \brief Get the board power source
 *
 * \retval value  power source [0: USB_POWER, 1: BATTERY_POWER]
 */
uint8_t GetBoardPowerSource( void );

/*!
 * \brief Get the board version
 *
 * \retval value  Version
 */
Version_t BoardGetVersion( void );