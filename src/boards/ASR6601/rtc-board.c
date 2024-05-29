#include <math.h>
#include "tremo_rtc.h"
#include "tremo_rcc.h"
#include "tremo_pwr.h"
#include "utilities.h"
#include "timer.h"
#include "radio.h"
#include "rtc-board.h"
#include <tremo_regs.h>

#define TICKS_PER_SECOND 32768
/*!
 * Number of seconds in a minute
 */
static const uint8_t SecondsInMinute = 60;

/*!
 * Number of seconds in an hour
 */
static const uint16_t SecondsInHour = 3600;

/*!
 * Number of seconds in a day
 */
static const uint32_t SecondsInDay = 86400;

/*!
 * Number of hours in a day
 */
//static const uint8_t HoursInDay = 24;

/*!
 * Number of seconds in a leap year
 */
static const uint32_t SecondsInLeapYear = 31622400;

/*!
 * Number of seconds in a year
 */
static const uint32_t SecondsInYear = 31536000;

/*!
 * Number of days in each month on a normal year
 */
static const uint8_t DaysInMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Number of days in each month on a leap year
 */
static const uint8_t DaysInMonthLeapYear[] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

/*!
 * Flag used to indicates a the MCU has waken-up from an external IRQ
 */
volatile bool NonScheduledWakeUp = false;

/*!
 * RTC timer context
 */
typedef struct RtcCalendar_s {
    rtc_calendar_t CalendarTime;
} RtcCalendar_t;

/*!
 * Current RTC timer context
 */
RtcCalendar_t RtcCalendarContext;

TimerTime_t RtcTimerContext;

/*!
 * \brief Flag to indicate if the timestamp until the next event is long enough
 * to set the MCU into low power mode
 */
static bool RtcTimerEventAllowsLowPower = false;

/*!
 * \brief Flag to disable the low power mode even if the timestamp until the
 * next event is long enough to allow low power mode
 */
static bool LowPowerDisableDuringTask = false;

/*!
 * \brief Indicates if the RTC is already Initialized or not
 */
static bool RtcInitialized = false;

/*!
 * \brief Indicates if the RTC Wake Up Time is calibrated or not
 */
//static bool WakeUpTimeInitialized = false;

/*!
 * \brief Hold the Wake-up time duration in ms
 */
volatile uint32_t McuWakeUpTime = 0;

/*!
 * \brief RTC wakeup time computation
 */
//static void RtcComputeWakeUpTime( void );

/*!
 * \brief Start the RTC Alarm (timeoutValue is in ms)
 */
static void RtcStartWakeUpAlarm(uint32_t timeoutValue);

/*!
 * \brief Converts a RtcCalendar_t value into TimerTime_t value
 *
 * \param[IN/OUT] calendar Calendar value to be converted
 *                         [NULL: compute from "now",
 *                          Others: compute from given calendar value]
 * \retval timerTime New TimerTime_t value
 */
static TimerTime_t RtcConvertCalendarTickToTimerTime(RtcCalendar_t *calendar);

static TimerTime_t RtcConvertMsToTick(TimerTime_t timeoutValue);

//static TimerTime_t RtcConvertTickToMs( TimerTime_t timeoutValue );


/*!
 * \brief Returns the internal RTC Calendar and check for RTC overflow
 *
 * \retval calendar RTC calendar
 */
static RtcCalendar_t RtcGetCalendar();

extern void rtc_check_syn();

void RtcInit() {
    if(RtcInitialized == false) {
        rtc_deinit();
        rcc_enable_peripheral_clk(RCC_PERIPHERAL_RTC, true);

        rtc_calendar_cmd(ENABLE);
        NVIC_EnableIRQ(RTC_IRQn);
        
        RtcInitialized = true;
    }
}

void RtcSetTimeout(uint32_t timeout) {RtcStartWakeUpAlarm(timeout);}

void RtcStopTimeout() {
    rtc_cyc_cmd(DISABLE);
    rtc_config_interrupt(RTC_CYC_IT, DISABLE); // disable
}

TimerTime_t RtcGetTimerContext() {return RtcTimerContext;}

TimerTime_t RtcSetTimerContext() {return RtcGetTimerValue();}

TimerTime_t RtcGetElapsedTime() {return (RtcGetTimerValue() - RtcTimerContext);}

TimerTime_t RtcGetAdjustedTimeoutValue(uint32_t timeout) {
    if( timeout > McuWakeUpTime ) {   // we have waken up from a GPIO and we have lost "McuWakeUpTime" that we need to compensate on next event
        if( NonScheduledWakeUp == true ) {
            NonScheduledWakeUp = false;
            timeout -= McuWakeUpTime;
        }
    }

    if( timeout > McuWakeUpTime ) {   // we don't go in low power mode for delay below 50ms (needed for LEDs)
        if( timeout < 50 ) // 50 ms
            RtcTimerEventAllowsLowPower = false;
        else {
            RtcTimerEventAllowsLowPower = true;
            timeout -= McuWakeUpTime;
        }
    }
    return  timeout;
}

void RtcRecoverMcuStatus() {

}

TimerTime_t RtcGetTimerValue() {return RtcConvertCalendarTickToTimerTime(NULL);}

void BlockLowPowerDuringTask ( bool status )
{
    if( status == true )
    {
        RtcRecoverMcuStatus();
    }
    LowPowerDisableDuringTask = status;
}

void RtcEnterLowPowerStopMode( void )
{
    if(Radio.GetStatus() != RF_IDLE)
        return;
    
    rtc_check_syn();
    pwr_deepsleep_wfi(PWR_LP_MODE_STOP3);
}

//static void RtcComputeWakeUpTime( void )
//{
//    if( WakeUpTimeInitialized == false )
//    {

//        McuWakeUpTime = 2;

//        WakeUpTimeInitialized = true;
//    }
//}

static void RtcStartWakeUpAlarm(uint32_t timeout) {   
    RtcCalendar_t now;
    if( timeout <= 5 )
        timeout = 5;

    // Load the RTC calendar
    now = RtcGetCalendar();

    // Save the calendar into RtcCalendarContext to be able to calculate the elapsed time
    RtcCalendarContext = now;

    rtc_cyc_cmd(DISABLE);
    rtc_config_cyc_max(RtcConvertMsToTick(timeout));
    rtc_config_cyc_wakeup(ENABLE);
    rtc_cyc_cmd(ENABLE);
    rtc_config_interrupt(RTC_CYC_IT, ENABLE);
}

static TimerTime_t RtcConvertCalendarTickToTimerTime(RtcCalendar_t *calendar) {
    TimerTime_t timeCounter = 0;
    RtcCalendar_t now;
    TimerTime_t timeCounterTemp = 0;

    // Passing a NULL pointer will compute from "now" else,
    // compute from the given calendar value
    if(calendar == NULL)
        now = RtcGetCalendar();
    else
        now = *calendar;

    // Years (calculation valid up to year 2099)
    for(int16_t i = 0; i < (now.CalendarTime.year-2000) ; i++) {
        if(( i == 0 ) || ( i % 4 ) == 0)
            timeCounterTemp += (uint32_t)SecondsInLeapYear;
        else
            timeCounterTemp += (uint32_t)SecondsInYear;
    }

    // Months (calculation valid up to year 2099)*/
    if((now.CalendarTime.year % 4) == 0) {
        for( uint8_t i = 0; i < (now.CalendarTime.month - 1); i++)
            timeCounterTemp += (uint32_t)(DaysInMonthLeapYear[i] * SecondsInDay);
    }
    else {
        for( uint8_t i = 0;  i < ( now.CalendarTime.month - 1 ); i++ )
            timeCounterTemp += ( uint32_t )( DaysInMonth[i] * SecondsInDay );
    }

    timeCounterTemp += (TimerTime_t)( now.CalendarTime.second +
                                      ((TimerTime_t)now.CalendarTime.minute * SecondsInMinute) +
                                      ((TimerTime_t)now.CalendarTime.hour * SecondsInHour) +
                                      ((TimerTime_t)((now.CalendarTime.day-1) * SecondsInDay)));

    timeCounter = timeCounterTemp*1000 + now.CalendarTime.subsecond/1000;

    return (timeCounter);
}

TimerTime_t RtcConvertMsToTick(TimerTime_t timeoutValue) {
    return((TimerTime_t)round(((double)timeoutValue) * TICKS_PER_SECOND / 1000));
}

static RtcCalendar_t RtcGetCalendar() {
    RtcCalendar_t now;
    rtc_get_calendar(&now.CalendarTime);
    return(now);
}

/*!
 * \brief RTC IRQ Handler of the RTC Alarm
 */
void RtcOnIrq() {
    uint8_t intr_stat;
    intr_stat =  rtc_get_status(RTC_CYC_SR);
 
    if(intr_stat == true) {
        rtc_cyc_cmd(DISABLE);
        rtc_config_interrupt(RTC_CYC_IT, DISABLE); // disable
        rtc_set_status(RTC_CYC_SR, false); // clear
        
        TimerIrqHandler();
    
        rtc_config_interrupt(RTC_CYC_IT, ENABLE); // enable
    }
}

void RtcProcess()
{
    // Not used on this platform.
}

uint32_t RtcGetMinimumTimeout() {

}

uint32_t RtcGetTimerElapsedTime() {
    return ((RTC->CYC_MAX) - (RTC->CYC_CNT)) * 1000000 / TICKS_PER_SECOND;
}

TimerTime_t RtcTick2Ms(uint32_t tick) {
    return (uint32_t) round((double) tick / TICKS_PER_SECOND * 1000);
}

uint32_t RtcMs2Tick(TimerTime_t milliseconds) {
    return (uint32_t) round(((double)milliseconds) * TICKS_PER_SECOND / 1000);
}

void RtcSetAlarm(uint32_t timeout) {
    uint32_t time_params[] = {0, 0, 0, 0}; //day hour min sec
    uint8_t time_div[] = {24, 60, 60}; //hour min sec div  
    uint8_t mask_params[] = {1, 1, 1}; //day hour min check
    uint8_t dec_shift[] = {24, 20, 18, 14, 11, 7, 4, 0}; //dec data pos
    uint32_t* ptr;
    uint8_t i;
    RtcCalendar_t calendar;
    
    rtc_get_calendar(&calendar);
    ptr = &calendar + 4; //points to day

    if(RtcTick2Ms(timeout) > 1000)
        time_params[4] = (uint32_t)((double)RtcTick2Ms(timeout) / 1000 + *(ptr + 3));

    for(i = 3; i >= 1; i--) {

        if(time_params[i] > time_div[i - 1]) {
            time_params[i - 1] = time_params[i] / time_div[i - 1] + *(ptr + i - 1); //parameter overflow + current parameter
            time_params[i] %= time_div[i - 1];
            mask_params[i - 1] = 0;
        }
        else 
            break;
    }
    
    RTC->ALARM0 = 0;
    for(i = 29; i >= 27; i--)
        RTC->ALARM0 |= mask_params[29 - i] << i;

    for(i = 0; i < 8; i ++)
        RTC->ALARM0 |= (i % 2 ? time_params[i / 2] % 10 : time_params[i / 2] / 10) << dec_shift[i];
        
    RTC->ALARM0 |= RTC_ENABLE_ALARM;
}

void RtcStopAlarm() {rtc_alarm_cmd(0, false);}