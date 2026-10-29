#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

#define DISPLAY_ADRES  0x39

// Zet de I2C-bus klaar en zet het 7-segmentendisplay uit.
void display_init(void);

void display_update(uint8_t grootte);

#endif