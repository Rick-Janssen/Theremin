#include <Arduino.h>
#include <stdlib.h>
#include "ultrasoon.h"
#include "potmeter.h"

#define BUF_LEN       8
#define TOON_MIN_HZ   230UL
#define TOON_MAX_HZ   1400UL

typedef uint16_t (*omreken_fn)(uint16_t);

static uint16_t naar_cm(uint16_t ticks) { return ((uint32_t)ticks * TICK_US) / US_PER_CM; }
static uint16_t naar_us(uint16_t ticks) { return ticks * TICK_US; }

static uint16_t  *metingen = NULL;
static omreken_fn omreken  = naar_cm;
static uint8_t    positie  = 0;

// Elke halve periode van de toon: PWM aan/uit wisselen
ISR(TIMER0_COMPA_vect)
{
  TCCR2A ^= (1 << COM2B1);
  if (!(TCCR2A & (1 << COM2B1))) PORTD &= ~(1 << PORTD3);
}

void setup()
{
  ultrasoon_init();
  potmeter_init();

  DDRD |= (1 << DDD3);                          // pin 3 = buzzer

  // Timer2: fast PWM 62,5 kHz, OCR2B = volume
  TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
  TCCR2B = (1 << CS20);

  // Timer0: CTC, prescaler 256, bepaalt de toon
  TCCR0A = (1 << WGM01);
  TCCR0B = (1 << CS02);
  OCR0A  = 134;                                 // start op 230 Hz
  TIMSK0 = (1 << OCIE0A);

  metingen = (uint16_t *)calloc(BUF_LEN, sizeof(uint16_t));   // calloc zet alles op 0
  if (metingen == NULL) while (1);              // geen geheugen: stop veilig
}

void loop()
{
  OCR2B = potmeter_lees() >> 2;                 // potmeter = volume (0..255)

  if (ultrasoon_nieuw()) {
    uint16_t ticks = ultrasoon_lees();
    if (ticks > ECHO_MAX_TICKS) ticks = ECHO_MAX_TICKS;

    uint16_t hz = TOON_MAX_HZ - ((uint32_t)ticks * (TOON_MAX_HZ - TOON_MIN_HZ)) / ECHO_MAX_TICKS;
    OCR0A = 31250UL / hz - 1;                   // toonhoogte uit de afstand

    metingen[positie] = omreken(ticks);
    positie = (positie + 1) % BUF_LEN;
  }
}