/* 
This file provides functions for the MSP430g series MCUs
to interface with the 74AHC164 SIPO shift register IC.
*/

#ifndef 74AHC164_H
#define 74AHC164_H

/*
NOTE: These functions are designed for the following two
configurations:
1. DSA and DSB tied together
2. DSB tied to VCC
*/

/*
Global 74AHC164 input pin registers.
*/

extern unsigned char *pDSA_REG, *pDSB_REG;  // Serial data
extern unsigned char *pMR_N_REG;  // Active-low RST
extern unsigned char *pCP_REG;  // Clock pulse

/*
Global input pin masks
*/

extern unsigned char  DSA_PIN, DSB_PIN;  // Serial data
extern unsigned char  MR_N_PIN;  // Active-low RST
extern unsigned char  CP_PIN;  // Clock pulse

// Assign the registers of I/O pins being used
void SIPO_reg_init(unsigned char *DSA_addr, unsigned char *DSB_addr,
  unsigned char *MR_N_addr, unsigned char *CP_addr);

// Pass in masks for pins being used
void SIPO_pin_init(unsigned char DSA_pin, unsigned char DSB_pin,
  unsigned char MR_N_pin, unsigned char CP_pin);

// reset 74AHC164 register (Async master RST)
void SIPO_reset();

// Shift one byte over 8 clock cycles
void SIPO_shift(char shift_byte);


#endif
