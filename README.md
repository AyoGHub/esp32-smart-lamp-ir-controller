# ESP32 Smart Lamp & IR Controller

A Bluetooth/IR-controlled smart lamp and TV control hub

## Features

- Lamp has a 4-mode cycle between:
    - Manual color wheel selection (using built-in joystick)
    - Automatic rainbow (RGB)
    - Pure white
    - Off state
- Dimmable (using a potentiometer)
- Using the app "Serial Bluetooth Terminal", you can wirelessly:
    - Access 9 unique colors (including pure white)
    - Toggle RGB
    - Toggle the lamp on/off
- Using the IR remote, you can wirelessly:
    - Access 8 unique colors
    - Toggle RGB
    - Toggle the lamp on/off
- The IR remote can send signals to the ESP32, which transmits IR codes to a Samsung TV

## Hardware

- ESP32
- Elegoo IR remote
- RGB LED
- Wired Analog Joystick
- Potentiometer
- Jumper Wires
- Breadboard
- IR Transmitter
- IR Receiver
- Flashlight & Cardboard Casings

## System Design, Wiring and Power
  
  - The ESP32 is powered via a USB-C cable from a smart device or power outlet (stable 5 V). It retrieves inputs from a smartphone via Bluetooth, while the IR receiver reads signals from the remote.
  - The receiver, transmitter, potentiometer, RGB LED, and analog joystick are connected to ESP32 GPIO pins via a breadboard and jumper wires
  - The IR receiver and transmitter were positioned to maintain reliable line-of-sight reception from the remote and transmission to the TV
 - A plastic flashlight housing provides structural support and encloses the RGB LED. The wiring is encased in cardboard

## Controls

- Pressing the thumbstick button cycles between modes
- Moving the joystick in manual mode provides color selection
- Twisting the potentiometer adjusts brightness
- After pairing the ESP32 to your phone, in Serial Bluetooth Terminal:
     - Typing and entering a color's first letter (Red --> Purple, + Cyan, Magenta, White) displays that color on the lamp
     - Pressing "W" will cycle between "White" and "Off"
     - "A" triggers RGB
     - Case-insensitive. With macros, you only need to push a button (see [`Serial-Bluetooth-Terminal-with-Macros.jpg`](Serial-Bluetooth-Terminal-with-Macros.jpg)
- The IR remote has directional inputs, selection, volume controls, a "Home" and "Return" button, and can display all of the above colors except white

## Code and Development (Arduino IDE)

- "smartlamp.ino" is the program running on the ESP32, available in [`smartlamp.ino`](smartlamp.ino)
- The program reads the analogue joystick’s X- and Y-axis positions and maps them to a hue on the colour wheel. It also receives IR commands, either translating them into compatible Samsung TV signals or using them to select the displayed color
- It uses ESP32 GPIO outputs to read the potentiometer's position
- AI tools assisted with code generation and troubleshooting; I assembled, integrated, tested, and verified the hardware system.

## Libraries

- BluetoothSerial — enables Bluetooth communication with the ESP32.
- IRremote — receives IR commands and transmits compatible Samsung TV IR signals
         
## Photos/Demo

- All relevant photos and the demo video file are in this repository
- Note: This demo focuses on the automatic rainbow mode and IR remote/TV-control functions.
 The project also supports manual joystick colour selection, potentiometer brightness control and Bluetooth commands


      
      
