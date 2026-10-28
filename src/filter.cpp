#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdlib.h>
#include "filter.h"

// Eén element: leeftijd (volgorde van binnenkomst) + waarde (voor sorteren op grootte)
typedef struct {
  uint8_t  age;
  uint16_t value;
} toonhoogte;

static toonhoogte    *buffer      = NULL;        
static uint8_t           grootte     = 0; // actieve grootte (0 = nog niet gealloceerd)
static volatile uint8_t  gewenst     = FILTER_MAX; // gewenste grootte, door ISR aangepast
static uint8_t           gevuld      = 0; // 0 = buffer nog niet gevuld met eerste waarde
static volatile uint8_t  vorige_pind = 0;

#define KNOP_MIN  (1 << PIND4)   // knop: filter kleiner
#define KNOP_PLUS (1 << PIND5)   // knop: filter groter

// Compare-functie voor qsort(): sorteer op value (oplopend)
static int vergelijk_value(const void *a, const void *b)
{
  uint16_t x = ((const toonhoogte *)a)->value;
  uint16_t y = ((const toonhoogte *)b)->value;
  return (x > y) - (x < y);
}

// Vul het buffer met één waarde; elke age is uniek (0..grootte-1)
static void buffer_vullen(uint16_t waarde)
{
  toonhoogte *p = buffer;
  for (uint8_t i = 0; i < grootte; i++, p++) {
    p->age   = i;
    p->value = waarde;
  }
}

// Buffer vrijgeven en opnieuw alloceren met de nieuwe grootte
static uint8_t buffer_alloceren(uint8_t nieuwe_grootte)
{
  free(buffer);
  buffer = (toonhoogte *)malloc(nieuwe_grootte * sizeof(toonhoogte));

  if (buffer == NULL) {
    grootte = 0;
    return 0;
  }

  grootte = nieuwe_grootte;
  return 1;
}

void filter_init(void)
{
  // PD4 en PD5: input met interne pull-up
  DDRD  &= ~(KNOP_MIN | KNOP_PLUS);
  PORTD |=  (KNOP_MIN | KNOP_PLUS);

  vorige_pind = PIND & (KNOP_MIN | KNOP_PLUS);

  // Pin Change Interrupt 2 (PCINT16..23) aan, alleen PCINT20 (PD4) en PCINT21 (PD5)
  PCICR  |= (1 << PCIE2);
  PCMSK2 |= (1 << PCINT20) | (1 << PCINT21);

  // grootte blijft 0: het buffer wordt bij de eerste filter_verwerk() gealloceerd
}

uint8_t filter_grootte(void)
{
  return grootte;
}

uint16_t filter_verwerk(uint16_t waarde)
{
  // Nieuwe grootte overnemen (ISR past alleen 'gewenst' aan, zodat het buffer veilig blijft)
  uint8_t nieuw = gewenst;
  if (nieuw != grootte) {
    if (!buffer_alloceren(nieuw)) {
      return waarde;                // geen geheugen: ongefilterd doorgeven, volgende keer opnieuw proberen
    }
    gevuld = 0;                     // buffer opnieuw opbouwen
  }

  // Eerste keer (of na resize): hele buffer vullen met de nieuwe waarde
  if (!gevuld) {
    buffer_vullen(waarde);
    gevuld = 1;
    return waarde;
  }

  // Alle leeftijden ophogen en de oudste zoeken
  toonhoogte *oudste = buffer;
  toonhoogte *einde  = buffer + grootte;
  for (toonhoogte *p = buffer; p < einde; p++) {
    p->age++;
    if (p->age > oudste->age) {
      oudste = p;
    }
  }

  // Oudste overschrijven met de nieuwe waarde
  oudste->age   = 0;
  oudste->value = waarde;

  // Sorteren op grootte; leeftijd zit in de struct, dus de volgorde gaat niet verloren
  qsort(buffer, grootte, sizeof(toonhoogte), vergelijk_value);

  // Midden: (grootte / 2) + 1 (1-based) = index grootte / 2 (0-based)
  return (buffer + grootte / 2)->value;
}

// Knoppen: reageer op neergaande flank (indrukken, pull-up)
ISR(PCINT2_vect)
{
  uint8_t nu = PIND & (KNOP_MIN | KNOP_PLUS);
  uint8_t gedrukt = vorige_pind & ~nu;      // was hoog, nu laag
  vorige_pind = nu;

  if ((gedrukt & KNOP_MIN) && gewenst > FILTER_MIN) {
    gewenst-= 2;
  } 
  if ((gedrukt & KNOP_PLUS) && gewenst < FILTER_MAX) {
    gewenst+= 2;
  }
}