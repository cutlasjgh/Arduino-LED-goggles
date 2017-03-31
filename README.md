## Arduino-LED-goggles
light rings and light strips controlled using Arduino type devices

## Description of files

3 files:

SimpleGoggle.ino -  This works. This was Adafruit's Kaleidescope Eyes welding goggle converted for use with Circuit Playground instead of trinket. I drive a 16 LED Neopixel, so be sure to load up Neopixel and CircuitPlayground libraries, refer to adafruit or the Usage block below.

GooglyEyes.ino  - this is probably straight from Adafruit, supports a Flora and a Flora Gyro Accelerometer for gravity based eyes.

GooglyEyes_CircuitPlayground.ino - essentially GooglyEyes.ino but converted for use with Circuit Playground board instead of Flora. Circuit Playground is a few dollars more but has many more features.  Gyro might not yet be working, but this should be close.

## Usage

Simply load up one of the .ino files into your Arduino.cc ide tool. Note you must load the proper libraries to support your board, be it trinket, flora or CircuitPlayground. Adafruit.com has guides to getting arduino.cc setup to run code on your hardware, follow those guides.
