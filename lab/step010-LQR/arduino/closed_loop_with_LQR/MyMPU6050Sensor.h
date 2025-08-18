#ifndef MPU_H_
#define MPU_H_

// Sensor (MPU6050) I2C address and registers
// See Datasheets/Invensense2013-MPU-6050-Register-Map.pdf for information
// on registers; page numbers are page numbers for this document. 
#define MPU_ADDRESS 0x68              // MPU6050 has either address 0x68 or 0x69
#define MPU_PWR_MGMT_1 0x6B           // address required to wake up MPU6050
#define MPU_CONFIG 0x1A               // address used to set gyro digital low pass filter bandwidth, p. 13
#define MPU_GYRO_CONFIG 0x1B          // address used to set gyro range 
#define MPU_GYRO_YOUT_H 0x45          //
const uint8_t gyro_sensitivity= 0x10; // value selects range +/-1000 deg/s, p. 14 
const float gyroRawTo1000dps= 2.0/32.8; // conversion factor for gyro signal, p. 31, not sure why 2 is required

#endif // MPU_H_
