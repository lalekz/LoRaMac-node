/*!
 * \file      spi-board.c
 *
 * \brief     Target board SPI driver implementation
 *
 * \copyright Revised BSD License, see section \ref LICENSE.
 *
 * \code
 *                ______                              _
 *               / _____)             _              | |
 *              ( (____  _____ ____ _| |_ _____  ____| |__
 *               \____ \| ___ |    (_   _) ___ |/ ___)  _ \
 *               _____) ) ____| | | || |_| ____( (___| | | |
 *              (______/|_____)_|_|_| \__)_____)\____)_| |_|
 *              (C)2013-2017 Semtech
 *
 * \endcode
 *
 * \author    Miguel Luis ( Semtech )
 *
 * \author    Gregory Cristian ( Semtech )
 */
#include "board.h"
#include "gpio.h"
#include "spi-board.h"
#include "tremo_spi.h"
#include "tremo_rcc.h"
#include <stdio.h>

ssp_init_t init_structs[2];

ssp_typedef_t* get_ssp_address(SpiId_t spiId) {
  switch(spiId) {
    case SPI_1:
      return SSP1;
    case SPI_2:
      return SSP2;
  }
  return 0;
}

ssp_init_t* get_ssp_init_struct(SpiId_t spiId) {
  return init_structs + (uint8_t)(spiId) * sizeof(ssp_init_t);
}

void ssp_initialize(Spi_t* obj) {
  ssp_typedef_t* ssp = get_ssp_address(obj->SpiId);
  if(!(RCC->CGR0 & (RCC_CGR0_SSP1_CLK_EN_MASK >> (uint8_t)(obj->SpiId))))
    RCC->CGR0 |= RCC_CGR0_SSP1_CLK_EN_MASK >> (uint8_t)(obj->SpiId);
  ssp_init(ssp, get_ssp_init_struct(obj->SpiId));
  if(!(ssp->CR1 & 2))
    ssp->CR1 |= 2;
  
}
void SpiInit( Spi_t *obj, SpiId_t spiId, PinNames mosi, PinNames miso, PinNames sclk, PinNames nss ) {
  ssp_init_t* init_struct = get_ssp_init_struct(spiId);
  obj->SpiId = spiId;
  GpioInit(&obj->Mosi, mosi, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, 4);
  GpioInit(&obj->Miso, miso, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, 4);
  GpioInit(&obj->Sclk, sclk, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, 4);
  GpioInit(&obj->Nss, nss, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, 4);
  ssp_init_struct(init_struct);
}

void SpiDeInit( Spi_t *obj ) {
  ssp_deinit(get_ssp_address(obj->SpiId));
  GpioInit(&obj->Mosi, obj->Mosi.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0);
  GpioInit(&obj->Miso, obj->Miso.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0);
  GpioInit(&obj->Sclk, obj->Sclk.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0);
  GpioInit(&obj->Nss, obj->Nss.pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0);
}

void SpiFormat( Spi_t *obj, int8_t bits, int8_t cpol, int8_t cpha, int8_t slave ) {
  ssp_init_t* init_struct = get_ssp_init_struct(obj->SpiId);
  init_struct->ssp_data_size = bits - 1;
  init_struct->ssp_clk_pol = cpol ? SPI_CLK_POLARITY_HIGH : SPI_CLK_POLARITY_LOW;
  init_struct->ssp_clk_phase = cpha ? SPI_CLK_PHASE_2EDGE : SPI_CLK_PHASE_1EDGE;
  init_struct->ssp_role = slave ? SSP_ROLE_SLAVE : SSP_ROLE_MASTER;
  ssp_initialize(obj);
}

void SpiFrequency( Spi_t *obj, uint32_t hz ) {
  ssp_init_t* init_struct = get_ssp_init_struct(obj->SpiId);
  init_struct->ssp_sclk = hz;
  ssp_initialize(obj);
}

uint16_t SpiInOut( Spi_t *obj, uint16_t outData ) {
  ssp_typedef_t* ssp = get_ssp_address(obj->SpiId);
  while (!(ssp->SR & SSP_FLAG_TX_FIFO_NOT_FULL));
  ssp->DR = outData;
  while (!(ssp->SR & SSP_FLAG_RX_FIFO_NOT_EMPTY));
  return ssp->DR;
}

