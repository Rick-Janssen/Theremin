#include <avr/io.h>
#include <avr/interrupt.h>
#include "ultrasoon.h"

#define ECHO_MAX     870UL         // 60 cm = 3480 us = 870 ticks van 4 us
#define ECHO_VERTRAGING  80            

static volatile uint16_t afstand_ticks = 0;
static volatile uint8_t  nieuwe_meting = 0;

void ultrasoon_init(void)
{
  DDRB |= (1 << DDB2);              // pin 10 = trigger , echo = pin 8 

  TCCR1A = (1 << COM1B1) | (1 << WGM11) | (1 << WGM10);
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10);
  OCR1A  = 14999;                   // elke 60 ms een trigger
  OCR1B  = 2;                       // trigger 12 us hoog
  TIMSK1 = (1 << ICIE1);
}

uint8_t ultrasoon_nieuw(void)
{
  return nieuwe_meting;
}

uint16_t ultrasoon_lees(void)
{
  uint8_t sreg = SREG;
  cli();                            
  uint16_t t = afstand_ticks;
  nieuwe_meting = 0;
  SREG = sreg;
  return t;
}

ISR(TIMER1_CAPT_vect)
{
  uint16_t t = ICR1;
  t = (t > ECHO_VERTRAGING) ? t - ECHO_VERTRAGING : 0;
  if (t > ECHO_MAX) t = ECHO_MAX;
  afstand_ticks = t;
  nieuwe_meting = 1;
}