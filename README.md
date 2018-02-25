# Morse Key USB Keyboard

### What's this?

This repo contains code to convert a physical Morse key into a USB keyboard, via a Teensy 3.2 or Arduino ProMicro. This means you can plug the Morse key into any laptop or computer and use it in exactly the same way as a conventional keyboard, except you type in Morse and the characters appear on your screen.

![morse key usb keyboard](https://github.com/save2love/morse/raw/master/morsekey.png "Morse Key USB Keyboard")

### Features

* Supports English and Russian Morse alphabets
* Supports numbers 0-9
* Supports lowercase and uppercase letters
* Useful and simple menu to change settings
* Enable/disable sound

Unrecognised characters are printed out as a dash, or "-".

Timings between characters and words are currently hardcoded in ms. This isn't hugely accurate since different operators will key at different speeds, but the hardcoded timings can easily be tweaked.

#### Schema and pinout

![morse key usb keyboard schema](https://github.com/save2love/morse/raw/master/schema.png "Morse Key USB Keyboard schema")

| ProMicro pins | Target               |
| ------------- |----------------------|
| PIN 4         | to LED               |
| PIN 7         | to Morse key         |
| PIN 8         | to functional button |
| PIN 9         | to buzzer            |
| GND           | to GNDs              |

### TODO list

* Include timings in settings
