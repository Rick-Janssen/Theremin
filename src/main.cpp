#include <Arduino.h>

#define TOON_HZ 1000UL

void setup() {
  DDRD |= (1 << DDD3);

  TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
  TCCR2B = (1 << CS20);
  OCR2B = 0;

  TCCR0A = (1 << WGM01);
  TCCR0B = (1 << CS02);
  OCR0A = (62500UL / (2UL * TOON_HZ)) - 1;
  TIMSK0 = (1 << OCIE0A);

  ADMUX = (1 << REFS0);
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
  DIDR0 = (1 << ADC0D);
}

ISR(TIMER0_COMPA_vect) {
  TCCR2A ^= (1 << COM2B1);

  if (!(TCCR2A & (1 << COM2B1))) {
    PORTD &= ~(1 << PORTD3);
  }
}

void loop() {
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
  OCR2B = ADC >> 2;
}