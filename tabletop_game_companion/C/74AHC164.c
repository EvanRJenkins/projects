#include "74AHC164.h"


/*
Global 74AHC164 input pin registers.
*/

unsigned char *pDSA_REG, *pDSB_REG;  // Serial data
unsigned char *pMR_N_REG;  // Active-low RST
unsigned char *pCP_REG;  // Clock pulse

/*
Global input pin masks
*/

unsigned char  DSA_PIN, DSB_PIN;  // Serial data
unsigned char  MR_N_PIN;  // Active-low RST
unsigned char  CP_PIN;  // Clock pulse

// Assign the registers of I/O pins being used
void SIPO_reg_init(unsigned char *DSA_addr, unsigned char *DSB_addr,
  unsigned char *MR_N_addr, unsigned char *CP_addr)
{
  // Copy register addresses to extern ptrs
  pDSA_REG = DSA_addr;
  pDSB_REG = DSB_addr;
  pMR_N_REG = MR_N_addr;
  pCP_REG = CP_addr;
}

// Pass in masks for pins being used
void SIPO_pin_init(unsigned char DSA_pin, unsigned char DSB_pin,
  unsigned char MR_N_pin, unsigned char CP_pin)
{
  // Copy register addresses to extern ptrs
  DSA_PIN = DSA_pin;
  DSB_PIN = DSB_pin;
  MR_N_PIN = MR_N_pin;
  CP_PIN = CP_pin;

  // Set active low reset high initially
  *pMR_N_REG |= MR_N_PIN;
}

// reset 74AHC164 register (Async master RST)
void SIPO_reset()
{
  *pMR_N_REG &= ~MR_N_PIN;
  *pMR_N_REG |= MR_N_PIN;
}


// Shift one byte over 8 clock cycles
void SIPO_shift(char shift_byte)
{
  unsigned char shift_counter = 0;
  for (shift_counter = 0; shift_counter < 8; shift_counter++)
  {
    if ((shift_byte << shift_counter) & 0x01)
    {
      // Set shift bit high
      *pDSA_REG |= DSA_PIN;
      // Pulse clock
      *pCP_REG |= CP_PIN;
      *pCP_REG &= ~CP_PIN;       
    }
    else
    {
      // Set shift bit low
      *pDSA_REG &= ~DSA_PIN;
      // Pulse clock
      *pCP_REG |= CP_PIN;
      *pCP_REG &= ~CP_PIN;
    }
  }
}









