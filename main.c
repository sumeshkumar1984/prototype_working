/* --COPYRIGHT--,BSD_EX
 * Copyright (c) 2012, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************************
 * 
 *                       MSP430 CODE EXAMPLE DISCLAIMER
 *
 * MSP430 code examples are self-contained low-level programs that typically
 * demonstrate a single peripheral function or device feature in a highly
 * concise manner. For this the code may rely on the device's power-on default
 * register values and settings such as the clock configuration and care must
 * be taken when combining code from several examples to avoid potential side
 * effects. Also see www.ti.com/grace for a GUI- and www.ti.com/msp430ware
 * for an API functional library-approach to peripheral configuration.
 *
 * --/COPYRIGHT--*/
//******************************************************************************
//  MSP430G2xx2 - I2C Slave Receiver / Slave Transmitter, multiple bytes
//
//  Description: I2C Master communicates with I2C Slave using
//  the USI. Master data should increment from 0x55 with each transmitted byte.
//  ACLK = n/a, MCLK = SMCLK = Calibrated 1MHz
//
//  ***THIS IS THE SLAVE CODE***
//
//                  Slave                      Master
//                                    (MSP430G2xx2_usi_12.c)
//            MSP430G2xx2          MSP430G2xx2
//             -----------------          -----------------
//         /|\|              XIN|-    /|\|              XIN|-
//          | |                 |      | |                 |
//          --|RST          XOUT|-     --|RST          XOUT|-
//            |                 |        |                 |
//      LED <-|P1.0             |        |                 |
//            |                 |        |             P1.0|-> LED
//            |         SDA/P1.7|------->|P1.6/SDA         |
//            |         SCL/P1.6|<-------|P1.7/SCL         |
//
//  Note: internal pull-ups are used in this example for SDA & SCL
//
//  D. Dang
//  Texas Instruments Inc.
//  December 2010
//  Built with CCS Version 4.2.0 and IAR Embedded Workbench Version: 5.10
//******************************************************************************

#include <msp430.h>
#include <i2c_lib.h>
#include <stdbool.h>
#include <stdint.h>

uint32_t get_sleep_time_ref()
{
  return 16632154;
}

bool SetTimer(uint32_t timer)
{
  uint32_t blue = timer+5;
  if(blue > 0)
  return true;
  else
  return false;
}

typedef enum {
    INIT            = 0x00,
    DEEP_SLEEP     = INIT + 1,
    ACTIVATION     = DEEP_SLEEP + 1,
    NORMAL_SLEEP   = ACTIVATION + 1,
    TOTAL_RULES    = NORMAL_SLEEP + 1,

} RuleType;

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog
  if (CALBC1_1MHZ==0xFF)					// If calibration constant erased
  {											
    while(1);                               // do not load, trap CPU!!	
  }
  DCOCTL = 0;                               // Select lowest DCOx and MODx settings
  BCSCTL1 = CALBC1_1MHZ;                    // Set DCO
  DCOCTL = CALDCO_1MHZ;
  Setup_USI_Slave();
  RuleType response;
  LPM0;
  while(1)
  {
     i2c_commands i2c_cmd = (i2c_commands) get_i2c_cmd();
     if (i2c_cmd != 0)
     {
         if (i2c_cmd < CMD_TIMER_STATUS_VAL)
         {
             int reading = 0;
             switch (i2c_cmd)
             {
             case (CMD_READ_ADC_1):
                 reading = 1234;
                 break;
             case (CMD_READ_ADC_4):
                 reading = 5678;
                 break;
             case (CMD_READ_ADC_5):
                 reading = 9875;
                 break;
             case (CMD_READ_ADC_6):
                 reading = 5252;
                 break;
             case (CMD_TEMP_SENS):
                 reading = 5151;
                 break;
             default:
                 __no_operation();
                 break;
             }
             set_i2c_resp_int(reading);
         }
         else if (i2c_cmd == CMD_TIMER_STATUS_VAL)
         {
             set_i2c_resp_ui32(get_sleep_time_ref());
         }
     }

     i2c_cmd = (i2c_commands) get_i2c_read_cmd();
     if (i2c_cmd >= CMD_TIMER_CNTRL)
     {
         switch (i2c_cmd)
         {
         case (CMD_TIMER_CNTRL):
             if (!SetTimer(get_timer_value()))
             {
                 response = DEEP_SLEEP;
             }
             break;
         case (CMD_SLP_GO):
             if (sleep_instruction() == 0x05)
             {
                 response = DEEP_SLEEP;  // This is to set to deepsleep
             }
             else if (sleep_instruction() == 0x0A)
             {
                 response = NORMAL_SLEEP;
             }
             break;

         default:
             __no_operation();
             break;
         }

         finished_acting_on_received();
     }
     LPM0;

  }    // CPU off, await USI interrupt
  
 // __no_operation();
}














