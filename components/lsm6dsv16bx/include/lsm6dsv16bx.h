
/* Includes ------------------------------------------------------------------*/
#include "lsm6dsv16bx_reg.h"
#include "config.h"
#include "driver/i2c.h"
#include <stdio.h>
#include <string.h>

/* Private macro -------------------------------------------------------------*/
#define    BOOT_TIME            10 //ms

typedef enum {
    MODE_TDMENABLE,
    MODE_SIMPTDM,
    MODE_FIFOENABLE,
    MODE_DATAPOLLING,
} LSM6DSV16BX_InitMode;

class LSM6dsv16bx_System
{
private:
    uint8_t whoamI;
    uint8_t tx_buffer[1000];
    int16_t data_raw_acceleration[3];
    int16_t data_raw_angular_rate[3];
    int16_t data_raw_temperature;
    float acceleration_mg[3];
    float angular_rate_mdps[3];
    float temperature_degC;
    lsm6dsv16bx_filt_settling_mask_t filt_settling_mask;
    lsm6dsv16bx_xl_axis_t xl_axis;

    int16_t *datax;
    int16_t *datay;
    int16_t *dataz;

    void SetupDataPulling();
    void SetupFIFO();
    void SetupTDM();
    void SetupSimpleTDM();
    void lsm6dsv16bx_mlc_activity_handler();

    bool SerialOutputToggle;
    uint8_t activity_event_catched = 0;

    lsm6dsv16bx_fifo_status_t fifo_status;
    lsm6dsv16bx_all_sources_t all_sources;
    lsm6dsv16bx_reset_t rst;
    stmdev_ctx_t dev_ctx;
    
public:
    LSM6dsv16bx_System();
    ~LSM6dsv16bx_System();

    /*
    * @brief  Initialize the setting
    */
    void InitializedSystem(LSM6DSV16BX_InitMode initmode);

    /*
    * @brief  Put in everyloop to fetch the sample
    */
    void ProcessLoop();
    // void FIFOLoop();
    /*
    * @brief  Enable to have all the autooutput
    */
    void SetSerialOutput(bool out){ SerialOutputToggle = out;}

    float* GetAcc() { return acceleration_mg; }
    float* GetAng() { return angular_rate_mdps; }
    float GetTemp() { return temperature_degC;}

    void StateMachineSetup();
    // void StateLoop();


    static int32_t platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len);
    static int32_t platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len);
};

