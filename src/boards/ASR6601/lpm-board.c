#include "lpm-board.h" 
#include "board.h"
#include "tremo_pwr.h"
#include "tremo_rtc.h"
#include "rtc-board.h"
#include "radio.h"

static uint32_t StopModeDisable = 0;
static uint32_t OffModeDisable = 1;

extern const struct Radio_s Radio;

void LpmSetOffMode(LpmId_t id, LpmSetMode_t mode) {
    CRITICAL_SECTION_BEGIN();
    switch(mode) {
        case LPM_DISABLE:
            OffModeDisable |= (uint32_t)id;
            break;
        case LPM_ENABLE:
            OffModeDisable &= ~( uint32_t )id;
            break;
        default: 
            break;
    }
    CRITICAL_SECTION_END();
}

void LpmSetStopMode(LpmId_t id, LpmSetMode_t mode) {
    CRITICAL_SECTION_BEGIN();
    switch( mode ) {
        case LPM_DISABLE:
            StopModeDisable |= ( uint32_t )id;
            break;
        case LPM_ENABLE:
            StopModeDisable &= ~( uint32_t )id;
            break;
        default:
            break;
    }
    CRITICAL_SECTION_END( );
    return;
}

void LpmEnterLowPower() {
    if( StopModeDisable != 0 ) {
        /*!
        * SLEEP mode is required
        */
        LpmEnterSleepMode( );
        LpmExitSleepMode( );
    }
    else { 
        if( OffModeDisable != 0 ) {
            /*!
            * STOP mode is required
            */
            LpmEnterStopMode( );
            LpmExitStopMode( );
        }
        else {
            /*!
            * OFF mode is required
            */
            LpmEnterOffMode( );
            LpmExitOffMode( );
        }
    }
}

void LpmEnterSleepMode() {
}

void LpmExitSleepMode() {
}

void LpmEnterStopMode() {
    if(Radio.GetStatus() != RF_IDLE)
        return;
    
    rtc_check_syn();
    pwr_deepsleep_wfi(PWR_LP_MODE_STOP3);
}

void LpmExitStopMode() {
}

void LpmEnterOffMode() {
}

void LpmExitOffMode()
{
}
