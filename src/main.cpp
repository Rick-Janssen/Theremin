#include <Arduino.h>
#include "ultrasoon.h"
#include "potmeter.h"

#define TOON_MIN_HZ       230UL      // laagste toon in Hz 
#define TOON_MAX_HZ       1400UL     // hoogste toon in Hz
#define TIMER0_KLOK_HZ    62500UL    // 16 MHz / prescaler 256

ISR(TIMER0_COMPA_vect)
{
  // elke halve periode van de toon: PWM-uitgang aan/uit
  TCCR2A ^= (1 << COM2B1);

  if (!(TCCR2A & (1 << COM2B1))) {
    PORTD &= ~(1 << PORTD3);
  }
}

void setup()
{
  ultrasoon_init();
  potmeter_init();

  DDRD |= (1 << DDD3); // PD3 (OC2B): buzzer

  // Timer2: fast PWM, geen prescaler (62,5 kHz) -> volume
  TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
  TCCR2B = (1 << CS20);

  // Timer0: CTC, prescaler 256 -> toonhoogte
  TCCR0A = (1 << WGM01);
  TCCR0B = (1 << CS02);
  OCR0A  = 134;                      
  TIMSK0 = (1 << OCIE0A);
}

void loop()
{
  OCR2B = potmeter_lees_volume();

  if (ultrasoon_nieuwe_meting()) {
    uint16_t afstand_mm = ultrasoon_lees_afstand();

    if (afstand_mm > ULTRASOON_MAX_AFSTAND) afstand_mm = ULTRASOON_MAX_AFSTAND;
    if (afstand_mm < ULTRASOON_MIN_AFSTAND) afstand_mm = ULTRASOON_MIN_AFSTAND;

    // dichtbij = hoge toon, ver weg = lage toon
    uint16_t hz = TOON_MAX_HZ -
      ((uint32_t)(afstand_mm - ULTRASOON_MIN_AFSTAND) * (TOON_MAX_HZ - TOON_MIN_HZ))
      / (ULTRASOON_MAX_AFSTAND - ULTRASOON_MIN_AFSTAND);

    OCR0A = TIMER0_KLOK_HZ / (2UL * hz) - 1;
  }
}