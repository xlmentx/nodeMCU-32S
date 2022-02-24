# 2021SpringTeam2
From MAE/ECE 148 - Introduction to Autonomous Vehicles

## Team Members
Ari Cortes – ECE Senior//
Antoine Laget – CSE Senior (UCSD Extension)\\
Kevin Lam – MAE Senior
Jack Ringelberg – MAE Senior
Teampic team2 sp21.jpg

## Project Overview
In this class, students are tasked with programming a remote control (RC) car to navigate a track autonomously. This is first accomplished by using deep learning to train an artificial intelligence (AI) model with the Donkey Car framework, and then tackled using Robot Operating System (ROS) to implement image processing and lane-following algorithms. In both cases, training and tuning result in a lot of eccentric behavior and crashes, so an emergency stop is implemented to minimize accidents. Currently, a relay controlled by a wireless clicker disables the PCA9685 pulse-width modulation (PWM) board, stopping steering and throttle commands from reaching the servo and motor. This assembly is not ideal because it is bulky, requires a lot of jumper wires, and causes the car to coast to a stop rather than brake. The goal of this project is to replace the relay and PWM board assembly with a single ESP32 wi-fi capable microcontroller. The ESP32 will generate PWM signals to control the servo and motor and will receive emergency stop commands through wi-fi from a user's phone or computer.

Overview Pic Team 2 Proj.png
Figure 1: Old and New Emergency Stop System

### Must Haves
ESP32 generates PWM signals based on commands from Jetson Nano to control the servo and motor.
ESP32 functions as an access point for a phone or computer to connect to.
Website with a red button activates an emergency stop.

### Nice to Haves
Heartbeat and watchdog to shutdown car when wi-fi connection or serial connection with Jetson is lost.
Expand website to include additional functions beyond emergency stop.

### Project Video
### Project Presentation
The project overview presentation can be found here.

## Hardware
Cooler Shot Team 2.png

Figure 2: Team 2 Car

### Mechanical Design
The major components of the mechanical design include the baseplate, camera mount, and Jetson Nano case.

### Baseplate
Starting from a high-contrast image of the car chassis, the baseplate was designed to conform to the existing body shape of the RC car. A central slot allows for easy wire passthrough for cameras and circuitry. A reversible design allows for ease of electrical debugging, and once that's working, simply flipping over the plate protects the electronics from collisions.
SP21 T2 Baseplate.png
Figure 3: Baseplate Design

### Camera Mount
Multiple camera mount design iterations were tested over the course of the quarter. Starting with an adjustable design, once an ideal camera angle was chosen, a sturdy rigid mount was used to provide ample camera protection.
Figure 4a: Final Mount Design
Figure 4b: Adjustable Mount Design

### Jetson Nano Case
Based on a Thingiverse design, mounting holes were added for fixing the case to the baseplate.
SP21 T2 JetsonCase.png
Figure 5: Jetson Nano Case Modified from Thingiverse Design

### Electrical Design
The car's electrical assembly consists of eleven main components:

Jetson Nano – The single board computer (SBC) in charge of controlling the remote control car.
3 Cell LiPo Battery and Alarm– The power source for the car. An alarm is attached to notify the user when the battery charge has depleted.
Power Switch – Switches power to all components on the car except the electronic speed controller (ESC).
DC-DC Converter – Regulates the battery voltage (anywhere from ~10-12V during operation) to a constant 5V.
USB Camera – Camera connected to the Jetson via USB cable to provide a live video feed.
Servo – Steers the car.
Electronic Speed Controller (ESC) and Switch – Controls the DC motor based on commands from the PWM board and provides power to the servo. The ESC can be switched on or off.
Brushless DC Motor – Drives the car's four wheels.
PCA9685 Pusle-Width Modulation (PWM) Board – Receives commands from the Jetson Nano via I2C communication to control the steering and throttle. Generates PWM signals to send to the servo and to the ESC to be then sent to the motor. Also provides power for the status LED light.
Status Light-Emitting Diode (LED) – LED light denoting enabled or disabled state of the car.
Emergency Stop Relay – The relay is connected to a wireless remote which switches 3.3V power from the common (CO) terminal between the normally closed (NC) and normally open (NO) terminals. The disable pin (OE) of the PWM board is connected to the NC terminal, stopping generation of PWM signals when switched and therefore shutting down the steering and throttle. To denote the enabled and disabled conditions, the blue light is connected to the NC terminal of the relay and the red light is connected to the NO terminal.
Team2 WiringDiagram.png
Figure 6: Car Wiring Diagram with Original Hardware

The ESP32 (component 9* in the new diagram) replaces components 9, 10, and 11. The ESP32 communicates with the Jetson Nano via serial communication through a USB cable. Pin 0 and 4 of the ESP32, which are capable of generating PWM signals, are connected to the PWM inputs to the servo and ESC, respectively. The 6V power output from the ESC is connected to the servo. The servo, ESC, and ESP32 are all grounded to the Jetson.
Team 2 ESP32 Wiring Diagram.png
Figure 7: Car Wiring Diagram after ESP32 Implementation

## Software
This Code was written using using VSCode and the PlatformIO extension for use on the ESP32-DevKitC micro controller. Further project details and video demostration can be found here: [ESP32 Web Server Project](https://docs.google.com/document/d/1h9dRktVf6lAae34t0Z2zWiOfpSByz_bvqmT_pmIQF1w/edit#)

### Data Folder
Contains all of the HTML, JavaScripts, and CSS files needed to make the webserver function

### Src Folder
Contains all of the C++ files need to handle comunications between the Jetson Nano, ESP

### ESP32 (LOLIN32) Code
### Website Code
The website server code is divided into 3 parts: HTML, CSS, and JavaScript. The HTML governs the look of the site, CSS handles the layout, and JavaScript handles the button events. In order to push the server data onto the ESP32, you need to store the website code in a folder named data and upload a filesystem image via platformio.

### Websockets and Syncronization
Device syncronization is handled via main.cpp and server code. Whenever a button is pressed on a device, javascript sends a JSON message over serial to the esp32, and the button lights up to indicate it has been pressed. The JSON message is then deserialized and the buttons on the server are updated using the new values. The main.cpp code then sends a websocket "text" over serial to all connected devices notifying them of the change. The "text" messages are then deserialized via javascript on each device and the buttons' CSS are updated.
Website Team 2.png
Figure 8: Website User Interface

### Serial Communication
The Jetson and ESP32 use serial communication through a USB cable. The Jetson sends steering and throttle commands to the ESP32 in the form of a JSON formated message, which is a formatted string that provides a normalized throttle and steering value between -1 and 1. A typical JSON message looks like this:

{"throttle": 0.00, "steering": 0.00}

The ESP32 decodes this message and sends the proper signals to the servo and ESC to achieve the desired normalized steering and throttle values. The ESP32 is also able to send JSONs back to the Jetson, which was initially used to confirm that the command has been received properly. However, new items can be easily appended to the JSON so that the ESP32 can send back commands to the Jetson to change model or ROS parameters, for example allowing the user to make live changes to the ROS color filter:

{"throttle": 0.00, "steering": 0.00, "hue_lower": 30, "hue_upper": 80}

We decided to use JSON format because it is a well documented and a computer science standard. JSON is human readable and easily scalable for further developpement.

### PWM Signal Generation
For the servo PWM signal generated at pin 0, the command ledcWrite was used to generate a 3.3V PWM signal of varying duty cycle based on a normalized steering command between -1 and 1 from the Jetson . For a PWM frequency of 300 Hz, a 33% duty cycle was found to correspond to full left steering and 58% corresponded to full right. Visualization of this signal is provided below:

Servo Signal.png

  Figure 9: Servo Left and Right Steering Signal
It was found that the ESC does not function with a fixed-frequency PWM signal. Instead, the ESC requires a 20 millisecond low period followed by a 1-2 millisecond high (3.3V) pulse. 1 millisecond high corresponds to full reverse rotation of the motor and 2 milliseconds corresponds to full forward rotation. To generate such a signal, the writeMicroseconds command from the ESP32Servo Library was used to modulate the duration of the 3.3V pulse generated at pin 4. The visualization of this signal is shown below:

ESC Signal.png

  Figure 10: Motor Full Reverse and Forward Signal

### Watchdog
The ESP32 has a watchdog which trips whenever it takes longer than 200 milliseconds to receive a new command from the Jetson, which happens when the Jetson freezes or crashes. This watchdog forces the ESP32 into a backup routine where it shuts down the steering and throttle and waits 3 seconds before checking if the Jetson is back online. If the serial connection is restored and new messages are received, the ESP32 will resume normal operation, otherwise it will continue to run the backup routine.


### Jetson Nano Code
On the Jetson Nano, provided image processing and vehicle control nodes output throttle and steering topics. These topics are parsed by our ESPcomms_ROS node and formatted as a JSON string for serial passing to the ESP32.

RQT Graph T2.png

  Figure 11: rqt_graph of ROS Nodes on Jetson Nano

## Quarter Milestones
### Donkey Car Deep Learning Autonomous Laps
### ROS Autonomous Laps
### ESP32 with E-Stop
## Advice and Suggestions for the Future
### Advice
Build to Crash - No matter how carefully you drive, you will probably crash a lot! A lot of collisions occur between cars and are often not your fault. The best way to prepare for this is to ensure your mechanical parts are beefy and electrical components are covered.
Get Driving ASAP - Training a deep learning model at the tent track was a particularly difficult task due to the constantly changing lighting conditions. It was critical to start training early to ensure there was enough time to work out any kinks and develop a robust model. Listen to Professor Silberman and send your parts out to be manufactured in the first week!

### Future Suggestions for ESP32 Use
Expanding ESP32 Website Functionality - Possible functions to add to the website include live calibration of steering and throttle, live color filter adjustment, and live PID controller tuning.
Creating Class for DonkeyCar Implementation - Similarly to how the ESPcomms_ROS node was added to send steering and throttle controls over serial for the ROS framework, an ESP32 class can be added to the DonkeyCar steering and throttle code to send commands to the ESP32 over serial based on inputs from the driver or AI model.
We recommend the WEMOS ESP32 (Rather than LOLIN) board, which goes into flashing mode only when you hold a button, rather than always looking to download code through serial.
We recommend using platform.io for javascript flashing.
Developing a PCB for attaching PWM cables will make things cleaner, consider a flat back for velcro mounting or thru holes for more screw mounting.
Reset ESP32 remotely using the EN pin through Jetson, rather than having to press the button on the board.

## Acknowledgements and References
### Acknowledgements
Team 1 - Thank you for helping us determine the proper PWM signals to send to the motor and servo.
Dominic and Haoru - Thank you for debugging with us throughout the quarter.
Professor Silberman and Professor de Oliveira - Thank you for providing a priceless learning opportunity!

### References
ESP32 PWM with Arduino IDE, RandomNerdTutorials.com, 2021.
ESP32Servo Library, John K. Bennett, 2017.
Thingiverse Jetson Nano Case by ecoiras
