/*
 * utils.c
 *
 *  Created on: 26 May 2022
 *      Author: kanna
 */

#include "utils.h"
#include <msp430.h>

void CopyArray(uint8_t *source, uint8_t *dest, uint8_t count)
{
    uint8_t copyIndex = 0;
    for(copyIndex = 0; copyIndex < count; copyIndex++)
    {
        dest[copyIndex] = source[copyIndex];
    }
}

void CopyInt16Array(int16_t source, uint8_t *dest)
{
    dest[0] = source >> 8;
    dest[1] = (source & 0xff);
}

void CopyUi32ToArray(uint32_t source, uint8_t *dest)
{
    dest[0] = source >> 24;
    dest[1] = source >> 16;
    dest[2] = source >> 8;
    dest[3] = (source & 0xff);
}

uint32_t ConvertUi8pToUi32(uint8_t *source)
{
    uint32_t resp = 0;
    int g;
    for(g = 0; g < 4; g++)
    {
        resp = resp<<8;
        resp |= source[g];
    }
    return resp;
}

void delay_ms()
{
    __delay_cycles(16000);
}

void delay_s()
{
    int k = 0;
    for (k = 0; k < 1000; k++)
    {
        delay_ms();
    }
}
