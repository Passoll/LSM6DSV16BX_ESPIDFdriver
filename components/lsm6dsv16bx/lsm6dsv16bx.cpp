#include <stdio.h>
#include "lsm6dsv16bx.h"
#include "lsm6dsv16bx_reg.h"
#include "ad_wire.h"
#include "esp_log.h"

static const char *TAG2 = "lsmsys";

/* Main Example --------------------------------------------------------------*/

LSM6dsv16bx_System::LSM6dsv16bx_System(){
  SerialOutputToggle = false;
}

LSM6dsv16bx_System::~LSM6dsv16bx_System(){

}

void LSM6dsv16bx_System::InitializedSystem(LSM6DSV16BX_InitMode initmode){

  /* Uncomment to configure INT 1 */
  //lsm6dsv16bx_pin_int1_route_t int1_route;
  /* Uncomment to configure INT 2 */
  //lsm6dsv16bx_pin_int2_route_t int2_route;
  /* Initialize mems driver interface */
  dev_ctx.write_reg = platform_write;
  dev_ctx.read_reg = platform_read;
  dev_ctx.handle = nullptr;
  /* Check device ID */
  lsm6dsv16bx_device_id_get(&dev_ctx, &whoamI);
  ESP_LOGI(TAG2, "WHO_AM_I = %X", whoamI);
  if (whoamI != LSM6DSV16BX_ID){
    ESP_LOGI(TAG2,"The id is not correct, please check the connection or slave address");
    while (1);
  }
  ESP_LOGI(TAG2,"id true");
 
  switch (initmode)
  {
    case MODE_TDMENABLE:
      SetupTDM();
      break;
    case MODE_DATAPOLLING:
      SetupDataPulling();
      break;
    case MODE_FIFOENABLE:
      SetupFIFO();
      break;
    case MODE_SIMPTDM:
      SetupSimpleTDM();
      break;
    default:
      SetupDataPulling();
      break;
  }

}

void LSM6dsv16bx_System::SetupDataPulling(){
  /* Restore default configuration */
  lsm6dsv16bx_reset_set(&dev_ctx, LSM6DSV16BX_RESTORE_CTRL_REGS);
  do {
    lsm6dsv16bx_reset_get(&dev_ctx, &rst);
  } while (rst != LSM6DSV16BX_READY);

  /* Enable Block Data Update */
  lsm6dsv16bx_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
  /* Set Output Data Rate.
   * Selected data rate have to be equal or greater with respect
   * with MLC data rate.
   */
  lsm6dsv16bx_xl_data_rate_set(&dev_ctx, LSM6DSV16BX_XL_ODR_AT_7Hz5);
  lsm6dsv16bx_gy_data_rate_set(&dev_ctx, LSM6DSV16BX_GY_ODR_AT_15Hz);
  /* Set full scale */
  lsm6dsv16bx_xl_full_scale_set(&dev_ctx, LSM6DSV16BX_2g);
  lsm6dsv16bx_gy_full_scale_set(&dev_ctx, LSM6DSV16BX_2000dps);
  /* Configure filtering chain */
  filt_settling_mask.drdy = PROPERTY_ENABLE;
  filt_settling_mask.irq_xl = PROPERTY_ENABLE;
  filt_settling_mask.irq_g = PROPERTY_ENABLE;
  lsm6dsv16bx_filt_settling_mask_set(&dev_ctx, filt_settling_mask);
  lsm6dsv16bx_filt_gy_lp1_set(&dev_ctx, PROPERTY_ENABLE);
  lsm6dsv16bx_filt_gy_lp1_bandwidth_set(&dev_ctx, LSM6DSV16BX_GY_ULTRA_LIGHT);
  lsm6dsv16bx_filt_xl_lp2_set(&dev_ctx, PROPERTY_ENABLE);
  lsm6dsv16bx_filt_xl_lp2_bandwidth_set(&dev_ctx, LSM6DSV16BX_XL_STRONG);
}

void LSM6dsv16bx_System::SetupFIFO(){
  /* Restore default configuration */
  lsm6dsv16bx_reset_set(&dev_ctx, LSM6DSV16BX_RESTORE_CTRL_REGS);
  do {
    lsm6dsv16bx_reset_get(&dev_ctx, &rst);
  } while (rst != LSM6DSV16BX_READY);

  /* Enable Block Data Update */
  lsm6dsv16bx_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
  /* Set full scale */
  lsm6dsv16bx_xl_full_scale_set(&dev_ctx, LSM6DSV16BX_2g);
  lsm6dsv16bx_gy_full_scale_set(&dev_ctx, LSM6DSV16BX_2000dps);
  /* Configure filtering chain */
  filt_settling_mask.drdy = PROPERTY_ENABLE;
  filt_settling_mask.irq_xl = PROPERTY_ENABLE;
  filt_settling_mask.irq_g = PROPERTY_ENABLE;
  lsm6dsv16bx_filt_settling_mask_set(&dev_ctx, filt_settling_mask);
  lsm6dsv16bx_filt_gy_lp1_set(&dev_ctx, PROPERTY_ENABLE);
  lsm6dsv16bx_filt_gy_lp1_bandwidth_set(&dev_ctx, LSM6DSV16BX_GY_ULTRA_LIGHT);
  lsm6dsv16bx_filt_xl_lp2_set(&dev_ctx, PROPERTY_ENABLE);
  lsm6dsv16bx_filt_xl_lp2_bandwidth_set(&dev_ctx, LSM6DSV16BX_XL_STRONG);

  /*
   * Set FIFO watermark (number of unread sensor data TAG + 6 bytes
   * stored in FIFO) to 10 samples
   */
  lsm6dsv16bx_fifo_watermark_set(&dev_ctx, 50);
  lsm6dsv16bx_fifo_stop_on_wtm_set(&dev_ctx, PROPERTY_ENABLE);
  lsm6dsv16bx_fifo_xl_batch_set(&dev_ctx, LSM6DSV16BX_XL_BATCHED_AT_1920Hz);
  lsm6dsv16bx_fifo_gy_batch_set(&dev_ctx, LSM6DSV16BX_GY_BATCHED_AT_1920Hz);

  /* Set FIFO mode to Stream mode (aka Continuous Mode) */
  lsm6dsv16bx_fifo_mode_set(&dev_ctx, LSM6DSV16BX_STREAM_MODE);

  /* Set Output Data Rate.
   * Selected data rate have to be equal or greater with respect
   * with MLC data rate.
   */
  lsm6dsv16bx_xl_data_rate_set(&dev_ctx, LSM6DSV16BX_XL_ODR_AT_30Hz);
  lsm6dsv16bx_gy_data_rate_set(&dev_ctx, LSM6DSV16BX_GY_ODR_AT_60Hz);
}

void LSM6dsv16bx_System::SetupSimpleTDM(){
//A basic routine for configuring and enabling TDM mode is given below.
// 1. Configure the TDM mode through the TDM_CFG0, TDM_CFG1, and TDM_CFG2 registers.
// 2. Write 20h to the CTRL1 register.
// 3. Start acquiring data through the TDM interface.

    /*Restore Configuration*/
  lsm6dsv16bx_reset_set(&dev_ctx, LSM6DSV16BX_RESTORE_CTRL_REGS);
  do {
    lsm6dsv16bx_reset_get(&dev_ctx, &rst);
  } while (rst != LSM6DSV16BX_READY);

  /* Enable Block Data Update */
  // lsm6dsv16bx_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);

  // lsm6dsv16bx_tdm_dis_wclk_pull_up_set(&dev_ctx, PROPERTY_DISABLE);
  // lsm6dsv16bx_tdm_tdmout_pull_up_set(&dev_ctx, PROPERTY_DISABLE);
  // lsm6dsv16bx_tdm_wclk_bclk_set(&dev_ctx, LSM6DSV16BX_WCLK_8kHZ_BCLK_2048kHz);
  // lsm6dsv16bx_tdm_bclk_edge_set(&dev_ctx, LSM6DSV16BX_BCLK_FALLING);
  //lsm6dsv16bx_tdm_slot_set(&dev_ctx, LSM6DSV16BX_SLOT_012);
  // lsm6dsv16bx_tdm_delayed_conf_set(&dev_ctx, PROPERTY_DISABLE);
  // lsm6dsv16bx_tdm_axis_order_set(&dev_ctx, LSM6DSV16BX_TDM_ORDER_ZYX);
  // lsm6dsv16bx_tdm_xl_full_scale_set(&dev_ctx, LSM6DSV16BX_TDM_2g);
  
  uint8_t cf0 = 0x82;//rising + 16 + 2048(config1) A1 falling + 8 2048
  lsm6dsv16bx_write_reg(&dev_ctx, LSM6DSV16BX_TDM_CFG0, &cf0 , 1);
  uint8_t cf1 = 0xE0;//default
  lsm6dsv16bx_write_reg(&dev_ctx, LSM6DSV16BX_TDM_CFG1, &cf1 , 1);
  uint8_t cf2 = 0x01;// mask off, 4g
  lsm6dsv16bx_write_reg(&dev_ctx, LSM6DSV16BX_TDM_CFG2, &cf2 , 1);
  uint8_t sp = 0x20;
  lsm6dsv16bx_write_reg(&dev_ctx, LSM6DSV16BX_CTRL1, &sp, 1);


}

void LSM6dsv16bx_System::SetupTDM(){

  /*Restore Configuration*/
  lsm6dsv16bx_reset_set(&dev_ctx, LSM6DSV16BX_RESTORE_CTRL_REGS);
  do {
    lsm6dsv16bx_reset_get(&dev_ctx, &rst);
  } while (rst != LSM6DSV16BX_READY);

  /* Enable Block Data Update */
  xl_axis.x = PROPERTY_ENABLE;
  xl_axis.y = PROPERTY_ENABLE;//change to enable
  xl_axis.z = PROPERTY_ENABLE;
  lsm6dsv16bx_xl_axis_set(&dev_ctx, xl_axis);

  /* Enable Block Data Update */
  lsm6dsv16bx_block_data_update_set(&dev_ctx, PROPERTY_ENABLE);
  /* Set Output Data Rate */
  lsm6dsv16bx_xl_data_rate_set(&dev_ctx, LSM6DSV16BX_XL_ODR_AT_30Hz);
  lsm6dsv16bx_gy_data_rate_set(&dev_ctx, LSM6DSV16BX_GY_ODR_AT_60Hz);
  /* Set full scale */
  lsm6dsv16bx_xl_full_scale_set(&dev_ctx, LSM6DSV16BX_2g);
  lsm6dsv16bx_gy_full_scale_set(&dev_ctx, LSM6DSV16BX_2000dps);
  /* Configure filtering chain */
  filt_settling_mask.drdy = PROPERTY_ENABLE;
  filt_settling_mask.irq_xl = PROPERTY_ENABLE;
  filt_settling_mask.irq_g = PROPERTY_ENABLE;
  filt_settling_mask.tdm_excep_code = PROPERTY_ENABLE;
  lsm6dsv16bx_filt_settling_mask_set(&dev_ctx, filt_settling_mask);
  lsm6dsv16bx_filt_gy_lp1_set(&dev_ctx, PROPERTY_ENABLE);
  lsm6dsv16bx_filt_gy_lp1_bandwidth_set(&dev_ctx, LSM6DSV16BX_GY_ULTRA_LIGHT);
  lsm6dsv16bx_filt_xl_lp2_set(&dev_ctx, PROPERTY_ENABLE);
  lsm6dsv16bx_filt_xl_lp2_bandwidth_set(&dev_ctx, LSM6DSV16BX_XL_STRONG);

  lsm6dsv16bx_xl_mode_set(&dev_ctx,LSM6DSV16BX_XL_HP_TDM_MD);//add the tdm support 

  /* Configure TDM */
  lsm6dsv16bx_tdm_dis_wclk_pull_up_set(&dev_ctx, PROPERTY_DISABLE);
  lsm6dsv16bx_tdm_tdmout_pull_up_set(&dev_ctx, PROPERTY_DISABLE);
  lsm6dsv16bx_tdm_wclk_bclk_set(&dev_ctx, LSM6DSV16BX_WCLK_16kHZ_BCLK_2048kHz);
  lsm6dsv16bx_tdm_bclk_edge_set(&dev_ctx, LSM6DSV16BX_BCLK_RISING);
  lsm6dsv16bx_tdm_slot_set(&dev_ctx, LSM6DSV16BX_SLOT_012);
  lsm6dsv16bx_tdm_delayed_conf_set(&dev_ctx, PROPERTY_DISABLE);
  lsm6dsv16bx_tdm_axis_order_set(&dev_ctx, LSM6DSV16BX_TDM_ORDER_ZYX);
  lsm6dsv16bx_tdm_xl_full_scale_set(&dev_ctx, LSM6DSV16BX_TDM_4g);
  
}

// void LSM6dsv16bx_System::StateMachineSetup(){
//     uint32_t i;
//     /* Restore default configuration */
//     lsm6dsv16bx_reset_set(&dev_ctx, LSM6DSV16BX_RESTORE_CTRL_REGS);
//     do {
//       lsm6dsv16bx_reset_get(&dev_ctx, &rst);
//     } while (rst != LSM6DSV16BX_READY);

//     /* Start Machine Learning Core configuration */
//     for ( i = 0; i < (sizeof(lsm6dsv16bx_activity_recognition_for_wrist) /
//                       sizeof(ucf_line_t) ); i++ ) {
//       lsm6dsv16bx_write_reg(&dev_ctx, lsm6dsv16bx_activity_recognition_for_wrist[i].address,
//                         (uint8_t *)&lsm6dsv16bx_activity_recognition_for_wrist[i].data, 1);
//     }
// }

void LSM6dsv16bx_System::lsm6dsv16bx_mlc_activity_handler(void)
{
  lsm6dsv16bx_all_sources_t status;
  lsm6dsv16bx_mlc_out_t mlc_out;

  /* Read output only if new xl value is available */
  lsm6dsv16bx_all_sources_get(&dev_ctx, &status);

  if (status.mlc1) {
    lsm6dsv16bx_mlc_out_get(&dev_ctx, &mlc_out);
    activity_event_catched = mlc_out.mlc1_src;
  }
}

// void LSM6dsv16bx_System::FIFOLoop(){
//   String out;
//   uint16_t num = 0;
//     /* Read watermark flag */
//   lsm6dsv16bx_fifo_status_get(&dev_ctx, &fifo_status);

//   if (fifo_status.fifo_th == 1) {
//     num = fifo_status.fifo_level;
//     while (num--) {
//       lsm6dsv16bx_fifo_out_raw_t f_data;
//       /* Read FIFO tag */
//       lsm6dsv16bx_fifo_out_raw_get(&dev_ctx, &f_data);
//       dataz = (int16_t *)&f_data.data[0]; /* axis are inverted */
//       datay = (int16_t *)&f_data.data[2];
//       datax = (int16_t *)&f_data.data[4];

//       switch (f_data.tag) {
//         case lsm6dsv16bx_fifo_out_raw_t::LSM6DSV16BX_XL_NC_TAG:
//           out  = "Acceleration [mg]:" + String(lsm6dsv16bx_from_fs2_to_mg(*datax))
//           + " " + String(lsm6dsv16bx_from_fs2_to_mg(*datay))
//           + " " + String(lsm6dsv16bx_from_fs2_to_mg(*dataz));
//           if(SerialOutputToggle) Serial.println(out);
//           break;
          
//         case lsm6dsv16bx_fifo_out_raw_t::LSM6DSV16BX_GY_NC_TAG:
//           out = "GYR [mdps]: " 
//           + String(lsm6dsv16bx_from_fs2000_to_mdps(*datax))
//           + " " + String(lsm6dsv16bx_from_fs2000_to_mdps(*datay))
//           + " " + String(lsm6dsv16bx_from_fs2000_to_mdps(*dataz));
//           if(SerialOutputToggle) Serial.println(out);
//           break;

//         default:
//           /* Flush unused samples */
//           break;
//       }
//     }
//   }
// }


// void LSM6dsv16bx_System::StateLoop(){
//     lsm6dsv16bx_mlc_activity_handler();
//     //
//     if (activity_event_catched != 0x0) {
//       switch(activity_event_catched) {
//       case 1:
//         //sprintf((char *)tx_buffer, "Stationary\r\n");
//         Serial.println("Stationary");
//         break;
//       case 4:
//         //sprintf((char *)tx_buffer, "Walking\r\n");
//         Serial.println("Walking");
//         break;
//       case 8:
//         //sprintf((char *)tx_buffer, "Jogging/Running\r\n");
//         Serial.println("Jogging/Running");
//         break;
//       }
//       Serial.println(activity_event_catched);
//       activity_event_catched = 0;
//     }
// }

void LSM6dsv16bx_System::ProcessLoop(){

  //TOD1: Switch the get mode
  //lsm6dsv16bx_flag_data_ready_get(&dev_ctx, &drdy);
  lsm6dsv16bx_all_sources_get(&dev_ctx, &all_sources);
 
  if (all_sources.drdy_xl) {
    /* Read acceleration field data */
    memset(data_raw_acceleration, 0x00, 3 * sizeof(int16_t));
    lsm6dsv16bx_acceleration_raw_get(&dev_ctx, data_raw_acceleration);
    acceleration_mg[0] =
      lsm6dsv16bx_from_fs2_to_mg(data_raw_acceleration[0]);
    acceleration_mg[1] =
      lsm6dsv16bx_from_fs2_to_mg(data_raw_acceleration[1]);
    acceleration_mg[2] =
      lsm6dsv16bx_from_fs2_to_mg(data_raw_acceleration[2]);
    if(SerialOutputToggle) ESP_LOGI(TAG2, "ACC = %4.2f \t %4.2f \t %4.2f", acceleration_mg[0], acceleration_mg[1] , acceleration_mg[2]);
  }

  if (all_sources.drdy_gy) {
    /* Read angular rate field data */
    memset(data_raw_angular_rate, 0x00, 3 * sizeof(int16_t));
    lsm6dsv16bx_angular_rate_raw_get(&dev_ctx, data_raw_angular_rate);
    angular_rate_mdps[0] =
      lsm6dsv16bx_from_fs2000_to_mdps(data_raw_angular_rate[0]);
    angular_rate_mdps[1] =
      lsm6dsv16bx_from_fs2000_to_mdps(data_raw_angular_rate[1]);
    angular_rate_mdps[2] =
      lsm6dsv16bx_from_fs2000_to_mdps(data_raw_angular_rate[2]);
    if(SerialOutputToggle) ESP_LOGI(TAG2, "ANG = %4.2f \t %4.2f \t %4.2f", angular_rate_mdps[0], angular_rate_mdps[1] , angular_rate_mdps[2]);
  }

  if (all_sources.drdy_gy) {
    /* Read temperature data */
    memset(&data_raw_temperature, 0x00, sizeof(int16_t));
    lsm6dsv16bx_temperature_raw_get(&dev_ctx, &data_raw_temperature);
    temperature_degC = lsm6dsv16bx_from_lsb_to_celsius(
                          data_raw_temperature);
    if(SerialOutputToggle) ESP_LOGI(TAG2, "TEMP = %f", temperature_degC);
  }
  //if()
  
  //TOD2 : We can get more data here
}


/*
 * @brief  Write generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to write
 * @param  bufp      pointer to data to write in register reg
 * @param  len       number of consecutive register to write
 *
 */
int32_t LSM6dsv16bx_System::platform_write(void *handle, uint8_t reg, const uint8_t *bufp,
                              uint16_t len)
{
  esp_err_t ret = ESP_OK;
  for(int i = 0; i < len; i++){
    I2C_TransferSeq_TypeDef seq;
    uint8_t i2c_write_data[2];
    uint8_t i2c_read_data[1];

    seq.addr = LSM6DSV16BX_SLAVE_ADDRESS;
    seq.flags = I2C_FLAG_WRITE;
    /* Select length of data to be read */
    i2c_write_data[0] = reg + i;
    i2c_write_data[1] = bufp[i];
    seq.buf[0].data = i2c_write_data;
    seq.buf[0].len  = 2;
    /* Select length of data to be read */
    seq.buf[1].data = i2c_read_data;
    seq.buf[1].len  = 0;

    ret = I2CSPM_Transfer(&seq);
  }
  
  return (int32_t)ret;
}

/*
 * @brief  Read generic device register (platform dependent)
 *
 * @param  handle    customizable argument. In this examples is used in
 *                   order to select the correct sensor bus handler.
 * @param  reg       register to read
 * @param  bufp      pointer to buffer that store the data read
 * @param  len       number of consecutive register to read
 *
 */
int32_t LSM6dsv16bx_System::platform_read(void *handle, uint8_t reg, uint8_t *bufp,
                             uint16_t len)
{
  esp_err_t ret = ESP_OK;
  for(int i = 0; i < len; i++){
    
    I2C_TransferSeq_TypeDef seq;
    uint8_t i2c_write_data[1];

    seq.addr = LSM6DSV16BX_SLAVE_ADDRESS;
    seq.flags = I2C_FLAG_WRITE_READ;
    /* Select length of data to be read */
    i2c_write_data[0] = reg + i;
    seq.buf[0].data = i2c_write_data;
    seq.buf[0].len  = 1;
    /* Select length of data to be read */
    seq.buf[1].data = &bufp[i];
    seq.buf[1].len  = 1;

    ret = I2CSPM_Transfer(&seq);
  }
  if (ret != ESP_OK) {
    *bufp = 0x00;
    return (int32_t)ret;
  }
  return (int32_t)ret;
}
