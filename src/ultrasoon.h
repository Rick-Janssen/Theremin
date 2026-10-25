#ifndef ULTRASOON_H
#define ULTRASOON_H

#include <stdint.h>

#define TICK_US         4     // 1 timertick = 4 us
#define US_PER_CM       58
#define MAX_AFSTAND_CM  120
#define ECHO_MAX_TICKS  ((MAX_AFSTAND_CM * US_PER_CM) / TICK_US)   

void     ultrasoon_init(void);
uint8_t  ultrasoon_nieuw(void);
uint16_t ultrasoon_lees(void);   

#endif