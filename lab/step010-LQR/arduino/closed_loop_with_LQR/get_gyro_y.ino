/**
 * Polls gyro signal around y-axis from MPU6050 sensor. 
 */
#include "get_gyro_y.h"
float get_gyro_y(uint8_t the_mpu_address){
  Wire.beginTransmission(the_mpu_address);
  Wire.write(MPU_GYRO_YOUT_H);     // start reading at register GYRO_YOUT_H= high byte for gyro signal along y-axis
  Wire.endTransmission(false); // send all bytes, false means arduino remains to be master
  Wire.requestFrom(MPU_ADDRESS, 2, true);  // request a total of 2 registers = 2 bytes
  float gyro_y_raw = Wire.read() << 8 | Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  return gyro_y_raw; 
}
