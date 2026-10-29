#include <avr/io.h>
#include "display.h"

// twi.h/twi.c is een C-library: voor C++ moeten de functies als C worden gekoppeld

#include "twi.h"


// PCF8574A-pinnen: P0 = segment A, P1 = B, P2 = C, P3 = D, P4 = E, P5 = F, P6 = G, P7 = DP
// Het display heeft een gemeenschappelijke anode (CA aan +5V): een segment licht op als de pin LAAG is.
#define SEGMENTEN_UIT  0xFF

// Alleen de oneven filtergroottes 1, 3, 5, 7, 9, 11, 13, 15 hoeven getoond te worden (hexadecimaal 1 3 5 7 9 b d F).
// Per oneven waarde: welke segmenten aan moeten (1 = aan), bit 0 = A ... bit 6 = G, bit 7 = DP (altijd uit)
static const uint8_t segment_patroon[8] = {
  0x06,   //  1 = 1: B C
  0x4F,   //  3 = 3: A B C D G
  0x6D,   //  5 = 5: A C D F G
  0x07,   //  7 = 7: A B C
  0x6F,   //  9 = 9: A B C D F G
  0x7C,   // 11 = b: C D E F G
  0x5E,   // 13 = d: B C D E G
  0x71    // 15 = F: A E F G
};

static uint8_t display_adres = 0;
static uint8_t laatst_getoond = 0xFF;   // laatst getoonde filtergrootte (0xFF = nog niets getoond)

// Stuurt 1 byte naar de PCF8574A met de TWI-library.
// Geeft 1 terug als de PCF8574A de data heeft bevestigd (ACK), anders 0.
static uint8_t display_schrijf(uint8_t adres, uint8_t data)
{
  TWI_MT_Start();                      // START
  TWI_Transmit_SLAW(adres);             // adres + schrijven
  TWI_Transmit_Byte(data);             // data
  uint8_t gelukt = (TWI_STATUS == TWI_MT_DATA_ACK);   // status van de laatste byte, vóór de STOP
  TWI_Stop();                          // STOP                           // bus laten afronden voor de volgende START
  return gelukt;
}

void display_init(void)
{
  // PC4 (SDA) en PC5 (SCL): ingang met interne pull-up
  DDRC  &= ~((1 << DDC4) | (1 << DDC5));
  PORTC |=  (1 << PORTC4) | (1 << PORTC5);

  TWI_Init();

  // Zoek het aangesloten PCF8574(A)-adres, zoals in de displaytest.
  for (uint8_t adres = 0x38; adres <= 0x3F && display_adres == 0; adres++) {
    if (display_schrijf(adres, SEGMENTEN_UIT)) {
      display_adres = adres;
    }
  }
  for (uint8_t adres = 0x20; adres <= 0x27 && display_adres == 0; adres++) {
    if (display_schrijf(adres, SEGMENTEN_UIT)) {
      display_adres = adres;
    }
  }

  laatst_getoond = 0xFF;   // de eerste display_update() stuurt altijd
}

void display_update(uint8_t grootte)
{
  if (display_adres == 0 || grootte == laatst_getoond) return;   // geen display of niets veranderd

  uint8_t data = SEGMENTEN_UIT;            // standaard: display uit (bijvoorbeeld grootte 0)
  if (grootte >= 1 && grootte <= 15 && (grootte % 2) == 1) {
    // Omdraaien: aan-patroon (1 = aan) wordt pin-niveau (0 = laag = segment aan)
    data = (uint8_t)~segment_patroon[grootte / 2];
  }

  // Alleen onthouden als het versturen gelukt is; anders probeert de volgende loop() het opnieuw
  if (display_schrijf(display_adres, data)) {
    laatst_getoond = grootte;
  }
}