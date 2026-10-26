#include <Arduino.h>
#include "ultrasoon.h"
#include "potmeter.h"

#define TOON_MIN_HZ 230UL
#define TOON_MAX_HZ 1400UL

ISR(TIMER0_COMPA_vect)
{
  TCCR2A ^= (1 << COM2B1);

  if (!(TCCR2A & (1 << COM2B1))) {
    PORTD &= ~(1 << PORTD3);
  }
}

void setup()
{
  ultrasoon_init();
  potmeter_init();

  DDRD |= (1 << DDD3);

  // Timer2: PWM voor het volume
  TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
  TCCR2B = (1 << CS20);

  // Timer0: CTC voor de toonhoogte
  TCCR0A = (1 << WGM01);
  TCCR0B = (1 << CS02);
  OCR0A = 134;
  TIMSK0 = (1 << OCIE0A);
}

void loop()
{
  OCR2B = potmeter_lees() >> 2;

  if (ultrasoon_nieuw()) {
    uint16_t ticks = ultrasoon_lees();

    if (ticks > ECHO_MAX_TICKS) {
      ticks = ECHO_MAX_TICKS;
    }

    uint16_t hz = TOON_MAX_HZ -
      ((uint32_t)ticks * (TOON_MAX_HZ - TOON_MIN_HZ))
      / ECHO_MAX_TICKS;

    OCR0A = 31250UL / hz - 1;
  }
}