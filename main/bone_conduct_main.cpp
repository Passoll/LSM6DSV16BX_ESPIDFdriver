#include <iostream>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s_tdm.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_check.h"
#include "lsm6dsv16bx.h"
#include "ad_wire.h"

static const char *TAG = "esp32";

LSM6dsv16bx_System lsmsys;

extern "C" {

#define EXAMPLE_I2S_DUPLEX_MODE     0

#define EXAMPLE_TDM_BCLK_IO1        GPIO_NUM_4      // I2S bit clock io number
#define EXAMPLE_TDM_WS_IO1          GPIO_NUM_5     // I2S word select io number
#define EXAMPLE_TDM_DOUT_IO1         GPIO_NUM_18      // I2S data in io number
#define EXAMPLE_SLOT_CONFIG         (I2S_TDM_SLOT0 | I2S_TDM_SLOT1 | I2S_TDM_SLOT2) //3 channel

#define EXAMPLE_BUFF_SIZE               128 // 128 slot per wclk, 2048 / 16

#define EXAMPLE_I2S_SAMPLE_RATE    (16000) // wclk
#define EXAMPLE_I2S_MCLK_MULTIPLE  (I2S_MCLK_MULTIPLE_512)
#define EXAMPLE_I2S_BCLK_DIV_MCLK  4 //mclk = 16k * multiple. bclk = mclk / div = 2048

static i2s_chan_handle_t                tx_chan;        // I2S tx channel handler
static i2s_chan_handle_t                rx_chan;        // I2S rx channel handler

static int16_t r_buf[EXAMPLE_BUFF_SIZE];

static void i2s_example_read_task()
{
    assert(r_buf); // Check if r_buf allocation success
    size_t r_bytes = 0;

    /* Enable the RX channel */
    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));   
    // int tt = i2s_channel_enable(rx_chan);
    // printf( "%i     ", tt);
    /* ATTENTION: The print and delay in the read task only for monitoring the data by human,
     * Normally there shouldn't be any delays to ensure a short polling time,
     * Otherwise the dma buffer will overflow and lead to the data lost */
    while (1) {
        /* Read i2s data */
        int jj = i2s_channel_read(rx_chan, r_buf, sizeof(r_buf), &r_bytes, pdMS_TO_TICKS(1000));
        //printf( "%i", jj);
        if (jj == ESP_OK) {
            //printf("Read Task: i2s read %d bytes\n-----------------------------------\n", r_bytes);
            // printf("[0] %x [1] %x [2] %x [3] %x\n[4] %x [5] %x [6] %x [7] %x\n\n",
            //        r_buf[0], r_buf[1], r_buf[2], r_buf[3], r_buf[4], r_buf[5], r_buf[6], r_buf[7]);
            printf("slot1:%d slot2:%d slot3:%d \n",
                    r_buf[0], r_buf[1], r_buf[2]);

        } else {
            printf("Read Task: i2s read failed\n");
        }
        //vTaskDelay(pdMS_TO_TICKS(200));
    }
    free(r_buf);
    vTaskDelete(NULL);
}


static void i2s_example_init_tdm_duplex(void)
{
    /* Setp 1: Determine the I2S channel configuration and allocate both channels
     * The default configuration can be generated by the helper macro,
     * it only requires the I2S controller id and I2S role */

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &rx_chan));

    /* Step 2: Setting the configurations of TDM mode, and initialize rx & tx channels
     * The slot configuration and clock configuration can be generated by the macros
     * These two helper macros is defined in 'i2s_tdm.h' which can only be used in TDM mode.
     * They can help to specify the slot and clock configurations for initialization or re-configuring */
    i2s_tdm_config_t tdm_cfg = {
        .clk_cfg  = {
            .sample_rate_hz = EXAMPLE_I2S_SAMPLE_RATE,
            .clk_src = I2S_CLK_SRC_DEFAULT,
            .mclk_multiple = EXAMPLE_I2S_MCLK_MULTIPLE,
            .bclk_div = EXAMPLE_I2S_BCLK_DIV_MCLK //mclk to bclk 
        },
        /* For the target that not support full data bit-width in multiple slots (e.g. ESP32C3, ESP32S3, ESP32C6)
         * The maximum bits in one frame is limited by the hardware, the number of bit clock can't exceed 128 in one frame,
         * which is to say, TDM mode can only support 32 bit-width data upto 4 slots,
         * 16 bit-width data upto 8 slots and 8 bit-width data upto 16 slots
         * But for the target that support full data bit-width in multiple slots (e.g. ESP32H2)
         * There is no such limitation, it can support up to 32 bit-width with 16 slots */
        .slot_cfg = I2S_TDM_PCM_LONG_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO, i2s_tdm_slot_mask_t(EXAMPLE_SLOT_CONFIG)),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,    // some codecs may require mclk signal, this example doesn't need it
            .bclk = EXAMPLE_TDM_BCLK_IO1,
            .ws   = EXAMPLE_TDM_WS_IO1,
            .dout = I2S_GPIO_UNUSED,
            .din  = GPIO_NUM_18,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };
    /* While there are more than two slots activated, receiving data might be wrong if the mclk multiple is too small
     * The driver will increasing the multiple automatically to ensure the bclk_div is greater than 2.
     * Modify the mclk_multiple to 512 directly here to avoid the warning */
    //tdm_cfg.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_256;
    //tdm_cfg.intr_alloc_flags = (ESP_INTR_FLAG_LOWMED | ESP_INTR_FLAG_SHARED | ESP_INTR_FLAG_INTRDISABLED),
    /* Initialize the channels */
    //ESP_ERROR_CHECK(i2s_channel_init_tdm_mode(tx_chan, &tdm_cfg));
    ESP_ERROR_CHECK(i2s_channel_init_tdm_mode(rx_chan, &tdm_cfg));
}

void app_main(void)
{
    //uint8_t data[2];
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAG, "I2C initialized successfully");

    lsmsys.InitializedSystem(MODE_SIMPTDM);
    lsmsys.SetSerialOutput(false);
    i2s_example_init_tdm_duplex();
    i2s_example_read_task();

    //i2s_example_read_task();
    //xTaskCreate(i2s_example_read_task, "i2s_example_read_task", 4096, NULL, 5, NULL);
    //xTaskCreate(i2s_example_write_task, "i2s_example_write_task", 4096, NULL, 5, NULL);
    // while (1)
    // {
    //    // lsmsys.ProcessLoop();
    // }
    // ESP_ERROR_CHECK(i2c_driver_delete(I2C_MASTER_NUM));
    // ESP_LOGI(TAG, "I2C de-initialized successfully");
}



}
