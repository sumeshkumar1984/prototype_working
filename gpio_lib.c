/*
 * gpio_lib.c
 *
 *  Created on: 27 May 2022
 *      Author: kanna
 */
#include "gpio_lib.h"
#include <msp430.h>
#include <stdint.h>
#include "utils.h"

#define LED0_OUT    P1OUT
#define LED0_DIR    P1DIR
#define LED0_PIN    BIT0

uint8_t wakeup = 0;

void disable_wakeup()
{
    P1IE  &= ~BIT4;                            // P1.4 interrupt enabled
    P1IES &= ~BIT4;                            // P1.4 Hi/lo edge
}

void enable_wakeup()
{
    P1IE  |= BIT4;                             // P1.4 interrupt enabled
    P1IES |= BIT4;                            // P1.4 Hi/lo edge
    P1IFG &= ~BIT4;                           // P1.4 IFG cleared
}

uint8_t get_wakeup(void)
{
  return wakeup;
}

void clear_wakeup(void)
{
    wakeup = 0;
    disable_wakeup();
}


void initWakeupPin()
{
    P1DIR &= ~(BIT4);                   // Configure P1.3 as input direction pin
    P1OUT |= BIT4;                      // Configure P1.3 as pulled-up
    P1REN |= BIT4;                      // P1.7 pull-up register enable
    // Disable the GPIO power-on default high-impedance mode to activate
    // previously configured port settings
}

void Set_i2c_Pins()
{
    P1OUT |= 0xC0;                             // P1.6 & P1.7 Pullups
    P1REN |= 0xC0;                            // P1.6 & P1.7 Pullups
    P1DIR |= 0xC0;
}

void initGPIO()
{
    //LEDs
    LED0_OUT &= ~LED0_PIN;
    LED0_DIR |= LED0_PIN;
}

void Toggle_Control_Pin()
{
    LED0_OUT &= ~LED0_PIN;
    delay_s();
    LED0_OUT |= LED0_PIN;
    delay_s();
    LED0_OUT &= ~LED0_PIN;
}

// Port 1 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT1_VECTOR))) Port_1 (void)
#else
#error Compiler not supported!
#endif
{
  wakeup = 1;
  P1IFG &= ~0x10;                                       // P1.4 IFG cleared
__bic_SR_register_on_exit(LPM3_bits | LPM4_bits);       // Clear CPUOFF bit from LPM3
}

