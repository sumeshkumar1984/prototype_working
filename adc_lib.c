/*
 * adc_lib.c
 *
 *  Created on: 26 May 2022
 *      Author: kanna
 */

#include "adc_lib.h"
#include <msp430.h>
#include <stdint.h>


unsigned int measure(int channel)
{
    P1REN &= ~(0x0E);                            // disable any internal pullups
    P1DIR |= ~(0x0E);                            // define pin as inputs
    // Configure ADC A1 pins needed? or not
    ADC10CTL0 &= ~ENC;                        // disable any previous conversions.
    ADC10CTL1 = CONSEQ_0+(INCH_1*channel);    // single channel
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;
    __delay_cycles(100);
    ADC10AE0 = 0x01<<channel;                 // P1.1 ADC option select
    while (ADC10CTL1 & ADC10BUSY);            // Wait if ADC10 core is active
    ADC10CTL0 |= ENC + ADC10SC;               // Sampling and conversion start
     __bis_SR_register(CPUOFF + GIE);          // LPM0, ADC10_ISR will force exit
    //LPM0;
    ADC10AE0 = 0;
    ADC10CTL1 = 0;
    ADC10CTL0 = 0;
    long int adc_v;
    adc_v = ADC10MEM;
    adc_v = ((ADC10MEM * 15)/10);
    return adc_v;
}

unsigned int measure_temp()
{
    ADC10CTL1 = INCH_10 + ADC10DIV_3;         // Temp Sensor ADC10CLK/4
    ADC10CTL0 = SREF_1 + ADC10SHT_3 + REFON + ADC10ON + ADC10IE;
    __delay_cycles(600);
      ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion start
    __bis_SR_register(CPUOFF + GIE);        // LPM0 with interrupts enabled
    //LPM0;
    ADC10AE0 = 0;
    ADC10CTL1 = 0;
    ADC10CTL0 = 0;
    float adc_value;
    adc_value = ((ADC10MEM - 673) * 423) / 1024;
    return (adc_value*10);
}

unsigned int measure_adc1()
{
    return measure(1);
}

unsigned int measure_adc2()
{
    return measure(2);
}

unsigned int measure_adc3()
{
    return measure(3);
}

unsigned int measure_adc4()
{
    return 1212;
}

//******************************************************************************
// ADC Initialization *******************************************************
//******************************************************************************

void deinit_adc(void)
{

}

void configure_adc(void)
{

}

// ADC10 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC10_VECTOR))) ADC10_ISR (void)
#else
#error Compiler not supported!
#endif
{
  __bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
  //  LPM0_EXIT;                                // Exit LPM0 on return
}

