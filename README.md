CastleAritechArduinoRKP for ESP32
=================================

An Aritech Alarm compatible Remote Keypad that makes the Alarms functions and menus
accessible via any desktop or modern mobile phone browser.

Features a WIFI connection to your router, Built in Webserver to present the virtual keypad - websockets to provide two way connection from your phone. Sends Email when an alarm is triggered (gmail compatible). Also has a built in OLED screen.

Tested on several Aritech Alarms - CD34, CD73, CD34 etc.

![animation demo](https://github.com/OzmoOzmo/CastleAritechArduinoESP32/blob/master/HowTo/ArduinoAritechInternetKeypadLoop.gif)

Allows you to remote arm/disarm the panel as well as view logs etc.
Also Emails you when an Alarm happens.

Connection to the Alarm Panel requires connecting via the standard 4 wire Remote Keypad bus.

To compile, place all files in a folder called CastleAritechArduinoESP32
and open the "CastleAritechArduinoESP32.ino" in Arduino IDE.

The circuit required is as follows.

It can be soldered up to yuor requirments and parts available - but here is a suggestion using an Adafruit Proto Shield mounted on an ESP D1 R32 board.
Both available online from about $5 each.

![Wiring Diagram](https://raw.githubusercontent.com/OzmoOzmo/CastleAritechArduinoESP32/master/HowTo/SolderedBoard.png)

![Photo Soldered Up](https://raw.githubusercontent.com/OzmoOzmo/CastleAritechArduinoESP32/master/HowTo/SolderedBoard.jpg)

![schematic](https://raw.githubusercontent.com/OzmoOzmo/CastleAritechArduinoESP32/master/HowTo/Schematic.jpg)



*Note: This is the latest code for an ESP32 - supporting Wifi, Chrome, most phones and Gmail - An Arduino UNO version using Ethernet is also available*
March 2021
