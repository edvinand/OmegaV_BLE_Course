#include <zephyr.h>
#include <logging/log.h>
#include "nrfx_pwm.h"
#include <drivers/pwm.h>

int motor_init(void);
void set_motor_angle(uint16_t angle);