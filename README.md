# Gimbal Hackathon 2022

## Introduction
In this hackathon, you program a one-axis gimbal controller. 

It is our goal to determine a transfer function for the motor-camera combination, and to design a controller that keeps the camera upright if we turn its handle. The code is written in C for an arduino microcontroller, and we use matlab for visualization, analysis and controller design. 

The hardware is provided in the lab at RUB. While this is not the focus of the hackathon, you can also build the hardware yourself. You could explore how to use a smaller controller and motor driver, how to run the hardware on batteries, or 3d-print a nicer stand, for example. 


| ![components of the gimbal](readme-files/componentsOfTheGimbal.jpg "Components of the gimbal") |  ![gimbal demo](readme-files/gimbal-kachel.gif) |
|:--:|:--:|
| *Components of the gimbal* [(*large picture*)](readme-files/componentsOfTheGimbalLarge.jpg) | *Gimbal demo* [(*better video*)](https://youtu.be/X2nZriD9WRU) |


## Steps
In a nutshell, these steps are required:
  * [**Understand and test the sensor**](#understand-and-test-the-sensor). We use a gyroscope that transmits its signal to a microcontroller. 
  * [**Measure an open-loop step-response**](#measure-an-open-loop-step-response) with the sensor. We force the camera to turn by a certain angle (say, 5 or 10 degrees) with a motor, and we control this motor by sending commands from the microcontroller to a motor driver. Recording the sensor signal with the microcontroller as the camera turns then yields the step response. 
  * [**Find a transfer function that describes the step-response**](#identify-a-transfer-function). The transfer function will describe how the signal sent to the motor driver turns into ("is transferred into") the angle by which the camera is actually turned. The transfer function is our system model. 
  * [**Design a controller using the transfer function**](#design-a-controller). This requires the methods that you learn about in your automatic control courses.  
  * [**Test the closed-loop system**](#test-the-closed-loop-system). 
  * [**Hack on!**](#hack-on)

### Level of the project
We need to skip a lot of details in order to complete the project in the given time. The explanations given here are supposed to make sense even if you have not completed your first control course. 
Try to understand what the step response and transfer function are good for, try to distinguish the open-loop and closed-loop systems from one another, and try to understand that the case treated here is a disturbance rejection. 
You will also see that it is not the point to find a model that is as precise as possible, but one that is fit for its purpose. 

You will get to know matlab and learn a bit about microcontroller programming for digital control. The microcontroller and sensor board used here are used in many maker projects (in other words, they were never designed for industrial use). Rest assured that everything you learn here about microcontrollers and sensors also applies to their industrial variants.  

# Understand and test the sensor 
**All required code is in lab/step001-test-gyroscope/ for this step**. 

We use a gyroscope to measure the angular velocity by which the camera is turned. The particular gyroscope used here is a [MEMS Gyroscope](https://en.wikipedia.org/wiki/Vibrating_structure_gyroscope#MEMS_gyroscopes), which was [originally designed for use in devices like smartphones](https://invensense.tdk.com/products/motion-tracking/6-axis/mpu-6050/). In our case the sensor comes on a little prototyping board shown in the picture of the [components](#introduction).

## Record gyroscope data with the arduino 
**All required code is in lab/step001-test-gyroscope/arduino**.

Let's start by turning the camera on the gimbal by hand and measuring the angle with the gyroscope. You need to use the Arduino [IDE](https://en.wikipedia.org/wiki/Integrated_development_environment) and its serial monitor for this. You also need Matlab in the next step. Everything you need is installed on the PCs in the lab. If you are using your own PC or laptop, check the brief description of the [prerequisites](#prerequisites).

You **do not need to plug in the separate power supply** for these steps yet. The separate power supply is for the motor, which is still turned off here:

 * **Open the arduino sketch** in `gimbal2022/lab/step001-test-gyroscope/arduino/record_gyroscope_data_for_matlab/` in your Arduino IDE. **Connect the arduino** with the USB cable and check if you can compile and upload your code. **Do not plug in the separate power supply**. 

 * You should always **have an empty arduino sketch open** that you can upload immediately to reset the arduino in case anything goes wrong. You can create an empty sketch by clicking File->New in the Arduino IDE. **Practise uploading an empty sketch** just to be sure. 

 * Now **open the arduino serial monitor**. Upload the sketch again and verify the serial monitor is printing lines for the duration set in the sketch. Try to find this duration by reading the sketch. We print for a few seconds and then stop, just so the serial monitor will show a finite set of data for use in matlab. 

 * Upload the sketch and, once it starts running, **turn the camera in its case by hand**. Try to turn it by a defined angle (such as 90 degrees) and observe that the numbers shown in the serial monitor change as you move the camera. You will only see the numbers run across the screen without being able to make sense out of them at this point. We will record them for use in matlab in the next step. 

 Once you completed the steps above, you are ready to **record data for use in matlab**:

  * Upload an empty arduino sketch.
  * Clear the serial monitor.
  * Upload our arduino sketch, wait for the Arduino IDE to display "Upload completed" and the serial monitor to start printing, then turn the camera by a defined angle. For example, turn it 90 degrees clockwise and 90 degrees back. 
  * Select the output in the serial monitor (try double clicking and using CTRL-A to select all) and paste it into your favourite text editor.
  * Save to a text file. It is convenient to save it to `gimbal2022/lab/step001-test-gyroscope/arduino/record_gyroscope_data_for_matlab/data_for_matlab.txt`, because this filename is accessed by the matlab script used below. 

## Analyse recorded data in matlab 
**All required code is in lab/step001-test-gyroscope/matlab**.

Start matlab and open the script `check_gyroscope_data.mlx`. The script loads the data from the text file you just created. You will have to adjust the script so it finds your data file. If you have not created a data text file in the previous step, the script may use one provided on the repo. Make sure you work with your own data! 

All steps taken by the matlab script are explained in the script itself. Make sure you understand the following points:

* The gyroscope measures **angular velocity**. Because we want to control the angle of the camera, we need to **integrate the angular velocity** to determine the angle. 
* Because we measure the angular velocity at **discrete points in time**, the integration is approximated by a summation. The arduino is programmed to take a measurement every 5 microseconds. This is the sampling time, which is called `delta_t` in the code. 
* The arduino carries out the summation (the approximate integration) in these lines, where `phi` and `omega` refer to the angle and the angular velocity, respectively:
(We will treat `gyro_y_raw_offset` below.)
```
  // measure phi and integrate omega with the arduino

  // Measure angular velocity omega with gyrometer, 
  // subtract offset.
  gyro_y_raw= get_gyro_y(MPU_ADDRESS); 
  gyro_y= gyro_y_raw- gyro_y_raw_offset;

  // Convert angular velocity to degrees per second (dps),
  // integrate to angle phi. 
  omega= gyro_y* gyroRawTo1000dps; 
  phi= phi+ omega* delta_t; // integrates omega
```
* The arduino prints the angle `phi` and angular velocity `omega` to the serial monitor. This way, they are both available after reading the data file in matlab. For the sake of testing, we also calculate `phi` independently in matlab by integrating `omega` recorded by the arduino in matlab. This happens in these lines in the matlab code:
```
  # integrate omega in matlab

  delta_t= 0.005; % seconds
  phimatlab= zeros(1, length(tdata)); 
  for i=2:length(tdata)
      phimatlab(i)= phimatlab(i-1)+ delta_t* omegadata(i);
  end
  figure();
  plot(tdata, phimatlab); 
  title("phi, integrated from omega in matlab");
  xlabel("t in seconds");
```
The output of the matlab script is shown [here](https://raw.githack.com/mmorub/gimbal2022/main/lab/step001-test-gyroscope/matlab/html/check_gyroscope_data.html) for a turn by 90 degrees in one direction and 90 degrees back. 

## Calibrate and repeat

Gyroscopes suffer from a constant offset. We need to determine this offset and subtract it everytime we take a measurement with the gyroscope. This happens in the line 
```
  gyro_y= gyro_y_raw- gyro_y_raw_offset;
```
where the offset is set in the header of the arduino sketch in the line
```
  // Variables needed for polling gyrometers.
  const float gyro_y_raw_offset= -82.768; 
```
The offset was determined for one of the sensors used in the lab. It needs to be adjusted for the particular sensor you are using. Carry out these steps to do so:
* Clear the serial monitor.
* Run the arduino sketch in `lab/step001-test-gyroscope/arduino/record_gyroscope_data_for_matlab/` again, but **do not move the camera this time**.
* Copy and paste the result in the arduino serial monitor to a text editor and **store it in a new file**. It is convenient to name this file `gimbal2022/lab/step001-test-gyroscope/arduino/record_gyroscope_data_for_matlab/calibration_data_for_matlab.txt`, because this filename is used in the matlab script.
* **Run the matlab script** `check_gyroscope_offset.mlx` to determine the offset.
The result of the matlab script should look like this [output](https://raw.githack.com/mmorub/gimbal2022/main/lab/step001-test-gyroscope/matlab/html/check_gyroscope_offset.html). Use this sample output and the sample data files only for debugging your own script.  
* Set the offset in `check_gyroscope_data.mlx`, and run the test again. Check again if the output rembles this [output](https://raw.githack.com/mmorub/gimbal2022/main/lab/step001-test-gyroscope/matlab/html/check_gyroscope_data.html). Again, use this output only for debugging and make sure you complete your own run.

[**Go back to the overview**](#steps) or continue with the next step. 

<!---
<span style="color: red"> TODO Reduce existing script just so it does not integrate the angular velocity. </span>

<span style="color: red"> TODO Use pretty phi, t and so on in the live scripts. </span>
-->

# Measure an open-loop step response

## Understand the step response

**All required code is in lab/step002-open-loop-step-response/arduino/**.

<!--
<span style="color: red"> TODO: Briefly describe the motor and the driver in this text block:</span>
-->

We use the motor for the first time in this step. When we turn on the motor, it will lock into a random position. The motor is turned on in the setup-function in the arduino code. The first few lines prepare a `driver` and a `motor` instance. Details are not important at this point, but you can see that the microcontroller needs to know the supply voltage provided to the driver, and we set a voltage and a velocity limit. Note that the values of `driver_voltage_power_supply`, `Uqmax` and `motor_velocity_limit` are set in the file [MyGB2208Motor.h](/lab/step002-open-loop-step-response/arduino/record_open_loop_step_response_data_for_matlab/MyGB2208Motor.h):
```
  driver.voltage_power_supply = driver_voltage_power_supply; // set to e.g. 
                                               // 7.5V in MyGB2208Motor.h 
  driver.init();
  motor.linkDriver(&driver);                   // link the motor and the driver

  // set motor constraints and initialize motor
  motor.voltage_limit = Uqmax;                 // set to e.g. 5V in MyGB2208Motor.h 
  motor.velocity_limit = motor_velocity_limit; // set to e.g. 500deg/s in MyGB2208Motor.h  
  motor.init();

  electrical_angle= 0; // rad
  motor.setPhaseVoltage(Uqmax, 0, electrical_angle); // syntax is 
                                               // setPhaseVoltage(Uq, Ud, electrical_angle)
  delay(1000);                                 // wait 1s
```
(from [setup() in record_open_loop_step_response_data_for_matlab.ino](/lab/step002-open-loop-step-response/arduino/record_open_loop_step_response_data_for_matlab/record_open_loop_step_response_data_for_matlab.ino))

The last lines in the code snippet above set the motor angle to zero degrees. The command `delay(1000);` tells the microcontroller to pause for a second (1000 microseconds) before continuing with the remainder of the `setup()` function. 

<!--
<span style="color: red"> TODO: Add animated gif/video of locking to illustrate this text block:</span>
-->

Because we run the code with the motor and camera in arbitrary position to start with, the code locks the motor and camera in an unknown angle. We call this unknown position '0 degrees'. We then use the microcontroller and motor driver to turn the motor by a 5 degrees and back by -5 degrees to generate our step responses. The number of step responses to be recorded is set at the beginning of the `loop()` function in [record_open_loop_step_response_data_for_matlab.ino](/lab/step002-open-loop-step-response/arduino/record_open_loop_step_response_data_for_matlab/record_open_loop_step_response_data_for_matlab.ino):
```
  const uint16_t num_responses= 5;          // record for this many time steps
  const uint16_t num_time_steps= 200;       // time steps in between angular steps
  const float angular_step= 3.1416/180.0*5; // 5 deg in rad
```
The variable `num_time_steps` defines the number of time steps in between the step signals sent to the motor (200* 5 microseconds= 1 second here). The variable `angular_step` defines the input step signal. We will see that an input signal that corresponds to 5 degrees does not exactly result in a physical turn by 5 degrees. The deviation of the physical angle from the commanded angle will be captured by our transfer function.  

<!--
<span style="color: red"> TODO: animated gif of the 5 step responses to illustrate the text block above</span>
-->

## Record the step response with the arduino
**All required code is in lab/step002-open-loop-step-response/arduino/**.

The resulting motion of the rotor and camera is the desired step response. We will record it with the serial monitor, and then use it to find a transfer function that fits this data ("identify a transfer function") in matlab in the next step. Carry out these steps to record the step responses:
 * Open an **empty arduino sketch**. Connect the arduino with the USB cable and upload the empty sketch. Keep the empty sketch, just so you can quickly upload it if something goes wrong.
 * Make sure the external power supply for the motor driver is set to 7.5V. **Plug in the power supply**.
 * **Open the arduino sketch** in [record_open_loop_step_response_data_for_matlab.ino](lab/step002-open-loop-step-response/arduino/record_open_loop_step_response_data_for_matlab/record_open_loop_step_response_data_for_matlab.ino) (remember you should use a local clone of this repository for convenience). Upload it, wait for the upload to complete, observe how the motor is locked, and observe the five step responses.
 * **Upload the empty arduino sketch**.

 Having set up the hard- and software, you can now record the step responses:
 * **Open and clear the serial monitor**.
 * **Upload the sketch** [record_open_loop_step_response_data_for_matlab.ino](lab/step002-open-loop-step-response/arduino/record_open_loop_step_response_data_for_matlab/record_open_loop_step_response_data_for_matlab.ino) again and record observe how the step responses are recorded by the serial monitor. 
 * **Upload the empty arduino sketch** and **unplug the power supply**.  
 * **Copy and paste the output from the serial monitor to a text file**. It is convenient to store the text file in `lab/step002-open-loop-step-response/arduino/record_open_loop_step_response_data_for_matlab` and to call it `data_for_matlab.txt`, because this path and filename are used in the matlab scripts explained below. If you do not store your own data, you may be using data from the repository that does not fit your motor-camera combination well. So make sure you use your own data!

## Visualize the step response in matlab
**All required code is in lab/step002-open-loop-step-response/matlab/**.

Start matlab and open the script `plotStepResponses.mlx`. The script reads the data file you just recorded (set the variable `filename` to point to your file). 

All steps are explained in the script. Your result should like this [output](https://raw.githack.com/mmorub/gimbal2022/main/lab/step002-open-loop-step-response/matlab/html/plotStepResponses.html). Try to use your own data, however, and only use the copy of the ouput provided here for checking and debugging your own result. 

[**Go back to the overview**](#steps) or continue with the next step. 

# Identify a transfer function
**Code for this section is in lab/step003-identify-transfer-function/matlab/**.

<span style="color: red"> To be done. </span>

All steps are explained in the matlab script identifyTransferFunctionContinuousTime.mlx. Your result should look like this [output](https://raw.githack.com/mmorub/gimbal2022/main/lab/step003-identify-transfer-function/matlab/html/identifyTransferFunctionContinuousTime.html). Make sure to use your own data, and only use the reference output to check and debug your own code and result. 

[**Go back to the overview**](#steps) or continue with the next step. 

# Design a controller
<span style="color: red"> To be done. </span>

[**Go back to the overview**](#steps) or continue with the next step. 

# Test the closed-loop system
<span style="color: red"> To be done. </span>

[**Go back to the overview**](#steps) or continue with the next step. 

# Hack on!
Up to this point, the hackathon has been more like a lab. If you make it here, feel free to write your own code that combines all the steps described so far. You could, for example, have your own code first run the gyroscope calibration, then determine the direction of gravity, align the camera, and keep it aligned until the gimbal is turned off. 

Maybe you prefer to improve the gimbal stand instead or construct your own multi-axis gimbal starting from here. Let us know if you want to contribute!

[**Go back to the overview.**](#steps)

# Prerequisites

## Matlab
Matlab runs on Windows, macOS and Linux and an [academic license](https://www.it-services.ruhr-uni-bochum.de/services/software/matlab_simulink.html) is available to RUB students. Make sure to select the Control Systems Toolbox during the installation.

## Arduino IDE
The software we use to program the microcontroller, the Arduino IDE, is also available for Windows, macOS and Linux. It is [free](https://www.arduino.cc/en/software) and open source software. If you want to use your own PC or laptop to connect and program the arduino microcontroller, you have to do so at your own risk. 
During the hackathon you can use our lab PCs, of course. 

<!--
<span style="color: red"> TODO List libraries that need to be installed in the Arduino IDE. </span>
-->

## Github 
It is convenient to read this documentation on github with your favourite browser. However, you should checkout a local copy of the code to the PC or laptop you are using. This step will be explained in the lab. 
