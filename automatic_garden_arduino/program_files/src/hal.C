#include "hal.h"
#include <avr/interrupt.h>

void regConfig() 
{
  DDRB &= ~(1 << DDB0);                         // Set trigger pin direction as input (D8)
  DDRD &= ~(1 << FLOW_SENSOR_PIN);              // Set flow sensor pin data direction to input (D2)
  PORTB |= (1 << PORTB0);                       // Enable pull-up on pin D8
  PORTD |= (1 << PORTD2);                       // Enable pull-up on pin D2
  DDRD |= (1 << PUMP_PIN);                      // Set pump pin data direction to output

  /* Sleep mode */
  SMCR |= (1 << SM1);                           // Set Sleep Mode Control Register to Power Down Mode
  
  /* Enable interrupts */
  ADCSRA |= (1 << ADIE);                        // Enable 'ADC complete' interrupt  
  TIMSK0 |= (1 << TOIE0);                       // Enable Timer/Counter0 overflow interrupt
  EIMSK |= (1 << INT0);                         // Enable the INT0 external interrupt
  TIMSK1 |= (1 << OCIE1A);                      // Enable the timer1 compare interrupt

  /* Initialize Timer/Counter0 (watering) */
  TCCR0B &= ~((1 << CS02) | (1 << CS01) | (1 << CS00)); // Select no clock for Timer/Counter0
  TCNT0 = 0;                                            // Clear Timer/Counter0 data register

  /* Configure External Interrupt INT0 for Flow Sensor */
  EICRA |= (1 << ISC01);                        // Set INT0 to trigger on a falling edge
  
  /* Initialize Timer/Counter1 (ms) */
  TCCR1A = 0;                                   // Clear Timer/Counter1 Control Registers
  TCCR1B = 0;                                   // Clear Timer/Counter1 Control Registers
  TCCR1B |= (1 << WGM12);                       // Set CTC mode for Timer/Counter1
  TCCR1B |= (1 << CS11) | (1 << CS10);          // Set prescaler to 64
  OCR1A = 249;                                  // Set the compare match register for 1ms tick
}

state_t adcRecord(uint8_t targetPin)  // Enables ADC and starts conversion for targetPin
{
/* Set MUX2..0 to ADC channel of targetPin */
  switch (targetPin) 
  {
    case AI0:  // 000
      ADMUX &= ~((1 << MUX2) | (1 << MUX1) | (1 << MUX0));
      break;
  
    case AI1:  // 001
      ADMUX &= ~((1 << MUX2) | (1 << MUX1));
      ADMUX |= (1 << MUX0);
      break;
  
    case AI2:  // 010
      ADMUX &= ~((1 << MUX2) | (1 << MUX0));
      ADMUX |= (1 << MUX1);
      break;
  
    case AI3:  // 011
      ADMUX &= ~(1 << MUX2);
      ADMUX |= (1 << MUX1) | (1 << MUX0);
      break;
  
    case AI4:  // 100
      ADMUX &= ~((1 << MUX1) | (1 << MUX0));
      ADMUX |= (1 << MUX2);
      break;
  
    case AI5:  // 101
      ADMUX &= ~(1 << MUX1);
      ADMUX |= (1 << MUX2) | (1 << MUX0);
      break;
    
    default: // Default to channel 0
      ADMUX &= ~((1 << MUX2) | (1 << MUX1) | (1 << MUX0));
      break;
  }
  ADCSRA |= (1 << ADEN);  // Enable ADC
  ADCSRA |= (1 << ADSC);  // Start ADC conversion
  return WAIT_FOR_ADC;
}

state_t startPump() 
{
  PORTD |= (1 << PUMP_PIN);   // Activate tank pump  
  return WAIT_FOR_FLOW;
}

state_t stopPump() 
{
  PORTD &= ~(1 << PUMP_PIN);  // Deactivate tank pump   
  return LOG_POST;
}
