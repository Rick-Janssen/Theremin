#include <avr/io.h>
#include "potmeter.h"

void potmeter_init(void)
{
  ADMUX  = (1 << REFS0);
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint16_t potmeter_lees(void)
{
  ADCSRA |= (1 << ADSC);
  while (ADCSRA & (1 << ADSC));
  return ADC;
}