#include <Arduino.h>

#define TOON_HZ 1000UL                 // toonhoogte in Hz

void setup() {
  DDRD |= (1 << DDD3);                 // pin 3 als uitgang

  // Timer2: fast PWM, 62,5 kHz (volume)
  TCCR2A = (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
  TCCR2B = (1 << CS20);                // geen prescaler
  OCR2B = 0;                           // pulsbreedte

  // Timer0: CTC, bepaalt de toon
  TCCR0A = (1 << WGM01);
  TCCR0B = (1 << CS02);                // prescaler 256
  OCR0A = (62500UL / (2UL * TOON_HZ)) - 1;   // halve periode in ticks
  TIMSK0 = (1 << OCIE0A);              // interrupt aan

  // ADC: potmeter op A0
  ADMUX = (1 << REFS0);                // referentie AVcc, kanaal 0
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);  // ADC aan, prescaler 128
  DIDR0 = (1 << ADC0D);                // digitale buffer A0 uit
}

// Elke halve periode van de toon
ISR(TIMER0_COMPA_vect) {
  TCCR2A ^= (1 << COM2B1);             // PWM aan/uit wisselen

  if (!(TCCR2A & (1 << COM2B1))) {     // PWM uit?
    PORTD &= ~(1 << PORTD3);           // pin 3 laag
  }
}

void loop() {
  ADCSRA |= (1 << ADSC);               // start conversie
  while (ADCSRA & (1 << ADSC));        // wacht tot klaar
  OCR2B = ADC >> 2;                    // potmeter (0..1023) naar volume (0..255)
}