#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>

#define TICKS_MAX     870UL             // 65 cm = 3770 us = 942 ticks van 4 us
#define OFFSET_TICKS  80                // trigger -> begin echo (meten met analyzer, ca. 300 us)

volatile uint16_t afstand_ticks = 0;    // echobreedte in ticks van 4 us

int main(void)
{
  DDRB |= (1 << DDB2);                  // pin 10 = trigger (OC1B) echo = pin 8 (ICP1)

  // Timer1: fast PWM (TOP = OCR1A), prescaler 64 -> 1 tick = 4 us
  TCCR1A = (1 << COM1B1) | (1 << WGM11) | (1 << WGM10);
  TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11) | (1 << CS10);   // ICES1 = 0: dalende flank
  OCR1A  = 14999;                        // elke 60 ms een trigger
  OCR1B  = 2;                           // trigger 3 ticks hoog = 12 us
  TIMSK1 = (1 << ICIE1);                // interrupt bij flank op echo

  sei();
  while (1);
}

// Einde van de echo: ICR1 = tijd sinds start trigger
ISR(TIMER1_CAPT_vect)
{
  uint16_t t = ICR1;
  t = (t > OFFSET_TICKS) ? t - OFFSET_TICKS : 0;   // echobreedte
  if (t > TICKS_MAX) t = TICKS_MAX;                // buiten bereik begrenzen
  afstand_ticks = t;
}