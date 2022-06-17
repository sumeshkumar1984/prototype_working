/*
 * gpio_lib.h
 *
 *  Created on: 27 May 2022
 *      Author: kanna
 */

#ifndef GPIO_LIB_H_
#define GPIO_LIB_H_
#include <stdint.h>

extern void initGPIO();
extern void Toggle_Control_Pin();
extern void Set_i2c_Pins();
extern void initWakeupPin();
extern void clear_wakeup(void);
extern uint8_t get_wakeup(void);
extern void enable_wakeup();
extern void disable_wakeup();

#endif /* GPIO_LIB_H_ */
