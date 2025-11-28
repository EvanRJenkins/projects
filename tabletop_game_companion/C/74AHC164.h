/* 
This file provides functions for the MSP430g series MCUs
to interface with the 74AHC164 SIPO shift register IC.
*/

#ifndef 74AHC164_H
#define 74AHC164_H

/*
Global 74AHC164 input pin definitions.
*/

extern unsigned char * DSA_REG, DSB_REG;  // Serial data
extern unsigned char * MR_N_REG;  // Active-low RST
extern unsigned char * CP_REG;  // Clock pulse

/*
I/O Pin Masks
*/

extern unsigned char  DSA_PIN, DSB_PIN;  // Serial data
extern unsigned char  MR_N_PIN;  // Active-low RST
extern unsigned char  CP_PIN;  // Clock pulse

// Assign the registers of I/O pins being used
void SIPO_reg_init(unsigned char * DSA_reg, unsigned char * DSB_reg,
  unsigned char * MR_N_reg, unsigned char * CP_reg)
{
  DSA_REG = DSA_REG_reg;
  DSB_REG = DSB_REG_reg;
  MR_N_REG = MR_N_reg;
  CP_REG = CP_reg;
}

// Pass in masks for pins being used
void SIPO_pin_init(unsigned char DSA_pin, unsigned char DSB_pin,
  unsigned char MR_N_pin, unsigned char CP_pin)
{
  DSA_PIN = DSA_pin;
  DSB_PIN = DSA_pin;
  MR_N_PIN = MR_N_pin;
  CP_PIN = CP_pin;
}

// Shift one byte over 8 clock cycles
void SIPO_shift(char shift_byte)
{
  unsigned char shift_counter = 0;
  for (shift_counter = 0; shift_counter < 8; shift_counter++)
  {
    switch ((shift_byte >> shift_counter) & 0x01)
    {
      case 0:
        // Set shift bit low
        *DSA_REG &= ~DSA_PIN;
        // Pulse clock
        *CP_REG |= CP_PIN;
        *CP_REG &= ~CP_PIN;
        break;
      case 1:
        // Set shift bit high
        *DSA_REG |= DSA_PIN;
        // Pulse clock
        *CP_REG |= CP_PIN;
        *CP_REG &= ~CP_PIN;       
        break;
    }
  }
}










#endif
