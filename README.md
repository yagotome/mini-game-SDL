# Sonic, jump!
## Final Project: SDL - Arduino integration

### For Linux users:

To play the game, run:
```
$ make
```

Then, you'll find the executable game in _bin_ folder. Therefore, you could instead run:
```
$ make install
$ cd bin
$ ./'Sonic, jump!'
```

Remember you must have SDL installed:
```
$ sudo apt-get install libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev libsdl2-ttf-dev libsdl2-net-dev libsdl2-gfx-dev
```  

### Turn your Arduino UNO into a USB HID keyboard to play "Sonic, jump!":
Before you start, install the required packages. On Ubuntu and Debain systems, in a terminal run:
```
$ sudo apt-get install dfu-programmer dfu-util
```

Open the Arduino IDE and upload the sketch up_down_control.ino.  
After everything flashes normally, briefly bridge the reset pin with the ground and then:
```
$ sudo dfu-programmer atmega16u2 erase
$ sudo dfu-programmer atmega16u2 flash --debug 1 Arduino-keyboard-0.3.hex
$ sudo dfu-programmer atmega16u2 reset
```

Unplug and plug the arduino. **The Arduino UNO is already being recognized as a keyboard**.

NOTES:
* For this example, the Arduino UNO has two buttons (digital pins 2 and 3), simulating the keyboard up and down buttons.
* The chip **atmega16u2** may vary according to the arduino.
* The Arduino can only be flashed with skectches through the Adruino IDE if the Arduino-usbserial.hex bootloader is active.  
Again, briefly bridge the reset pin with the ground and then:
  ```
  $ sudo dfu-programmer atmega16u2 erase
  $ sudo dfu-programmer atmega16u2 flash --debug 1 Arduino-usbserial-uno.hex
  $ sudo dfu-programmer atmega16u2 reset
  ```
### Developed by:
* [Lucas Alves](https://github.com/lucascomp)
* [Yago Tomé](https://github.com/yagotome)

[Project's video](https://github.com/lucascomp/sonic-jump-arduino-sdl)
