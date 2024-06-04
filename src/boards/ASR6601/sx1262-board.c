#include <stdlib.h>
#include "delay.h"
#include "tremo_rcc.h"
#include "tremo_gpio.h"
#include "tremo_regs.h"
#include "tremo_delay.h"
#include "tremo_rtc.h"
#include "tremo_it.h"
#include "board-config.h"
#include "delay.h"
#include "radio.h"
#include "sx126x-board.h"
#include <stdio.h>
#define BOARD_TCXO_WAKEUP_TIME 5

extern Gpio_t LoraRfswCtrl;
extern Gpio_t LoraRfswVdd;

uint8_t gPaOptSetting = 0;
RadioOperatingModes_t operatingMode;

uint16_t SspIO(uint16_t outData) {
    uint32_t status;
    uint8_t read_data = 0;

    LORAC->SSP_DR = outData;

    while (1) {
        status = LORAC->SSP_SR;
        if ((status & 0x11) == 0x01)
            break;
    }

    read_data = LORAC->SSP_DR & 0xFF;

    return read_data;
}

uint32_t SX126xGetBoardTcxoWakeupTime() {return BOARD_TCXO_WAKEUP_TIME;}

void SX126xReset() {
    LORAC->CR1 &= ~(1 << 5); // nreset
    delay_us(100);
    LORAC->CR1 |= 1 << 5;    // nreset release
    LORAC->CR1 &= ~(1 << 7); // por release
    LORAC->CR0 |= 1 << 5;    // irq0
    LORAC->CR1 |= 0x1;       // tcxo

    while ((LORAC->SR) & 0x100);
}

void SX126xWaitOnBusy() {
    delay_us(10);
    while ((LORAC->SR) & 0x100);
}

void SX126xWakeup(void) {
    __disable_irq();

    LORAC->NSS_CR = 0;
    delay_us(20);

    SspIO(RADIO_GET_STATUS);
    SspIO(0x00);

    LORAC->NSS_CR = 1;

    // Wait for chip to be ready.
    
    SX126xWaitOnBusy();

    __enable_irq();
}

void SX126xWriteCommand(RadioCommands_t command, uint8_t *buffer, uint16_t size) {
    uint16_t i;
    SX126xCheckDeviceReady();

    LORAC->NSS_CR = 0;

    SspIO((uint8_t)command);

    for (i = 0; i < size; i++)
        SspIO(buffer[i]);

    LORAC->NSS_CR = 1;

    if (command != RADIO_SET_SLEEP)
        SX126xWaitOnBusy();
}

uint8_t SX126xReadCommand(RadioCommands_t command, uint8_t *buffer, uint16_t size) {
    uint16_t i;
    SX126xCheckDeviceReady();

    LORAC->NSS_CR = 0;

    SspIO((uint8_t)command);
    SspIO(0x00);
    for (i = 0; i < size; i++)
        buffer[i] = SspIO(0);

    LORAC->NSS_CR = 1;

    SX126xWaitOnBusy();
    return 0;
}

void SX126xWriteRegisters(uint16_t address, uint8_t *buffer, uint16_t size) {
    uint16_t i;
    SX126xCheckDeviceReady();

    LORAC->NSS_CR = 0;

    SspIO(RADIO_WRITE_REGISTER);
    SspIO((address & 0xFF00) >> 8);
    SspIO(address & 0x00FF);

    for (i = 0; i < size; i++)
        SspIO(buffer[i]);

    LORAC->NSS_CR = 1;

    SX126xWaitOnBusy();
}

void SX126xWriteRegister(uint16_t address, uint8_t value) {
    SX126xWriteRegisters(address, &value, 1);
}

void SX126xReadRegisters(uint16_t address, uint8_t *buffer, uint16_t size) {
    uint16_t i;
    SX126xCheckDeviceReady();

    LORAC->NSS_CR = 0;

    SspIO(RADIO_READ_REGISTER);
    SspIO((address & 0xFF00) >> 8);
    SspIO(address & 0x00FF);
    SspIO(0);
    for (i = 0; i < size; i++)
        buffer[i] = SspIO(0);

    LORAC->NSS_CR = 1;

    SX126xWaitOnBusy();
}

uint8_t SX126xReadRegister(uint16_t address) {
    uint8_t data;
    SX126xReadRegisters(address, &data, 1);
    return data;
}

void SX126xWriteBuffer(uint8_t offset, uint8_t *buffer, uint8_t size) {
    uint8_t i;
    SX126xCheckDeviceReady();

    LORAC->NSS_CR = 0;

    SspIO(RADIO_WRITE_BUFFER);
    SspIO(offset);
    for (i = 0; i < size; i++)
        SspIO(buffer[i]);

    LORAC->NSS_CR = 1;

    SX126xWaitOnBusy();
}

void SX126xReadBuffer(uint8_t offset, uint8_t *buffer, uint8_t size) {
    uint8_t i;
    SX126xCheckDeviceReady();

    LORAC->NSS_CR = 0;

    SspIO(RADIO_READ_BUFFER);
    SspIO(offset);
    SspIO(0);
    for (i = 0; i < size; i++)
        buffer[i] = SspIO(0);
    LORAC->NSS_CR = 1;

    SX126xWaitOnBusy();
}

void SX126xSetRfTxPower(int8_t power) { SX126xSetTxParams(power, RADIO_RAMP_40_US); }

uint8_t SX126xGetPaSelect(uint32_t channel) { return SX1262; }

void SX126xAntSwOn() {
    GpioInit(&LoraRfswVdd, PA_10, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_UP, 0);
}

void SX126xAntSwOff() {
    GpioInit(&LoraRfswVdd, PA_10, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0);
}

uint8_t SX126xGetPaOpt() { return gPaOptSetting; }

void SX126xSetPaOpt(uint8_t opt) {
    if (opt > 3)
        return;
    gPaOptSetting = opt;
}

bool SX126xCheckRfFrequency(uint32_t frequency) {
    // Implement check. Currently all frequencies are supported
    return true;
}

uint8_t SX126xGetDeviceId() {return SX1262;}


uint32_t SX126xGetDio1PinState()
{
    // NC
    return 0;
}

void SX126xSetOperatingMode(RadioOperatingModes_t mode) {operatingMode = mode;}

RadioOperatingModes_t SX126xGetOperatingMode() {return operatingMode;}

void SX126xIoTcxoInit() { /*
    CalibrationParams_t calibParam;

    SX126xSetDio3AsTcxoCtrl( TCXO_CTRL_1_7V, SX126xGetBoardTcxoWakeupTime( ) << 6 ); // convert from ms to SX126x time base
    calibParam.Value = 0x7F;
    SX126xCalibrate( calibParam );
    */
}

void SX126xIoRfSwitchInit() {
    SX126xSetDio2AsRfSwitchCtrl(true);
}

void SX126xIoInit() {
    GpioInit(&LoraRfswCtrl, PD_11, PIN_ALTERNATE_FCT, PIN_PUSH_PULL, PIN_PULL_DOWN, 3);
    GpioInit(&LoraRfswVdd, PA_10, PIN_OUTPUT, PIN_PUSH_PULL, PIN_PULL_DOWN, 0);
}

void SX126xIoIrqInit(DioIrqHandler dioIrq) {
}