#ifndef FILTER_H
#define FILTER_H

#include <stdint.h>

#define FILTER_MIN  1
#define FILTER_MAX  15

void     filter_init(void);                    
uint16_t filter_verwerk(uint16_t waarde);      // nieuwe waarde erin, mediaan eruit
uint8_t  filter_grootte(void);                 // huidige grootte van het buffer

#endif