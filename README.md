Arduino sketches and libraries to allow it to act as a virtual Keypad for a DSC Powerseries 18XX alarm panel:
-------------------------------------------------------------------------------------------------------------
 - Original DSC Keybus reverse engineering thread http://www.avrfreaks.net/comment/864193
 - Arduino Libraries from **stagf15's** [repositories] (https://github.com/stagf15)
 - Original "how to connect" shematic from **emcniece's** [Arduino-Keybus repository] (https://github.com/emcniece/Arduino-Keybus)

### How to connect Arduino to DSC Panel

![alt text][logo]

[logo]: https://raw.githubusercontent.com/sjlouw/dsc-alarm-arduino/master/schematic.png "Connect DSC Keybus to Arduino"

### How to use
1. Clone this repository
2. Zip the three directories in three separate zip files (DSCPanel.zip, TextBuffer.zip and Time.zip)
3. Install it as Arduino libraries
4. From within the Arduino IDE, choose File --> Examples --> DSCPanel
5. Upload to the Arduino and watch the Serial Monitor as Panel and Keypad data stream in

`readserial.py` can be used if Arduino is connected via USB to Raspberry Pi, to read the serial data from Arduino.
