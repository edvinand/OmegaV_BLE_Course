#include "motor_control.h"

#define LOG_MODULE_NAME motor_control
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define PWM_PERIOD_NS 20000000

static const struct pwm_dt_spec pwm_led0 = PWM_DT_SPEC_GET(DT_ALIAS(pwm_led0));

int motor_init(void)
{
    int err = 0;
    LOG_INF("Initializing Motor Control");

    if (!device_is_ready(pwm_led0.dev)) {
    LOG_ERR("Error: PWM device %s is not ready",
            pwm_led0.dev->name);
    return -EBUSY;
	}

    err = pwm_set_dt(&pwm_led0, PWM_PERIOD_NS, 1400000);
    if (err) {
        LOG_ERR("pwm_set_dt returned %d", err);
    }

    return err;
}

int set_motor_angle(uint32_t duty_cycle_ns)
{
    int err;
    err = pwm_set_dt(&pwm_led0, PWM_PERIOD_NS, duty_cycle_ns);
    if (err) {
        LOG_ERR("pwm_set_dt_returned %d", err);
    }

    return err;
}