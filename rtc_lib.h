/*
 * timer_lib.h
 *
 *  Created on: 25 May 2022
 *      Author: sumesh
 */

#ifndef TIMER_LIB_H_
#define TIMER_LIB_H_

#include <stdint.h>
#include <stdbool.h>


extern void StopTimer(void);

extern bool Check_if_timer_up(void);

extern void StartTimer(uint32_t Set_timer_in_sec);

extern bool SetTimer(uint32_t value);

extern uint32_t get_sleep_time_ref(void);

#endif /* TIMER_LIB_H_ */
