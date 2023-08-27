#include "ad_wire.h"

esp_err_t I2CSPM_Transfer(I2C_TransferSeq_TypeDef *seq) {
  
  if(seq->flags == I2C_FLAG_WRITE){
    return esp32_register_write_byte(seq->buf[0].data[0], seq->buf[0].data[1]);
  }
  
  else if(seq->flags == I2C_FLAG_WRITE_READ) {
    return esp32_register_read(seq->buf[0].data[0], seq->buf[1].data, 1);
  }
  
  else return 0;
}


