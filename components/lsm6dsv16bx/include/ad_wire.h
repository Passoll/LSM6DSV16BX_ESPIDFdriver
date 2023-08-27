#pragma once

#include "driver/i2c.h"
#include "hal/i2c_types.h"

#define I2C_FLAG_WRITE 0
#define I2C_FLAG_WRITE_READ 1

#define I2C_MASTER_SCL_IO           CONFIG_I2C_MASTER_SCL      /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO           CONFIG_I2C_MASTER_SDA      /*!< GPIO number used for I2C master data  */
#define LSM6DSV16BX_SLAVE_ADDRESS 0x6A
#define I2C_MASTER_TIMEOUT_MS       1000

#define I2C_MASTER_NUM              I2C_NUM_0                          /*!< I2C master i2c port number, the number of i2c peripheral interfaces available will depend on the chip */
#define I2C_MASTER_FREQ_HZ          400000                     /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE   0                          /*!< I2C master doesn't need buffer */

typedef struct {
  uint8_t *data;
  uint8_t len;
} I2C_TransferBuf_TypeDef;

typedef struct {
  uint8_t addr;
  uint8_t flags;
  I2C_TransferBuf_TypeDef buf[2];
} I2C_TransferSeq_TypeDef;

/*-----------------------Status-----------------------*/


esp_err_t I2CSPM_Transfer(I2C_TransferSeq_TypeDef *seq);


/**
 * @brief Read a sequence of bytes
 */
static esp_err_t esp32_register_read(uint8_t reg_addr, uint8_t *data, size_t len)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, LSM6DSV16BX_SLAVE_ADDRESS, &reg_addr, 1, data, len, I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/**
 * @brief Write a byte 
 */
static esp_err_t esp32_register_write_byte(uint8_t reg_addr, uint8_t data)
{
    int ret;
    uint8_t write_buf[2] = {reg_addr, data};

    ret = i2c_master_write_to_device(I2C_MASTER_NUM, LSM6DSV16BX_SLAVE_ADDRESS, write_buf, sizeof(write_buf), I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

    return ret;
}

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void)
{
    i2c_port_t i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
    I2C_MODE_MASTER,
    I2C_MASTER_SDA_IO,
    I2C_MASTER_SCL_IO,
    GPIO_PULLUP_ENABLE,
    GPIO_PULLUP_ENABLE,
    { .master = { I2C_MASTER_FREQ_HZ } },
    };
    // i2c_config_t conf = {
    //     .mode = I2C_MODE_MASTER,
    //     .sda_io_num = I2C_MASTER_SDA_IO,
    //     .scl_io_num = I2C_MASTER_SCL_IO,
    //     .sda_pullup_en = GPIO_PULLUP_ENABLE,
    //     .scl_pullup_en = GPIO_PULLUP_ENABLE,
    //     .master.clk_speed = I2C_MASTER_FREQ_HZ,
    // };
    

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}
