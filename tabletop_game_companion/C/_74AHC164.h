/* 
This file provides functions for the MSP430g series MCUs
to interface with the 74AHC164 SIPO shift register IC.
*/

#ifndef _74AHC164_H
#define _74AHC164_H

/*
NOTE: These functions are designed for the following two
configurations:
1. DSA and DSB tied together
2. DSB tied to VCC
*/

/*
Global 74AHC164 input pin registers.
*/

extern volatile unsigned char *pDSA_REG;  // Serial data
extern volatile unsigned char *pMR_N_REG;  // Active-low RST
extern volatile unsigned char *pCP_REG;  // Clock pulse

/*
Global input pin masks
*/

extern unsigned char  DSA_PIN;  // Serial data
extern unsigned char  MR_N_PIN;  // Active-low RST
extern unsigned char  CP_PIN;  // Clock pulse

// Assign the registers of I/O pins being used
void SIPO_reg_init(volatile unsigned char *DSA_addr,
  volatile unsigned char *MR_N_addr, volatile unsigned char *CP_addr);

// Pass in masks for pins being used
void SIPO_pin_init(unsigned char DSA_pin,
  unsigned char MR_N_pin, unsigned char CP_pin);

// reset 74AHC164 register (Async master RST)
void SIPO_reset();

// Shift one byte over 8 clock cycles
void SIPO_shift(unsigned char shift_byte);


#endif
