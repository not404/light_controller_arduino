Light Controller for Arduino
============================

Sensors and PWM Controls via JSON over Serial Port on Arduino
-------------------------------------------------------------

<img src="http://www.not404.com/system/images/W1siZiIsIjIwMTMvMDIvMDgvMDZfNTNfMzdfMjkzX05vdDQwNF9BcmR1aW5vX1NjcmVlbkRlbW9fMDIwMS5wbmciXV0/Not404_Arduino_ScreenDemo_0201.png">

<img src="http://www.not404.com/system/images/W1siZiIsIjIwMTMvMDIvMDgvMDZfMzhfMjVfMzI1X05vdDQwNF9BcmR1aW5vX1NjcmVlbkRlbW9fMDJfc2NoZW0ucG5nIl1d/Not404_Arduino_ScreenDemo_02_schem.png">

### Introduction
This project is one component of a larger Aquaponics Controller system that I am working on in my spare time. The overall design is to use a Raspberry Pi as the main controller, connected to "Slave" units based on Arduino technology. The code within this project is to implement such a Slave unit.

This code is intended for the Arduino Uno. Other products in the Arduino line, or compatible products may work right out of the box, or may work with minimal code modification.

### For More Information
See [my blog series](http://www.not404.com/blog/posts/using-android-to-control-an-arduino-part-1) for more background information on this project. Thanks!

### Dependencies
This project uses an external library to handle the parsing of JSON messages - [aJSON](https://github.com/interactive-matter/aJson). 

I have no intention of rolling my own JSON parsing library at this time. Yes, I know the current messages are simplistic and hand-parseable; but I prefer to future-proof my code with something more flexible and robust. The memory penalty of incorporating this library doesn't bother me at this stage of the project.

### Current Capabilities of this Circuit/Sketch Combination
1. Ambient Light Sensor. In production, I will enhance this to record light levels, water temperature, and air temperature.
2. PWM "Light Dimmer" for LED and/or Power Transistor to control larger DC circuits. In the blog, I show how to control a 12-Volt DC LED Light Strip; in production I will be controlling 12-Volt Water Pumps.
3. A pair of Micro-Switches to increase or decrease the LED's brightness (intensity). In production, I will use similar switches to calibrate or fine-tune the Water Pumps' speed.
4. A "command and control" set, based on JSON.  Current commands:

    1. **debug** - toggles debugging output to help troubleshoot connectivity issues.

    2. **getStatus** - return the current Sensor Reading, light switch state, ambient threshold level, and PWM intensity.

    3. **lightSwitch** - toggles the Light Switch state from Off, On, or Auto. Auto mode turns the light on if Ambient Light level readings fall below the Ambient Threshold Level setting. Easy to test by covering the photo resistor.

    4. **setAmbientThreshold** - Sets the minimum amount of light required to keep the PWM control "off" when the **lightSwitch** setting is in Auto mode.

    5. **setIntensity** - Sets the PWM control value. Higher values lead to brighter LED values.

