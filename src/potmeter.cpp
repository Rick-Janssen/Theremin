#include <avr/io.h>
#include "potmeter.h"

void potmeter_init(void)
{

  ADMUX  = (1 << REFS0) | (1 << ADLAR);

  // ADEN: ADC aan; prescaler 128 (16 MHz / 128 = 125 kHz ADC-klok)
  ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

uint8_t potmeter_lees_volume(void)
{
  ADCSRA |= (1 << ADSC);              
  while (ADCSRA & (1 << ADSC)) { }   
  return ADCH;                        
}