/*
 * adc_lib.h
 *
 *  Created on: 26 May 2022
 *      Author: kanna
 */

#ifndef ADC_LIB_H_
#define ADC_LIB_H_


extern unsigned int measure_temp();

extern unsigned int measure_adc1();

extern unsigned int measure_adc2();

extern unsigned int measure_adc3();

extern unsigned int measure_adc4();

extern void configure_adc(void);

extern void deinit_adc(void);

#endif /* ADC_LIB_H_ */
