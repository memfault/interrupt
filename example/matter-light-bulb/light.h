#ifndef LIGHT_H_
#define LIGHT_H_

#include <stdbool.h>
#include <stdint.h>

/* Initialize PWM for LEDs. Call AFTER Nrf::GetBoard().Init(). */
void light_init(void);

void light_set(bool on);
bool light_get(void);
void light_toggle(void);

/* Brightness: 0 = off, 1–254 = dim to full */
void light_set_level(uint8_t level);
uint8_t light_get_level(void);

#endif /* LIGHT_H_ */
