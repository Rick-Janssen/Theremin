#ifndef ULTRASOON_H
#define ULTRASOON_H

#include <stdint.h>

void     ultrasoon_init(void);
uint8_t  ultrasoon_nieuw(void);     // 1 als er een nieuwe meting is
uint16_t ultrasoon_lees(void);      // echobreedte in ticks van 4 us

#endif