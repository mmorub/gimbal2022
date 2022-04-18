#include "get_angle_to_gravity.h"
/**
 * Returns angle to direction of gravity in degrees. 
 * 
 * Returns angle that sensor needs to be turned by about y axis 
 * in order to align the z-direction of the sensor with the direction of gravity.
 * Assumes y-axis to be perpendicular to direction of gravity.
 * Assumes positive direction of y-axis to point into direction of view of camera. 
 */
double get_angle_to_gravity(uint8_t the_mpu_address){
  const double rad2deg= 360.0/2.0/3.1416; 
  int16_t accel_x_raw, accel_z_raw;

  Wire.beginTransmission(the_mpu_address);
  Wire.write(0x3b);  // start reading at register 0x3b = ACCEL_XOUT_H
  Wire.endTransmission(false); // send all bytes, false means arduino remains to be master
  Wire.requestFrom(the_mpu_address, (uint8_t) 6, (uint8_t) true);  // request a total of 3*2 registers = 3*2 bytes
    // casts to uint8_t are needed because requestFrom(int, int, int) and requestFrom(uint8_t, uint8_t, uint8_t) exist
  accel_x_raw = Wire.read() << 8 | Wire.read();  // 0x3b (ACCEL_XOUT_H) & 0x3c (ACCEL_XOUT_L)
  Wire.read(); Wire.read(); // disregard 0x3d (ACCEL_YOUT_H) and 0x3e (ACCEL_YOUT_L)
  accel_z_raw = Wire.read() << 8 | Wire.read();  // 0x3f (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)

  /*
   * Calculate angle to direction of gravitation.
   */
  double phi= atan2(accel_x_raw, -accel_z_raw)* rad2deg; // 0deg is desired position, sign correct for view through camera 
  return phi; 
}
