# chicken-coop-arduino
An arduino script for a fully automated chicken coop with temperature/humidity sensors, heatlamp/fan control, and sunset/sunrise door control.

## Tools and Technologies 
Integrated an Arduino microcontroller with various relay modules, buttons, temperature sensors, etc. 
<br>Embedded a heat lamp, a fan, and an automatic door (using a pulley system with a worm drive).
<br>The script was written in C/C++.
## How It's Automated
The temperature/humidity sensors are used to determine when to toggle the fan and heat lamp on or off. They will trigger once the heat and/or temperature reach a certain threshold.
<br>The automatic door will open every day at sunrise, and close every day 45 minutes after sunset. 
 
<br><br><br>![Preview](https://raw.githubusercontent.com/sethpoly/chicken-coop-arduino/master/coop_imgs/coop.jpg )
