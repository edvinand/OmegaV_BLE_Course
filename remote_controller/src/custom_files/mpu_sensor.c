#include "mpu_sensor.h"


#define LOG_MODULE_NAME mpu_sensor
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

const nrfx_twim_t m_twim_instance       = NRFX_TWIM_INSTANCE(0);

#define MPU_TWI_BUFFER_SIZE             14
#define MPU_TWI_TIMEOUT                 10000
#define MPU_ADDRESS                     0x68
volatile static bool twi_xfer_done      = false;
uint8_t twi_tx_buffer[MPU_TWI_BUFFER_SIZE];

int app_mpu_tx(const nrfx_twim_t *  p_instance,
                uint8_t             address,
                uint8_t *           p_data,
                uint8_t             length,
                bool                no_stop)
{
    int err;

    nrfx_twim_xfer_desc_t xfer = NRFX_TWIM_XFER_DESC_TX(address, p_data, length);
    err = nrfx_twim_xfer(p_instance, &xfer, 0);
    if (err != NRFX_SUCCESS) {
        return err;
    }

    return 0;
}

int app_mpu_rx(const nrfx_twim_t *   p_instance,
               uint8_t               address,
               uint8_t *             p_data,
               uint8_t               length)
{
    int err;
    nrfx_twim_xfer_desc_t xfer = NRFX_TWIM_XFER_DESC_RX(address, p_data, length);

    err = nrfx_twim_xfer(p_instance, &xfer, 0);
    if (err != NRFX_SUCCESS) {
        return err;
    }
    return 0;
}

int wait_for_xfer_done(void)
{
    int timeout = MPU_TWI_TIMEOUT;
    while ((!twi_xfer_done) && --timeout)
    {
        // Wait...
    }
    if (timeout == 0) {
        return NRFX_ERROR_TIMEOUT;
    }
    return 0;
}

int app_mpu_write_single_register(uint8_t reg, uint8_t data)
{
    int err;

    uint8_t packet[2] = {reg, data};

    twi_xfer_done = false;  // reset for new xfer
    err = app_mpu_tx(&m_twim_instance, MPU_ADDRESS, packet, 2, false);
    if (err) {
        return err;
    }
    err = wait_for_xfer_done();
    if (err == NRFX_ERROR_TIMEOUT) {
        return err;
    }
    
    return 0;
}

int app_mpu_write_registers(uint8_t reg, uint8_t * p_data, uint8_t length)
{
    int err;
    
    twi_tx_buffer[0] = reg;
    memcpy((twi_tx_buffer + 1), p_data, length);

    nrfx_twim_xfer_desc_t xfer = {0};
    xfer.address = MPU_ADDRESS;
    xfer.type = NRFX_TWIM_XFER_TX;
    xfer.primary_length = length+1;
    xfer.p_primary_buf = twi_tx_buffer;

    twi_xfer_done = false;  // reset for new xfer
    err = nrfx_twim_xfer(&m_twim_instance, &xfer,0);
    if (err != NRFX_SUCCESS) {
        return err;
    }
    err = wait_for_xfer_done();
    if (err == NRFX_ERROR_TIMEOUT) {
        return err;
    }

    return 0;
    
}

int app_mpu_read_registers(uint8_t reg, uint8_t * p_data, uint8_t length)
{
    int err;

    twi_xfer_done = false;  // reset for new xfer
    err = app_mpu_tx(&m_twim_instance, MPU_ADDRESS, &reg, 1, false);
    if (err) {
        return err;
    }
    err = wait_for_xfer_done();
    if (err == NRFX_ERROR_TIMEOUT) {
        return err;
    }

    twi_xfer_done = false;  // reset for new xfer
    err = app_mpu_rx(&m_twim_instance,MPU_ADDRESS, p_data, length);
    if (err) {
        LOG_ERR("app_mpu_rx returned %08x", err);
        return err;
    }
    err = wait_for_xfer_done();
    if (err == NRFX_ERROR_TIMEOUT) {
        return err;
    }

    return 0;
}

void my_twim_handler(nrfx_twim_evt_t const * p_event, void * p_context)
{
    // LOG_INF("TWIM callback");
    switch(p_event->type)
    {
        case NRFX_TWIM_EVT_DONE:
            twi_xfer_done = true;   // This is the event we are waiting for.
            break;
        case NRFX_TWIM_EVT_ADDRESS_NACK:
            LOG_ERR("address nack");
            break;
        case NRFX_TWIM_EVT_DATA_NACK:
            LOG_ERR("data nack");
            break;
        case NRFX_TWIM_EVT_OVERRUN:
            LOG_ERR("overrun");
            break;
        case NRFX_TWIM_EVT_BUS_ERROR:
            LOG_ERR("bus error");
            break;
        default:
            LOG_ERR("default (should never happen)");
            break;
    }
    
}

int twi_init(void)
{
    // Setup peripheral interrupt.
    IRQ_CONNECT(DT_IRQN(DT_NODELABEL(i2c0)),DT_IRQ(DT_NODELABEL(i2c0), priority), nrfx_isr, nrfx_twim_0_irq_handler,0);
    irq_enable(DT_IRQN(DT_NODELABEL(i2c0)));

    int err = 0;

    const nrfx_twim_config_t twim_config = {                          \
        .scl                = 4,                                      \
        .sda                = 3,                                      \
        .frequency          = NRF_TWIM_FREQ_400K,                     \
        .interrupt_priority = NRFX_TWIM_DEFAULT_CONFIG_IRQ_PRIORITY,  \
        .hold_bus_uninit    = false,                                  \
    };
    err = nrfx_twim_init(&m_twim_instance,
                          &twim_config,
                          my_twim_handler,
                          NULL);
                          
    if (err != NRFX_SUCCESS) {
        LOG_ERR("twim_init failed. (err %x)", err);
        return err;
    }

    nrfx_twim_enable(&m_twim_instance);
    
    return 0;
}

int app_mpu_config(void)
{
    int err = 0;

    uint8_t reset_value = 7;
    err = app_mpu_write_single_register(MPU_REG_SIGNAL_PATH_RESET, reset_value);
    if (err) {
        return err;
    }

    err = app_mpu_write_single_register(MPU_REG_PWR_MGMT_1, 1);
    if (err) {
        return err;
    }

    app_mpu_config_t mpu_config = {
        .smplrt_div                     = 19,             \
        .sync_dlpf_config.dlpf_cfg      = 1,              \
        .sync_dlpf_config.ext_sync_set  = 0,              \
        .gyro_config.fs_sel             = GFS_2000DPS,    \
        .accel_config.afs_sel           = AFS_2G,         \
        .accel_config.za_st             = 0,              \
        .accel_config.ya_st             = 0,              \
        .accel_config.xa_st             = 0,              \
    };

    uint8_t *data;
    data = (uint8_t*)&mpu_config; // Casting to a normal uint8_t so that app_mpu_write_register will accept it as an input parameter.
    
    err = app_mpu_write_registers(MPU_REG_SMPLRT_DIV, data, 4);
    return err;
}

int mpu_sensor_init(void)
{
    int err;

    LOG_INF("Initializing MPU Sensor");

    err = twi_init();
    if (err) {
        return err;
    }

    err = app_mpu_config();

    return err;
}

int read_accel_values(accel_values_t * p_accel_values)
{
    int err;
    uint8_t raw_values[6];
    err = app_mpu_read_registers(MPU_REG_ACCEL_XOUT_H, raw_values, 6);
    /** The data will come back bytewise in the following order (see registers from mpu6050.h) :        *
      * raw_values[0] = MPU_REG_ACCEL_XOUT_H     ACCEL_XOUT[15:8]                                       *
      * raw_values[1] = MPU_REG_ACCEL_XOUT_L     ACCEL_XOUT[ 7:0]                                       *
      * raw_values[2] = MPU_REG_ACCEL_YOUT_H     ACCEL_YOUT[15:8]                                       *
      * raw_values[3] = MPU_REG_ACCEL_YOUT_L     ACCEL_YOUT[ 7:0]                                       *
      * raw_values[4] = MPU_REG_ACCEL_ZOUT_H     ACCEL_ZOUT[15:8]                                       *
      * raw_values[5] = MPU_REG_ACCEL_ZOUT_L     ACCEL_ZOUT[ 7:0]                                       **/
    if (err) {
        LOG_ERR("Could not read accellerometer data. err: %d", err);
        return err;
    }

    p_accel_values->x = ((raw_values[0]<<8) + raw_values[1]);
    p_accel_values->y = ((raw_values[2]<<8) + raw_values[3]);
    p_accel_values->z = ((raw_values[4]<<8) + raw_values[5]);
    return 0;
}

int read_gyro_values(gyro_values_t * p_gyro_values)
{
    // Extra challenge: Implement this function yourself. Should be quite similar to read_accel_values().
    return 0;
}