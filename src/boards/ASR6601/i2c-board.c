#include "board-config.h"
#include "i2c-board.h"
#include "tremo_i2c.h"
#include "board.h"

static I2cAddrSize I2cInternalAddrSize = I2C_ADDR_SIZE_8;

i2c_t* get_i2c_address(I2cId_t i2cId) 
{
  switch(i2cId){
    case I2C_1:
      return I2C1;
    case I2C_2:
      return I2C2;
  }
  return 0;
}

void i2c_wait_receive(i2c_t* i2c) 
{
  while (!i2c_get_flag_status(i2c, I2C_FLAG_RECV_FULL));
  i2c_clear_flag_status(i2c, I2C_FLAG_RECV_FULL);
}

void i2c_wait_send(i2c_t* i2c) 
{
  while (!i2c_get_flag_status(i2c, I2C_FLAG_TRANS_EMPTY));
  i2c_clear_flag_status(i2c, I2C_FLAG_TRANS_EMPTY);
}

void I2cMcuInit(I2c_t *obj, I2cId_t i2cId, PinNames scl, PinNames sda) 
{
  obj->I2cId = i2cId;
  GpioInit(&obj->Scl, scl, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, 3);
  GpioInit(&obj->Sda, sda, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_UP, 3);
}

void I2cMcuFormat( I2c_t *obj, I2cMode mode, I2cDutyCycle dutyCycle, bool I2cAckEnable, I2cAckAddrMode AckAddrMode, uint32_t I2cFrequency ) 
{
  i2c_config_t config;
  i2c_t* i2c = get_i2c_address(obj->I2cId);
  if(!(RCC->CGR0 & (RCC_CGR0_I2C1_CLK_EN_MASK >> (uint8_t)(obj->I2cId)))) {
    RCC->CGR0 |= RCC_CGR0_I2C1_CLK_EN_MASK >> (uint8_t)(obj->I2cId);
  }
  i2c_config_init(&config);
  config.settings.master.speed = I2cFrequency > 100000 ? I2C_SPEED_FAST : I2C_SPEED_STANDARD;
  i2c_init(i2c, &config);

  if(!((i2c->CR) & I2C_CR_TWSI_UNIT_EN_MASK & I2C_CR_SCL_EN_MASK)) {
    
    i2c->CR |= I2C_CR_TWSI_UNIT_EN_MASK | I2C_CR_SCL_EN_MASK;
  }
}

void I2cMcuResetBus( I2c_t *obj ) 
{
  i2c_t* i2c = get_i2c_address(obj->I2cId);
  i2c->CR = I2C_CR_BUS_RESET_REQUEST_MASK;
  i2c->SR = 0;
  while(i2c->SR & I2C_SR_UNIT_BUSY_MASK);
}

void I2cMcuDeInit( I2c_t *obj ) 
{
  i2c_deinit(get_i2c_address(obj->I2cId));
  GpioInit(&obj->Scl, (obj->Scl).pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0);
  GpioInit(&obj->Sda, (obj->Sda).pin, PIN_ANALOGIC, PIN_PUSH_PULL, PIN_NO_PULL, 0);
}

void I2cSetAddrSize( I2c_t *obj, I2cAddrSize addrSize ) 
{
  I2cInternalAddrSize = addrSize;
}

LmnStatus_t I2cMcuWriteBuffer(I2c_t *obj, uint8_t deviceAddr, uint8_t *buffer, uint16_t size) 
{
  uint16_t i;
  i2c_t* i2c = get_i2c_address(obj->I2cId);
  i2c_master_send_start(i2c, deviceAddr, I2C_WRITE);
  i2c_wait_send(i2c);
  
  for(i = 0; i < size; i++) {
    i2c_send_data(i2c, buffer[i]);
    i2c_wait_send(i2c);
  }
  i2c_master_send_stop(i2c); 
  return LMN_STATUS_OK;
}

LmnStatus_t I2cMcuReadBuffer(I2c_t *obj, uint8_t deviceAddr, uint8_t *buffer, uint16_t size) 
{
  uint16_t i;
  i2c_t* i2c = get_i2c_address(obj->I2cId);
  i2c_master_send_start(i2c, deviceAddr, I2C_READ);
  i2c_wait_send(i2c);
  for(i = 0; i < size - 1; i++) {
    i2c_set_receive_mode(i2c, I2C_ACK);
    i2c_wait_receive(i2c);
    buffer[i] = i2c_receive_data(i2c);
  }
  i2c_set_receive_mode(i2c, I2C_NAK);
  i2c_wait_receive(i2c);
  buffer[size - 1] = i2c_receive_data(i2c);
  i2c_master_send_stop(i2c); 
  return LMN_STATUS_OK;
}

LmnStatus_t I2cMcuWriteMemBuffer(I2c_t *obj, uint8_t deviceAddr, uint16_t addr, uint8_t *buffer, uint16_t size) 
{
  uint16_t i;
  i2c_t* i2c = get_i2c_address(obj->I2cId);
  i2c_master_send_start(i2c, deviceAddr, I2C_WRITE);
  i2c_wait_send(i2c);
  i2c_send_data(i2c, (addr >> 8) & 0xFF);
  i2c_wait_send(i2c);
  i2c_send_data(i2c, addr & 0xFF);
  i2c_wait_send(i2c);
  
  for(i = 0; i < size; i++) {
    i2c_send_data(i2c, buffer[i]);
    i2c_wait_send(i2c);
  }
  
  i2c_master_send_stop(i2c);
  return LMN_STATUS_OK;
}

LmnStatus_t I2cMcuReadMemBuffer( I2c_t *obj, uint8_t deviceAddr, uint16_t addr, uint8_t *buffer, uint16_t size ) 
{
  uint16_t i;
  i2c_t* i2c = get_i2c_address(obj->I2cId);
  i2c_master_send_start(i2c, deviceAddr, I2C_WRITE);
  i2c_wait_send(i2c);
  i2c_send_data(i2c, (addr >> 8) & 0xFF);
  i2c_wait_send(i2c);
  i2c_send_data(i2c, addr & 0xFF);
  i2c_wait_send(i2c);
  i2c_master_send_start(i2c, deviceAddr, I2C_READ);
  i2c_wait_send(i2c);
  for(i = 0; i < size - 1; i++) {
    i2c_set_receive_mode(i2c, I2C_ACK);
    i2c_wait_receive(i2c);
    buffer[i] = i2c_receive_data(i2c);
  }
  i2c_set_receive_mode(i2c, I2C_NAK);
  i2c_wait_receive(i2c);
  buffer[size - 1] = i2c_receive_data(i2c);
  i2c_master_send_stop(i2c); 
  return LMN_STATUS_OK;
}

LmnStatus_t I2cMcuWaitStandbyState(I2c_t *obj, uint8_t deviceAddr) 
{
  while(get_i2c_address(obj->I2cId)->SR & I2C_SR_UNIT_BUSY_MASK);
  return LMN_STATUS_OK;
}
