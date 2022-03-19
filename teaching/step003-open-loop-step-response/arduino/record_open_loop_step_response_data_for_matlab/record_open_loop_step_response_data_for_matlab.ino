/* 
 * Integrate gyroscope signal to monitor the angle by which the camera is turned by hand. 
 * Print gyroscope signal and integrated angle to serial monitor for import into matlab. 
 *   
 * For registers like PWR_MGMT_1 of the MPU6050 see
 * Datasheets/Invensense2013-MPU-6050-Register-Map. "MPU_" is prepended 
 * to all registers. 
 */
 
#include "MPU.h"
#include "get_gyro_y.h"
#include <Wire.h>
#include <SimpleFOC.h> 

// Variables needed for polling gyrometers.
const int16_t gyro_y_raw_offset= -82; 
const float delta_t= 0.005; // seconds
int16_t gyro_y_raw;

// BLDC motor & driver instance
// BLDCMotor motor = BLDCMotor(pole pair number);
BLDCMotor motor = BLDCMotor(14);
// BLDCDriver3PWM driver = BLDCDriver3PWM(pwmA, pwmB, pwmC, Enable(optional));
BLDCDriver3PWM driver = BLDCDriver3PWM(11, 10, 9, 6, 5, 3);
const float pole_pair_factor= 14.0/2.0; // float for convenience
const float Uqmax= 5; // V, max voltage to be used in open loop


// Variables needed to ensure approximate realtime in loop().
uint32_t newtime, oldtime, timestep, integrated_jitter, time0;  
uint16_t time_step_counter; // counts time steps
uint16_t responses_counter; // counts step responses

float phi, omega;  // angle and angular velocity of camera
float electrical_angle; 
bool go_clockwise; // used to control direction of step

void loop(){

  const uint16_t num_responses= 5; // record for this many time steps
  const uint16_t num_time_steps= 200;         // in between angular steps
  const float angular_step= 3.1416/180.0*5; // 5 deg in rad
  
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
  gyro_y_raw= get_gyro_y(MPU_ADDRESS); 
  gyro_y_raw= gyro_y_raw- gyro_y_raw_offset;

  // Update states
  omega= gyro_y_raw* gyroRawTo1000dps; 
  phi= phi+ omega* delta_t; // integrates omega
  
  // Output for matlab
  if (responses_counter< num_responses){
    Serial.print(newtime- time0);
    Serial.print(',');
    Serial.print(phi);
    Serial.print(',');
    Serial.println(omega);
  }
  time_step_counter++; 

  // Check if next step is due. 
  if (time_step_counter== num_time_steps && responses_counter< num_responses) {
    time_step_counter= 0; 
    responses_counter++; 
    if (go_clockwise==1){
      electrical_angle= electrical_angle+ angular_step* pole_pair_factor; 
    } else {
      electrical_angle= electrical_angle- angular_step* pole_pair_factor; 
    }
    go_clockwise= !go_clockwise;
    motor.setPhaseVoltage(Uqmax, 0, electrical_angle); // setPhaseVoltage(Uq, Ud, electrical_angle)
  } else if (responses_counter== num_responses){
    motor.disable(); 
  }

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

  /**
   * Motor driver configuration.
   */
  // power supply voltage [V]
  driver.voltage_power_supply = 13.5;
  driver.init();
  // link the motor and the driver
  motor.linkDriver(&driver); // motor instance required, because setPhaseVoltage is

  // set motor constraints and init motor
  motor.voltage_limit = 5;   // [V]
  motor.velocity_limit = 500/360*2*3.1416; // 500deg/s 
  // init motor hardware
  motor.init();

  // setPhaseVoltage(Uq, Ud, electrical_angle
  electrical_angle= 0; // rad
  motor.setPhaseVoltage(Uqmax, 0, electrical_angle);    
  delay(1000); 

  go_clockwise= 1;

  /**
   * Sensor configuration (MPU6050)
   */
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
  Wire.write(MPU_GYRO_CONFIG);  // write 0x10 to MPU_GYRO_CONFIG register
  Wire.write(gyro_sensitivity); // to select +/-1000 deg/s sensitivity
  Wire.endTransmission(true);   // true means arduino let's other devices be master

  // Connect to serial interface; required for Serial.print. 
  Serial.begin(115200);
  while (!Serial) {
    delay(10); 
  }

  // set angle and angular velocity to defined value
  phi= 0.0;   // degrees 
  omega= 0.0; // degrees per second
  
  /**
   * Variables needed to ensure approximate realtime.
   */
  // All times are in microseconds.
  // Do this at the end of setup.
  time_step_counter= 0; 
  responses_counter= 0; 
  newtime= 0;
  timestep= 5000; // 5000 microseconds= 5ms, not constant to compensate jitter
  integrated_jitter = 0; // used to adjust timestep
  oldtime= micros(); 
  time0= oldtime; 
}
