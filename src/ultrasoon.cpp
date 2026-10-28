#include <avr/io.h>
#include <avr/interrupt.h>
#include "ultrasoon.h"

// Timer1: 16 MHz / prescaler 64 = 250 kHz -> 1 timertick = 4 microseconden
#define TIMER1_TICK_TIJD 4

#define US_PER_CM 58      // echo-tijd in us / 58 = afstand in cm
#define MM_PER_CM 10

#define TRIGGER_PULS_TIJD 8 
#define METING_PERIODE 40000   // tijd tussen twee metingen in us (40 ms)
#define SENSOR_LATENTIE 240     // tijd tussen start trigger en begin echo-puls in us

// Maximale echo-tijd in us (ULTRASOON_MAX_MM -> cm -> us)
#define MAX_ECHO_TIJD       ((ULTRASOON_MAX_AFSTAND / MM_PER_CM) * US_PER_CM)

static volatile uint16_t echo_tijd  = 0;   
static volatile uint8_t  nieuwe_meting = 0;   

void ultrasoon_init(void)
{
  DDRB |= (1 << DDB2); 

  // Fast PWM, TOP = OCR1A (mode 15), prescaler 64, OC1B 
  TCCR1A = (1 << COM1B1) | (1 << WGM11) | (1 << WGM10);
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10);
  OCR1A  = (METING_PERIODE / TIMER1_TICK_TIJD) - 1;  
  OCR1B  = TRIGGER_PULS_TIJD / TIMER1_TICK_TIJD;         
  TIMSK1 = (1 << ICIE1); // interrupt bij input capture (falling edge van echo)
}

uint8_t ultrasoon_nieuwe_meting(void)
{
  return nieuwe_meting;
}

uint16_t ultrasoon_lees_afstand(void)
{
  uint8_t sreg = SREG;
  cli();                          // echo_tijd is 16 bit: niet laten onderbreken
  uint16_t tijd = echo_tijd;
  nieuwe_meting = 0;              
  SREG = sreg;

  // tijd in us / 58 = cm;  cm * 10 = mm
  return (uint16_t)((uint32_t)tijd * MM_PER_CM / US_PER_CM);
}

ISR(TIMER1_CAPT_vect)
{
  uint16_t tijd = ICR1 * TIMER1_TICK_TIJD;
  tijd = (tijd > SENSOR_LATENTIE) ? tijd - SENSOR_LATENTIE : 0;
  if (tijd > MAX_ECHO_TIJD) tijd = MAX_ECHO_TIJD;
  echo_tijd = tijd;
  nieuwe_meting = 1;
}