#include <avr/io.h>
#include <avr/interrupt.h>
#include "ultrasoon.h"

// Timer1: 16 MHz / prescaler 64 = 250 kHz -> 1 timertick = 4 microseconden
#define TIMER1_TICK_TIJD 4

#define US_PER_CM 58 // echo-tijd in us / 58 = afstand in cm
#define MM_PER_CM 10

#define TRIGGER_PULS_TIJD 12 // us (HC-SR04 wil minimaal 10 us)
#define METING_PERIODE 40000 // tijd tussen twee metingen in us (40 ms)

// Maximale echo-tijd in us (ULTRASOON_MAX_AFSTAND mm -> cm -> us)
#define MAX_ECHO_TIJD ((ULTRASOON_MAX_AFSTAND / MM_PER_CM) * US_PER_CM)

static volatile uint16_t echo_tijd = 0;
static volatile uint8_t  nieuwe_meting = 0;
static volatile uint16_t rising_tick = 0;

void ultrasoon_init(void)
{
  DDRB |= (1 << DDB2);   
  TCCR1A = (1 << COM1B1) | (1 << WGM11) | (1 << WGM10);
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << ICES1) | (1 << CS11) | (1 << CS10);
  OCR1A  = (METING_PERIODE / TIMER1_TICK_TIJD) - 1;
  OCR1B  = TRIGGER_PULS_TIJD / TIMER1_TICK_TIJD;
  TIMSK1 = (1 << ICIE1) | (1 << OCIE1A);   
}

uint8_t ultrasoon_nieuwe_meting(void)
{
  return nieuwe_meting;
}

uint16_t ultrasoon_lees_afstand(void)
{
  uint8_t sreg = SREG;
  cli();                          
  uint16_t tijd = echo_tijd;
  nieuwe_meting = 0;
  SREG = sreg;

  // tijd in us / 58 = cm;  cm * 10 = mm
  return (uint16_t)((uint32_t)tijd * MM_PER_CM / US_PER_CM);
}

ISR(TIMER1_COMPA_vect)
{
  TCCR1B |= (1 << ICES1);
  TIFR1  |= (1 << ICF1);
}

ISR(TIMER1_CAPT_vect)
{
  if (TCCR1B & (1 << ICES1)) {
    rising_tick = ICR1;
    TCCR1B &= ~(1 << ICES1);
  } else {                                 
    uint16_t ticks = ICR1 - rising_tick;   
    uint32_t tijd = (uint32_t)ticks * TIMER1_TICK_TIJD;
    if (tijd > MAX_ECHO_TIJD) tijd = MAX_ECHO_TIJD;
    echo_tijd = (uint16_t)tijd;
    nieuwe_meting = 1;
    TCCR1B |= (1 << ICES1); // terug naar rising
  }
  TIFR1 |= (1 << ICF1); // flag wissen na wisselen van edge
}