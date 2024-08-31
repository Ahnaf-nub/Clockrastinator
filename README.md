# Pico W Clock with OLED Display and Pomodoro Timer

## Overview

This project features a clock built with a Raspberry Pi Pico W. It utilizes an OLED display to show real-time information, including the current time, date, temperature, and humidity. Additionally, the system includes an LDR sensor to monitor light levels and a pushbutton to activate a Pomodoro timer when light conditions are met.

**Try it out at: https://wokwi.com/projects/405395741064294401**
![image](https://github.com/user-attachments/assets/f2f0e83c-eab3-49fe-861b-642a6931525d)

## Features

- **Current Time and Date:** Displayed on the OLED screen.
- **Temperature and Humidity:** Real-time readings shown on the OLED display.
- **Ambient Light Detection:** LDR sensor assesses light levels in the room.
- **Pomodoro Timer:** Activate the Pomodoro timer by pressing the pushbutton when sufficient light is detected. The remaining time is displayed on the OLED.

## Components

- **Raspberry Pi Pico W**
- **OLED Display**
- **DHT22 Sensor** (for temperature and humidity)
- **LDR Sensor** (Light Dependent Resistor)
- **Pushbutton**
- **DS1307**
- **Buzzer**
