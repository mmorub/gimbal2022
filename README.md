# Gimbal Hackathon 2022

## Introduction
In this hackathon, you program a one-axis gimbal controller. 

It is our goal to determine a transfer function for the motor-camera combination, and to design a controller that keeps the camera upright if we turn its handle. The code is written in C for an arduino microcontroller, and we use matlab for visualization, analysis and controller design. 

The hardware is provided in the lab at RUB. While this is not the focus of the hackathon, you can also build the hardware yourself. You could explore how to use a smaller controller and motor driver, for example, or how to run the hardware on batteries.   

TODO: Photo of components

TODO: Video

![sample pic](readme-files/sample.jpg "silly sample pic") [![Everything Is AWESOME](https://img.youtube.com/vi/StTqXEQ2l-Y/0.jpg)](https://www.youtube.com/watch?v=StTqXEQ2l-Y "Everything Is AWESOME") 


In a nutshell, these steps are required:
  * [Understand and test the sensor](#Understand-and-test-the-sensor). We use a gyroscope that transmits its signal to a microcontroller. 
  * [Use the sensor to measure a step-response](Measure-open-loop-step-response). We force the camera to turn by a certain angle (say, $5$ or $10$ degrees) with a motor, and we control this motor by sending commands from the microcontroller to a motor driver. Recording the sensor signal with the microcontroller as the camera turns then yields the step response. 
  * [Find a transfer function that describes the step-response](Identify-a-transfer-function). The transfer function will describe how the signal sent to the motor driver turns into ("is transferred into") the angle by which the camera is actually turned. The transfer function is our system model. 
  * [Design a controller using the transfer function](Design-a-controller). This requires the methods that you learn about in your automatic control courses.  
  * [Test the closed-loop system](Test-the-closed-loop-system). 

### Level of the project
We need to skip a lot of details in order to complete the project in the given time. The explanations given here are supposed to make sense even if you have not completed your first control course. 
Try to understand what the step response and transfer function are good for, try to distinguish the open-loop and closed-loop systems from one another, and try to understand that the case treated here is a disturbance rejection. 
You will also see that it is not the point to find a model that is as precise as possible, but one that is fit for its purpose. 

You will get to know matlab and learn a bit about microcontroller programming for digital control. The microcontroller and sensor board used here are used in many maker projects (in other words, they were never designed for industrial use). Rest assured that everything you learn here about microcontrollers and sensors also applies to industrial their industrial variants.  

# Understand and test the sensor
Hello World.

# Measure an open-loop step response
Hello World.

# Identify a transfer function
Hello World.

# Design a controller
Hello World.

# Test the closed-loop system
Hello World.


