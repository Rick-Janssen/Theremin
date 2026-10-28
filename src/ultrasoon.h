#ifndef ULTRASOON_H
#define ULTRASOON_H

#include <stdint.h>

#define ULTRASOON_MIN_AFSTAND  20    // in mm = 2 cm
#define ULTRASOON_MAX_AFSTAND  650   // in mm = 65 cm (NFE.2)

void     ultrasoon_init(void);       // state machine, elke loop() aanroepen
uint8_t  ultrasoon_nieuwe_meting(void);
uint16_t ultrasoon_lees_afstand(void);    // afstand in mm

#endif