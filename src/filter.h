#ifndef FILTER_H
#define FILTER_H

#include <stdint.h>

#define FILTER_MIN  1
#define FILTER_MAX  15
#define FILTER_START 5   // aantal samples na het opstarten

void     filter_init(void);                    
uint16_t filter_verwerk(uint16_t waarde);      // nieuwe waarde erin, mediaan eruit
uint8_t  filter_grootte(void);                 // ingestelde filtergrootte (aantal samples)

#endif