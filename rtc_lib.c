/*
 * timer_lib.c
 *
 *  Created on: 25 May 2022
 *      Author: sumesh
 */

#include "rtc_lib.h"
#include <msp430.h>
volatile uint32_t RtcSleepTimeInSeconds = 0;
volatile uint32_t RtcSleepReferenceInSeconds = 0xfffffffe;
volatile bool Rtc_timer_ready_flag = false;

void Initialise_rtc_OneSecondTimer(void)
{
    BCSCTL1 |= DIVA_1;                        // ACLK / 2
    BCSCTL3 |= LFXT1S_2;                      // ACLK = VLO
    TACCR0 = 3000; // Delay to allow Ref to settle  // timer interrupt for delay
    TACCTL0 |= CCIE;                          // Compare-mode interrupt.
    TACTL = TASSEL_1 + MC_1; // TACLK = SMCLK, Up mode. ( check if this will affect power modes)
}

void StopTimer(void)
{
    TACCR0 = 0;                     // stop timer
    TACCTL0 &= ~CCIE;               // Compare-mode interrupt.
    RtcSleepTimeInSeconds = 0;
    RtcSleepReferenceInSeconds = 0xfffffffe;
}

void StartTimer(uint32_t Set_timer_in_sec)
{
    Initialise_rtc_OneSecondTimer();
    RtcSleepTimeInSeconds = 0;
    RtcSleepReferenceInSeconds = Set_timer_in_sec;
}

bool SetTimer(uint32_t value)
{
    if(value == 0xfffffffe )
    {
        StopTimer();
        return false;
    }
    else
    {
        StartTimer(value);
        return true;
    }
}

bool Check_if_timer_up(void)
{
    bool resp = Rtc_timer_ready_flag;
    if(resp)
    {
        Rtc_timer_ready_flag = false;
    }
    return resp;
}

uint32_t get_sleep_time_ref(void)
{
    return RtcSleepReferenceInSeconds;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A0_VECTOR
__interrupt void ta0_isr(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A0_VECTOR))) ta0_isr (void)
#else
#error Compiler not supported!
#endif
{
    RtcSleepTimeInSeconds++;
    if(RtcSleepTimeInSeconds > RtcSleepReferenceInSeconds)
    {
        //Rtc_timer_ready_flag = true;
        RtcSleepTimeInSeconds = 0;
        __bic_SR_register_on_exit(LPM3_bits);              // Clear CPUOFF bit from LPM3
    }
}


