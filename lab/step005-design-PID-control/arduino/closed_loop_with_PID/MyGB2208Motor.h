/**
 * Defines macros and constants for motor T-Motor GB2208.
 */

#ifndef MYGB2208MOTOR_H_
#define MYGB2208MOTOR_H_

const uint8_t pole_pair_number= 14; 
const float pole_pair_factor= (float) pole_pair_number/2.0; // float for convenience
const float Uqmax= 5.0; // V, max voltage to be used in open loop
const float driver_voltage_power_supply= 7.5; // V, voltage of motor driver power supply
const float motor_velocity_limit= 500/360*2*3.1416; // 500deg/s
#endif // MYGB2208MOTOR_H_
