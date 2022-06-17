/*
 * i2c_lib.h
 *
 *  Created on: 9 Jun. 2022
 *      Author: kanna
 */

#ifndef I2C_LIB_H_
#define I2C_LIB_H_
#include <stdint.h>


typedef enum
{
    NONE = 0,
    CMD_READ_ADC_1,
    CMD_READ_ADC_4,
    CMD_READ_ADC_5,
    CMD_READ_ADC_6,
    CMD_TEMP_SENS,
    CMD_TIMER_STATUS_VAL,
    CMD_TIMER_CNTRL,
    CMD_SLP_GO,
    CMD_TYPE_0_MASTER,
    CMD_TYPE_1_MASTER,
    CMD_TYPE_2_MASTER
}i2c_commands;


extern void set_i2c_resp_int(int value);
extern void set_i2c_resp_ui32(uint32_t value);
extern uint8_t get_i2c_cmd(void);
extern uint8_t get_i2c_read_cmd(void);
extern uint32_t get_timer_value(void);
extern void finished_acting_on_received();
extern uint8_t sleep_instruction(void);
extern void initI2C();
extern void deinit_i2c();

#endif /* I2C_LIB_H_ */
