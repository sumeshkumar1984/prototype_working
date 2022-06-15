/*
 * utils.h
 *
 *  Created on: 26 May 2022
 *      Author: kanna
 */

#ifndef UTILS_H_
#define UTILS_H_
#include <stdint.h>
extern void CopyInt16Array(int16_t source, uint8_t *dest);
extern void CopyArray(uint8_t *source, uint8_t *dest, uint8_t count);
extern void CopyUi32ToArray(uint32_t source, uint8_t *dest);
extern uint32_t ConvertUi8pToUi32(uint8_t *source);
extern void delay_ms();
extern void delay_s();
#endif /* UTILS_H_ */
