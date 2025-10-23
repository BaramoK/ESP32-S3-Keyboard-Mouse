# ESP32-S3 Keyboard/Mouse

This project uses an ESP32-S3 to convert a wired USB keyboard and mouse into a wireless keyboard-mouse combo by hosting them via USB OTG and broadcasting keystrokes over Bluetooth HID.

## Overview

The ESP32-S3 microcontroller is used to enable USB OTG (On-The-Go) functionality, allowing it to act as a host for USB devices. This project specifically focuses on converting a standard wired USB keyboard and mouse into a wireless keyboard-mouse combo that communicates via Bluetooth HID (Human Interface Device).

## Features

- **USB OTG Support**: Allows the ESP32-S3 to interface with USB keyboards/Mouses, supporting auto-repeat, modifier keys, and keyboard LED toggling.
- **Bluetooth HID**: Converts USB keyboard input into Bluetooth signals, making it compatible with various Bluetooth-enabled devices.

## Getting Started

### Usage

1. Connect the wired USB keyboard and mouse to the ESP32-S3 board by placing the USB D- (white) wire to GPIO 19 and the D+ (green) wire to GPIO 20.
2. Flash the firmware onto the ESP32-S3.
3. Pair the ESP32-S3 with your Bluetooth-compatible device.
4. Start typing on the wired USB keyboard, and the input will be transmitted wirelessly via Bluetooth.

## Credits

This project is based on the following libraries:
- [ESP32-keyboard](https://github.com/omeranha/ESP32-keyboard): Forked project, enables converting a USB keyboard to a Bluetooth keyboard.
- [EspUsbHost](https://github.com/tanakamasayuki/EspUsbHost): Provides USB host functionality for the ESP32.
- [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard): Facilitates Bluetooth HID communication.
- [ESP32-BLE-Mouse](https://github.com/T-vK/ESP32-BLE-Keyboard): Facilitates Bluetooth HID communication.
- [ESP32-BLE-Combo](https://github.com/blackketter/ESP32-BLE-Combo): Allows connecting both a keyboard and a mouse.
