/* 
 * Integrate gyroscope signal to monitor the angle by which the camera is turned by hand. 
 * Print gyroscope signal and integrated angle to serial monitor for import into matlab. 
 *   
 * For registers like PWR_MGMT_1 of the MPU6050 see
 * Datasheets/Invensense2013-MPU-6050-Register-Map. "MPU_" is prepended 
 * to all registers. 
 */
 
// Sensor (MPU6050) I2C address and registers
// See Datasheets/Invensense2013-MPU-6050-Register-Map.pdf for information
// on registers; page numbers are page numbers for this document. 
#define MPU_ADDRESS 0x68      // MPU6050 has either address 0x68 or 0x69
#define MPU_PWR_MGMT_1 0x6B   // required to wake up MPU6050
#define MPU_CONFIG 0x1A       // used to set gyro digital low pass filter bandwidth, p. 13
#define MPU_GYRO_CONFIG 0x1B  // used to set gyro range to +/-1000 deg/s, p. 14 

#include <Wire.h>

// Variables needed for polling gyrometers.
const int16_t gyro_y_raw_offset= -82; 
const float gyroRawTo1000dps= 1000.0/32767.0; 
const float delta_t= 0.005; // seconds
int16_t gyro_y_raw;

// Variables needed to ensure approximate realtime in loop().
uint32_t newtime, oldtime, timestep, integrated_jitter, time0;  
uint16_t time_step_counter; // counts time steps

float phi, omega; 

void loop(){

  const uint16_t num_time_steps= 5*200; // record for this many time steps
  
  /**
   * Needed to ensure approximate realtime, do not alter.
   * Wait until current timestep is over.
   */
  newtime= micros();
  while (newtime < oldtime + timestep){
    newtime= micros(); 
  }
  /* 
   * end real time control 
   */

  // Measure omega with gyrometer, 
  // integrate phi. 
  Wire.beginTransmission(MPU_ADDRESS);
  Wire.write(0x45);  // start reading at register 0x45 = GYROYOUT_H
  Wire.endTransmission(false); // send all bytes, false means arduino remains to be master
  Wire.requestFrom(MPU_ADDRESS, 2, true);  // request a total of 2 registers = 2 bytes
  gyro_y_raw = Wire.read() << 8 | Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
  gyro_y_raw= gyro_y_raw- gyro_y_raw_offset;

  // Update states
  omega= gyro_y_raw* gyroRawTo1000dps; 
  phi= phi+ omega* delta_t; // integrates omega
  
  // Output for matlab
  if (time_step_counter< num_time_steps){
    Serial.print(newtime- time0);
    Serial.print(',');
    Serial.print(phi);
    Serial.print(',');
    Serial.println(omega);
  }
  time_step_counter++; 
     
  /**
   * Needed for realtime control, do not alter
   */
  // Integrate sampling time error, recall newtime-oldtime should ideally be 5000,
  // but may be a little longer, see next comment.
  integrated_jitter = integrated_jitter- 5000 + newtime - oldtime;

  // Reduce current sampling time by one step (4e-6 seconds) 
  // if jitter is > 0. Recall jitter < 0 cannot occur,
  // since while-loop used above to delay next poll may only take a little too long,
  // but not too short a time. 
  if (integrated_jitter> 0) {
    timestep= 4996;
  } else {
    timestep= 5000;
  }
  oldtime= newtime;
  /* 
   * end real time control  
   */
}

void setup() {
  
  // Connect to sensor (MPU6050), 
  // turn off low pass filter, set resolution to +/-1000 deg/s.  
  Wire.begin(); // start i2c
  Wire.beginTransmission(MPU_ADDRESS); // connect to MPU6050
  Wire.write(MPU_PWR_MGMT_1); // write zero to PWR_MGMT_1 register 
  Wire.write(0);              // to wake up MPU6050
  Wire.endTransmission(false); // send all bytes, false means arduino reserves to be master 

  // connect again and select low pass filter bandwidth and gyro sensitivity
  Wire.beginTransmission(MPU_ADDRESS); 
  Wire.write(MPU_CONFIG);     // write zero to MPU_CONFIG register
  Wire.write(0);              // to select maximum low pass filter bandwidth

  // set gyro resolution 
  Wire.write(MPU_GYRO_CONFIG);// write 0x10 to MPU_GYRO_CONFIG register
  Wire.write(0x10);           // to select +/-1000 deg/s sensitivity
  Wire.endTransmission(true); // true means arduino let's other devices be master

  // Connect to serial interface; required for Serial.print. 
  Serial.begin(115200);
  while (!Serial) {
    delay(10); 
  }

  // set angle and angular velocity to defined value
  phi= 0.0;   // degrees 
  omega= 0.0; // degrees per second
  
  // Variables needed to ensure approximate realtime.
  // All times are in microseconds.
  // Do this at the end of setup.
  time_step_counter= 0; 
  newtime= 0;
  timestep= 5000; // 5000 microseconds= 5ms, not constant to compensate jitter
  integrated_jitter = 0; // used to adjust timestep
  oldtime= micros(); 
  time0= oldtime; 
}
