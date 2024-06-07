#include "lpm-board.h" 
#include "board.h"
#include "tremo_pwr.h"

static uint32_t StopModeDisable = 0;
static uint32_t OffModeDisable = 0;

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
    pwr_sleep_wfe(1);
}

void LpmExitSleepMode() {
    pwr_exit_lprun_mode();
}

void LpmEnterStopMode() {
    pwr_deepsleep_wfe(PWR_LP_MODE_STOP0);
}

void LpmExitStopMode() {
}

void LpmEnterOffMode() {
    pwr_deepsleep_wfe(PWR_LP_MODE_STANDBY);
}

void LpmExitOffMode()
{
}
