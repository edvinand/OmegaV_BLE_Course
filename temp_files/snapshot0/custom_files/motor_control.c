#include "motor_control.h"

#define LOG_MODULE_NAME motor_control
LOG_MODULE_REGISTER(LOG_MODULE_NAME);


/* Config */
#define SERVO_PIN                           3
#define PWM_PERIOD                          20000
static nrfx_pwm_t pwm = NRFX_PWM_INSTANCE(1);
static nrf_pwm_values_individual_t position_1[] = {
    {19450},
};
static nrf_pwm_values_individual_t position_2[] = {
    {17500},
};

static nrf_pwm_sequence_t position_1_sequence = {
    .values.p_individual    = position_1,
    .length                 = NRF_PWM_VALUES_LENGTH(position_1),
    .repeats                = 0,
    .end_delay              = 0
};
static nrf_pwm_sequence_t position_2_sequence = {
    .values.p_individual    = position_2,
    .length                 = NRF_PWM_VALUES_LENGTH(position_2),
    .repeats                = 0,
    .end_delay              = 0
};


//void set_gpio_voltage(void) // If your motor is struggling, try calling this function from motor_init().
//{
//    uint32_t regout = NRF_UICR->REGOUT0;
//    uint32_t target_voltage = UICR_REGOUT0_VOUT_3V3;
//
//    LOG_INF("Regout: 0x%08x", regout);
//    if ((regout & UICR_REGOUT0_VOUT_Msk) != target_voltage) {
//        LOG_INF("regout not set to 3V3. Configuring...");
//        
//
//        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Wen;
//        while (NRF_NVMC->CONFIG != NVMC_CONFIG_WEN_Wen){
//            // Wait...
//        }
//        NRF_UICR->REGOUT0 = (target_voltage | ~UICR_REGOUT0_VOUT_Msk);
//        while (NRF_NVMC->READY == NVMC_READY_READY_Busy) {
//            // Wait...
//        }
//        while ((NRF_UICR->REGOUT0 & UICR_REGOUT0_VOUT_Msk) != target_voltage) {
//            // Wait...
//        }
//        NRF_NVMC->CONFIG = NVMC_CONFIG_WEN_Ren;
//        while (NRF_NVMC->CONFIG != NVMC_CONFIG_WEN_Ren){
//            // Wait...
//        }
//        NVIC_SystemReset();
//    } else {
//        LOG_INF("Regout0 is set to target voltage.");
//    }
//
//}

int motor_init(void)
{
    LOG_INF("Initializing Motor Control");

    //set_gpio_voltage();

    nrfx_err_t err;
    nrfx_pwm_config_t pwm_config    = NRFX_PWM_DEFAULT_CONFIG(SERVO_PIN, NRFX_PWM_PIN_NOT_USED, NRFX_PWM_PIN_NOT_USED, NRFX_PWM_PIN_NOT_USED);
    pwm_config.top_value            = PWM_PERIOD;
    pwm_config.load_mode            = NRF_PWM_LOAD_INDIVIDUAL;
    
    err = nrfx_pwm_init(&pwm, &pwm_config, NULL, NULL);
    if (err != NRFX_SUCCESS) {  // NB: NRFX_SUCCESS != 0
        LOG_ERR("nrfx_pwm_init() failed, err %d", err);
    }

    nrfx_pwm_simple_playback(&pwm, &position_1_sequence, 50, NRFX_PWM_FLAG_STOP);

    return 0;
}

void set_motor_angle(uint16_t angle)
{

    LOG_INF("setting angle %d", angle);
    if (angle == 1) {
        nrfx_pwm_simple_playback(&pwm, &position_1_sequence, 100, NRFX_PWM_FLAG_STOP);
    }
    else if (angle == 2) {
        nrfx_pwm_simple_playback(&pwm, &position_2_sequence, 100, NRFX_PWM_FLAG_STOP);
    }

}