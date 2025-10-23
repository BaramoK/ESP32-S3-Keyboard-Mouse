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


---

# Clavier/Souris ESP32-S3

Ce projet utilise un ESP32-S3 pour transformer un clavier USB filaire et une souris en un clavier-souris sans fil en l'hébergeant via USB OTG et en diffusant les touches via Bluetooth HID.

## Aperçu

Le microcontrôleur ESP32-S3 est utilisé pour activer la fonctionnalité USB OTG (On-The-Go), ce qui lui permet d'agir comme hôte pour les périphériques USB. Ce projet se concentre spécifiquement sur la conversion d'un clavier et une souris USB filaire standard en un clavier-souris sans fil qui communique via Bluetooth HID (Human Interface Device).

## Fonctionnalités

- **Support USB OTG**: Permet à l'ESP32-S3 d'interfacer avec les claviers USB, supportant la répétition automatique des touches, les touches modificatrices et le basculement des LEDs du clavier.
- **Bluetooth HID**: Transforme l'entrée du clavier USB en signaux Bluetooth, le rendant compatible avec divers appareils équipés de Bluetooth.

## Prise en main

### Utilisation

1. Connectez le clavier et une sours USB filaire à la carte ESP32-S3 en plaçant le D- USB (blanc) dans la broche GPIO 19 et le D+ USB (vert) dans la broche GPIO 20.
2. Téléchargez le firmware sur l'ESP32-S3.
3. Appairez l'ESP32-S3 avec votre appareil compatible Bluetooth.
4. Commencez à taper sur le clavier USB filaire, et l'entrée sera transmise sans fil via Bluetooth.

## Crédits

Ce projet est basé sur les bibliothèques suivantes:
- [ESP32-keyboard](https://github.com/omeranha/ESP32-keyboard): Projet Forké, permet de transformer un clavier usb en clavier bleutooth
- [EspUsbHost](https://github.com/tanakamasayuki/EspUsbHost): Fournit la fonctionnalité d'hôte USB pour l'ESP32.
- [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard): Facilite la communication Bluetooth HID 
- [ESP32-BLE-Mouse](https://github.com/T-vK/ESP32-BLE-Keyboard): Facilite la communication Bluetooth HID 
- [ESP32-BLE-Combo](https://github.com/blackketter/ESP32-BLE-Combo): Permet de connecter un clavier et une souris
