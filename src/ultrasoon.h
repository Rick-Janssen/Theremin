#ifndef ULTRASOON_H
#define ULTRASOON_H

#include <stdint.h>

#define ULTRASOON_MIN_AFSTAND  20 //in mm dus 2 cm 
#define ULTRASOON_MAX_AFSTAND  650 // in mm dus 65 cm

void     ultrasoon_init(void);
uint8_t  ultrasoon_nieuwe_meting(void);
uint16_t ultrasoon_lees_afstand(void);      

#endif