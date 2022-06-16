/*
 * i2c_lib.c
 *
 *  Created on: 9 Jun. 2022
 *      Author: kanna
 */
#include "i2c_lib.h"
#include <string.h>
#include <stdbool.h>
#include <msp430.h>
#include "utils.h"


#define SLAVE_ADDR  (0x44<<1)
#define TYPE_0_4_LENGTH   2
#define TYPE_1_LENGTH     1
#define TYPE_TIMER_LENGTH   4

char SLV_Addr = SLAVE_ADDR;        // Address is 0x44<<1 for R/W
int  Bytecount, transmit = 0;     // State variables
uint8_t Number_of_Bytes = 0;                  // **** How many bytes?? ****
uint8_t transmit_index = 0;
uint8_t ui8pIncoming_Buffer[8]={0};
uint8_t ui8pOutgoing_Buffer[8]={0};

void Data_RX(void);
void TX_Data(void);
void Reg_RX(void);
uint8_t REG_ADDRESS;
uint8_t i2c_read_cmd = 0;
uint8_t i2c_write_cmd = 0;

void Setup_USI_Slave(void){
  P1OUT |= 0xC0;                             // P1.6 & P1.7 Pullups
  P1REN |= 0xC0;                            // P1.6 & P1.7 Pullups
  P1DIR |= 0xC0;                             // Unused pins as outputs
  P2OUT = 0;
  P2DIR = 0xFF;
  USICTL0 = USIPE6+USIPE7+USISWRST;         // Port & USI mode setup
  USICTL1 = USII2C+USIIE+USISTTIE;          // Enable I2C mode & USI interrupts
  USICKCTL = USICKPL;                       // Setup clock polarity
  USICNT |= USIIFGCC;                       // Disable automatic clear control
  USICTL0 &= ~USISWRST;                     // Enable USI
  USICTL1 &= ~USIIFG;                       // Clear pending flag
  transmit = 0;
  __enable_interrupt();
}

typedef enum I2C_ModeEnum{

    IDLE = 0,
    ADDR_RECIEVED = 2,
    CHECK_ADDRESS = 4,
    SET_REG_CHK = 5,
    CHECK_REG_ADDRESS = 6,
    SET_RX_MODE = 7,
    CHK_RX_DATA_AND_DONE = 8,
    DONE_WRITING = 9,
    SET_TX_DATA = 10,
    DUMMY = 11,
    CHECK_TX_ACK = 12,
    SPECIAL_MODE = 13,
    CHK_TX_DONE = 14,
} I2C_Mode;

I2C_Mode I2C_State = IDLE;


uint8_t get_i2c_cmd(void)
{
  return i2c_read_cmd;
}

uint8_t get_i2c_read_cmd(void)
{
  return i2c_write_cmd;
}


void clear_and_start_i2c_engine()
{


}

uint32_t get_timer_value(void)
{
    uint32_t value = ConvertUi8pToUi32(ui8pIncoming_Buffer);
    memset(ui8pIncoming_Buffer, 0x00, 8);
    clear_and_start_i2c_engine();
    return value;
}

uint8_t sleep_instruction(void)
{
    uint8_t value = ui8pIncoming_Buffer[0];
    memset(ui8pIncoming_Buffer, 0x00, 8);
    clear_and_start_i2c_engine();
    return ;
}

bool I2C_Slave_ProcessCMD(i2c_commands cmd)
{
//    ReceiveIndex = 0;
//    TransmitIndex = 0;
//    RXByteCtr = 0;
//    TXByteCtr = 0;
    bool resp =  true;
    switch (cmd)
    {
        case (CMD_READ_ADC_1):                      //Send device adc 1 reading
            i2c_read_cmd = CMD_READ_ADC_1;
            break;

        case (CMD_READ_ADC_4):                      //Send device adc 4 reading
            i2c_read_cmd = CMD_READ_ADC_4;
            break;

        case (CMD_READ_ADC_5):                      //Send device adc 5 reading
            i2c_read_cmd = CMD_READ_ADC_5;
            break;

        case (CMD_READ_ADC_6):                      //Send device adc 6 reading
            i2c_read_cmd = CMD_READ_ADC_6;
            break;

        case (CMD_TEMP_SENS):                      //Send device temprature reading
            //Set flag for transfer
            i2c_read_cmd = CMD_TEMP_SENS;
            break;

        case (CMD_TIMER_STATUS_VAL):
            i2c_read_cmd = CMD_TIMER_STATUS_VAL;
            break;

        case (CMD_TIMER_CNTRL):
            i2c_write_cmd = CMD_TIMER_CNTRL;
            Number_of_Bytes = TYPE_TIMER_LENGTH;
            break;

        case (CMD_SLP_GO):
            i2c_write_cmd = CMD_SLP_GO;
            Number_of_Bytes = TYPE_1_LENGTH;
            break;

        case (CMD_TYPE_0_MASTER):
            i2c_write_cmd = CMD_TYPE_0_MASTER;
            Number_of_Bytes = TYPE_0_4_LENGTH;
            break;

        case (CMD_TYPE_1_MASTER):
              i2c_write_cmd = CMD_TYPE_1_MASTER;
              Number_of_Bytes = TYPE_0_4_LENGTH;
            break;

        case (CMD_TYPE_2_MASTER):
              i2c_write_cmd = CMD_TYPE_2_MASTER;
              Number_of_Bytes = TYPE_0_4_LENGTH;
            break;

        default:
            resp = false;
            __no_operation();
            break;
    }
    return resp;
}

uint8_t templeft;
uint8_t tempright;

bool resp_var;
i2c_commands command;

//******************************************************************************
// USI interrupt service routine
// Rx bytes from master: State ADDR_RECIEVED->CHECK_ADDRESS->SET_RX_MODE->CHK_RX_DATA_AND_DONE
// Tx bytes to Master: State ADDR_RECIEVED->CHECK_ADDRESS->SET_TX_DATA->CHECK_TX_ACK->CHK_TX_DONE
//******************************************************************************
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = USI_VECTOR
__interrupt void USI_TXRX (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USI_VECTOR))) USI_TXRX (void)
#else
#error Compiler not supported!
#endif
{
  if (USICTL1 & USISTTIFG)                  // Start entry?
  {
    //P1OUT |= 0x01;                          // LED on: sequence start
    I2C_State = ADDR_RECIEVED;                          // Enter 1st state on start
    REG_ADDRESS = 0;
    memset(ui8pIncoming_Buffer,0,8);
  }

  switch(__even_in_range((int)I2C_State, 14))
    {
      case IDLE:                               // Idle, should not get here
              break;

      case ADDR_RECIEVED: // RX Address
              USICTL0 &= ~USIOE;            // SDA = input
              USICNT  =  (0xA0 | 0x8); // Bit counter = 8, RX address
              I2C_State = CHECK_ADDRESS;    // Go to next state: check address
              USICTL1 &= ~USISTTIFG;        // Clear start flag
              break;

      case CHECK_ADDRESS: // Process Address and send (N)Ack
             if (USISRL & 0x01)
             {            // If master read...
                 SLV_Addr = SLAVE_ADDR | 0x01;             // Save R/W bit
                 transmit = 1;
                 transmit_index = 0;
             }
             else
             {
                 transmit = 0;
                 SLV_Addr = SLAVE_ADDR;
             }
             USICTL0 |= USIOE;             // SDA = output
             if (USISRL == SLV_Addr)       // Address match?
             {
                 USISRL = 0x00;              // Send Ack
                 if (transmit == 0)
                 {
                     I2C_State = SET_REG_CHK;
                     USICNT |= 0x01;
                     //Reg_RX();
                 }           // Go to next state: RX data
                 if (transmit == 1)
                 {
                     I2C_State = SET_TX_DATA;
                     USICNT |= 0x01;               // Bit counter = 1, send (N)Ack bit
                 }     // Else go to next state: TX data
             }
             else
             {
                 USISRL = 0xFF;              // Send NAck
                 I2C_State = DONE_WRITING;    // next state: prep for next Start
                 USICNT |= 0x01;               // Bit counter = 1, send (N)Ack bit
             }

             break;

    case SET_REG_CHK:
        Reg_RX();
        break;


      case CHECK_REG_ADDRESS: // Process Address and send (N)Ack
        command = (i2c_commands) USISRL;
        resp_var = I2C_Slave_ProcessCMD(command);
        if (resp_var) // If it is a valid register
        {
            if (i2c_write_cmd >= CMD_TIMER_CNTRL)
            {
                USICTL0 |= USIOE;             // SDA = output
                USISRL = 0x00;              // Send Ack
                I2C_State = SET_RX_MODE;    // Rcv another byte
                USICNT |= 0x01;             // Bit counter = 1, send (N)Ack bit
            }
            else
            {
                USICTL0 |= USIOE;             // SDA = output
                USISRL = 0x00;              // Send Ack
                USICNT |= 0x01;             // Bit counter = 1, send (N)Ack bit
                I2C_State = DUMMY;
            }
        }
        else                          // Last Byte
        {
            USISRL = 0xFF;              // Send NAck
            USICTL0 &= ~USIOE;            // SDA = input
            SLV_Addr = SLAVE_ADDR;        // Reset slave address
            I2C_State = IDLE;             // Reset state machine
            Bytecount = 0;                 // Reset counter for next TX/RX
            //USICNT |= 0x01;
        }
        break;

    case SET_RX_MODE: // Receive data byte
              Data_RX();
              break;

    case CHK_RX_DATA_AND_DONE:// Check RX Data & TX (N)Ack
              USICTL0 |= USIOE;             // SDA = output
              ui8pIncoming_Buffer[Bytecount] = USISRL;
              if (Bytecount <= (Number_of_Bytes-2)) // If not last byte
              {
                USISRL = 0x00;              // Send Ack
                I2C_State = SET_RX_MODE;              // Rcv another byte
                Bytecount++;
                USICNT |= 0x01;             // Bit counter = 1, send (N)Ack bit
              }
              else                          // Last Byte
              {
                  USISRL = 0x00;              // Send ACK
                  I2C_State = DONE_WRITING;
                  USICNT |= 0x01;
//                  USICTL0 &= ~USIOE;            // SDA = input
//                  SLV_Addr = SLAVE_ADDR;        // Reset slave address
//                  I2C_State = IDLE;             // Reset state machine
              }
              break;

     case   DONE_WRITING:
            USISRL = 0xFF;              // Send NAck
            USICTL0 &= ~USIOE;            // SDA = input
            SLV_Addr = SLAVE_ADDR;        // Reset slave address
            I2C_State = IDLE;             // Reset state machine
            Bytecount = 0;                 // Reset counter for next TX/RX
            Number_of_Bytes = 0;
            transmit_index = 0;
            LPM0_EXIT;                  // Exit active for next transfer
            break;

     case SET_TX_DATA: // Send Data byte
              TX_Data();
              break;

     case DUMMY:
             I2C_State = SPECIAL_MODE;              // Rcv another byte
             USICNT &= ~(USISCLREL); // stop any further transactions.
             break;

      case CHECK_TX_ACK:// Receive Data (N)Ack
              USICTL0 &= ~USIOE;            // SDA = input
              USICNT |= 0x01;               // Bit counter = 1, receive (N)Ack
              I2C_State = CHK_TX_DONE;               // Go to next state: check (N)Ack
              break;

      case SPECIAL_MODE:
                USISRL = 0x00;                // Send Ack
                USICTL0 &= ~USIOE;            // SDA = input
                SLV_Addr = SLAVE_ADDR;        // Reset slave address
                I2C_State = IDLE;             // Reset state machine
                Bytecount = 0;                 // Reset counter for next TX/RX
                //Number_of_Bytes = 0;
                transmit_index = 0;
                break;

      case CHK_TX_DONE:// Process Data Ack/NAck
           if (USISRL & 0x01)               // If Nack received...
              {
                USICTL0 &= ~USIOE;            // SDA = input
                SLV_Addr = SLAVE_ADDR;        // Reset slave address
                I2C_State = IDLE;                // Reset state machine
                Bytecount = 0;
                transmit_index = 0;
                Number_of_Bytes = 0;
             // LPM0_EXIT;                  // Exit active for next transfer
              }
              else                          // Ack received
              {
                TX_Data();                  // TX next byte
              }
           break;
      }
  if(I2C_State != SPECIAL_MODE) // this is for clock stretching
  {
      USICTL1 &= ~USIIFG;           // Clear pending flags
  }
  else
  {
      USICTL1 &=  ~(USIIE);          // disable I2C USI interrupts
      LPM0_EXIT;                  // Exit to perform measurement
  }
}

void Reg_RX(void)
{

    USICTL0 &= ~USIOE;            // SDA = input
    //USICNT |= 0x08;              // Bit counter = 8, RX data
    //USICNT &= ~(USISCLREL);
    USICNT |= 0x08;              // Bit counter = 8, RX data
    I2C_State = CHECK_REG_ADDRESS;           // next state: Test data and (N)Ack
}

void Data_RX(void)
{
    USICTL0 &= ~USIOE;            // SDA = input
    USICNT |= 0x08;              // Bit counter = 8, RX data
    I2C_State = CHK_RX_DATA_AND_DONE;        // next state: Test data and (N)Ack
}

void TX_Data(void)
{
    USICTL0 |= USIOE;             // SDA = output
    if (transmit_index >= 8)
    {
        transmit_index = 0;
    }
    USISRL = ui8pOutgoing_Buffer[transmit_index];
    transmit_index++;
    USICNT |= 0x08;              // Bit counter = 8, TX data
    I2C_State = CHECK_TX_ACK;               // Go to next state: receive (N)Ack
}


void ReadyToTransmitData(uint8_t* b_array, uint8_t len)
{
    len = (len>8?8:len);
    memset(ui8pOutgoing_Buffer,0,8);
    memcpy(ui8pOutgoing_Buffer, b_array, len);
    //USICTL0 &= ~USIOE;            // SDA = input
    SLV_Addr = SLAVE_ADDR;        // Reset slave address
    //I2C_State = IDLE;             // Reset state machine
    Bytecount = 0;                 // Reset counter for next TX/RX
    USICTL1 |= USIIE;          // Enable I2C mode & USI interrupts
   // USICTL0 |= USIOE;              // SDA = output
   // USISRL = 0x00;                // Send Ack
    USICTL0 &= ~USIOE;            // SDA = input
    //SLV_Addr = SLAVE_ADDR;        // Reset slave address
    I2C_State = IDLE;             // Reset state machine
   // Bytecount = 0;                 // Reset counter for next TX/RX
    transmit_index = 0;
    //USISRL = 0x00;
    USICNT |= (0xA0);       // send Ack
    USICTL1 &= ~USIIFG;            // Clear pending flags

//    USISRL = 0x00;
    //I2C_State = IDLE;
 //  TX_Data();
 //   USICTL0 &= ~USIOE;            // SDA = input
//    USICTL0 |= USIOE;             // SDA = output
////    USICNT = (USICNT & 0xA0);
}

void set_i2c_resp_int(int value)
{
    //Fill out the TransmitBuffer
    uint8_t buffer[TYPE_0_4_LENGTH];
    CopyInt16Array(value, buffer);
    ReadyToTransmitData(buffer, TYPE_0_4_LENGTH);
    i2c_read_cmd = 0;
}

void set_i2c_resp_ui32(uint32_t value)
{
    //Fill out the TransmitBuffer
    uint8_t buffer[TYPE_TIMER_LENGTH];
    CopyUi32ToArray(value, buffer);
    ReadyToTransmitData(buffer, TYPE_TIMER_LENGTH);
    i2c_read_cmd = 0;
}

void finished_acting_on_received()
{
    i2c_write_cmd = 0;
}
