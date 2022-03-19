/* 
 * Control with observer for output omega and state feedback designed by pole placement.
 * Carry out repeated steps in r and print them to the serial monitor for use in matlab. 
 * Set step size with input_step. 
 *   
 * For registers like PWR_MGMT_1 of the MPU6050 see
 * Datasheets/Invensense2013-MPU-6050-Register-Map. "MPU_" is prepended 
 * to all registers. 
 */
 
// Sensor (MPU6050) I2C address and registers
// See Datasheets/Invensense2013-MPU-6050-Register-Map.pdf for information
// on registers; page numbers are page numbers for this document. 
#define MPU_ADDRESS 0x68      // MPU6050 has either address 0x68 or 0x69
#define MPU_PWR_MGMT_1 0x6B   // address required to wake up MPU6050
#define MPU_CONFIG 0x1A       // address used to set gyro digital low pass filter bandwidth, p. 13
#define MPU_GYRO_CONFIG 0x1B  // address used to set gyro range to +/-1000 deg/s, p. 14 
const uint8_t gyro_sensitivity= 0x10; // value selects range +/-1000 deg/s, p. 14 
const float gyroRawTo1000dps= 2.0/32.8; // conversion factor for gyro signal, p. 31, not sure why 2 is required

#include <Wire.h>
#include <SimpleFOC.h>

// Variables needed for polling gyrometers.
const int16_t gyro_y_raw_offset= -82; 
const float delta_t= 0.005; // seconds
int16_t gyro_y_raw;

// Variables for motor control.
const float num_pole_pairs= 14.0; // float for convenience, rem. electrical_angle= mechanical_angle* num_pole_pairs;
const float deg_to_rad= 3.1416/180.0; 
const float Uqmax= 5; // V, max voltage to be used in open loop
BLDCMotor motor = BLDCMotor(14); // BLDC motor & driver instance, parameter= pole pair number
BLDCDriver3PWM driver = BLDCDriver3PWM(11, 10, 9, 6, 5, 3); // BLDCDriver3PWM(pwmA, pwmB, pwmC, Enable(optional));
float electr_angle;  

// Variables needed to ensure approximate realtime in loop().
uint32_t newtime, oldtime, timestep, integrated_jitter, time0;  
uint16_t time_step_counter; // counts time steps
uint16_t input_step_counter; // counts steps applied to input


// System variables, inputs, etc.
float r, phi, omega, w; 

void loop(){
  // Feedback matrix (a row vector here) from matlab
  //const float kbar[1][3]= {-0.179019537120124, 0.011257210393251, -0.063682519707040}; // cont time poles at -51, -50, -53 (last entry belongs to integrator)
  const float kbar[1][3]= {1.473058724086584, 0.021489161153940, -0.354896359596025}; // cont time poles at -101, -100, -102 
  float y_measured, u; 

  // parameters for the input steps
  const uint16_t num_time_steps= 200; // wait this many timesteps before next step in r
  const float input_step= 5; // deg
  const uint16_t num_input_steps= 6; 

  // check if it is time for step in r
  if (time_step_counter== num_time_steps){
    r= r+ input_step;
    input_step_counter++; 
  } else if (time_step_counter== 2* num_time_steps){
    r= r- input_step;
    input_step_counter++; 
    time_step_counter= 0; 
  }
  time_step_counter++; 
  
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
  w= w+ r- phi; // integrates error e= r- phi
  u= -kbar[0][0]* phi- kbar[0][1]* omega- kbar[0][2]* w;
  electr_angle= u* deg_to_rad* num_pole_pairs; 
  motor.setPhaseVoltage(Uqmax, 0, electr_angle);    
  
  // Output for matlab
  if (input_step_counter>0 && input_step_counter<= num_input_steps){
    Serial.print(newtime- time0);
    Serial.print(',');
    Serial.print(phi);
    Serial.print(',');
    Serial.print(omega);
    Serial.print(',');
    Serial.print(w);
    Serial.print(',');
    Serial.println(u);
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
  Wire.write(MPU_GYRO_CONFIG);  // write to MPU_GYRO_CONFIG register
  Wire.write(gyro_sensitivity); // to select +/-1000 deg/s sensitivity
  Wire.endTransmission(true);   // true means arduino let's other devices be master

  // Connect to serial interface; required for Serial.print. 
  Serial.begin(115200);
  while (!Serial) {
    delay(10); 
  }
  
  // Configure motor driver.
  // Driver needs to know motor power supply voltage. 
  // Some constraints are also set.
  driver.voltage_power_supply = 13.5; // in V
  driver.init();
  motor.linkDriver(&driver); // link motor and driver
  motor.voltage_limit = 5;   // voltage constraint for safety
  motor.velocity_limit = 1500/360*2*3.1416; // limit angular velocity to 1500deg/s 
  motor.init();
   
  // Initial conditions and initial reference value for angle phi 
  r= 0.0;     // degrees
  phi= 0.0;   // degrees 
  omega= 0.0; // degrees per second
  w= 0.0;     // degrees
  
  // Variables needed to ensure approximate realtime.
  // All times are in microseconds.
  // Do this at the end of setup.
  time_step_counter= 0; 
  input_step_counter= 0; 
  newtime= 0;
  timestep= 5000; // 5000 microseconds= 5ms, not constant to compensate jitter
  integrated_jitter = 0; // used to adjust timestep
  oldtime= micros(); 
  time0= oldtime; 
}
