# ESP32-S3 Keyboard

This project uses an ESP32-S3 to turn a wired USB keyboard into a wireless one by hosting it over USB OTG and broadcasting keystrokes via Bluetooth HID.

## Overview

The ESP32-S3 microcontroller is leveraged to enable USB OTG (On-The-Go) functionality, allowing it to act as a host for USB devices. This project specifically focuses on converting a standard wired USB keyboard into a wireless keyboard that communicates via Bluetooth HID (Human Interface Device).

## Features

- **USB OTG Support**: Enables the ESP32-S3 to interface with USB keyboards, supporting auto-repeat key presses, modifier keys and toggling keyboard LEDs.
- **Bluetooth HID**: Transforms the USB keyboard input into Bluetooth signals, making it compatible with various Bluetooth-enabled devices.

## Getting Started

### Usage

1. Connect the wired USB keyboard to the ESP32-S3 board by placing the USB D- (white) into GPIO pin 19 and USB D+ (green) into GPIO pin 20.
2. Upload the firmware to the ESP32-S3.
3. Pair the ESP32-S3 with your Bluetooth-enabled device.
4. Start typing on the wired USB keyboard, and the input will be transmitted wirelessly via Bluetooth.

## Credits

This project is built upon the following libraries:

- [EspUsbHost](https://github.com/tanakamasayuki/EspUsbHost): Provides USB host functionality for the ESP32.
- [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard): Facilitates Bluetooth HID communication for the ESP32.

Special thanks to the authors of the EspUsbHost and ESP32-BLE-Keyboard libraries.

---

# Clavier ESP32-S3

Ce projet utilise un ESP32-S3 pour transformer un clavier USB filaire en un clavier sans fil en l'hébergeant via USB OTG et en diffusant les touches via Bluetooth HID.

## Aperçu

Le microcontrôleur ESP32-S3 est utilisé pour activer la fonctionnalité USB OTG (On-The-Go), ce qui lui permet d'agir comme hôte pour les périphériques USB. Ce projet se concentre spécifiquement sur la conversion d'un clavier USB filaire standard en un clavier sans fil qui communique via Bluetooth HID (Human Interface Device).

## Fonctionnalités

- **Support USB OTG**: Permet à l'ESP32-S3 d'interfacer avec les claviers USB, supportant la répétition automatique des touches, les touches modificatrices et le basculement des LEDs du clavier.
- **Bluetooth HID**: Transforme l'entrée du clavier USB en signaux Bluetooth, le rendant compatible avec divers appareils équipés de Bluetooth.

## Prise en main

### Utilisation

1. Connectez le clavier USB filaire à la carte ESP32-S3 en plaçant le D- USB (blanc) dans la broche GPIO 19 et le D+ USB (vert) dans la broche GPIO 20.
2. Téléchargez le firmware sur l'ESP32-S3.
3. Appairez l'ESP32-S3 avec votre appareil compatible Bluetooth.
4. Commencez à taper sur le clavier USB filaire, et l'entrée sera transmise sans fil via Bluetooth.

## Crédits

Ce projet est basé sur les bibliothèques suivantes:

- [EspUsbHost](https://github.com/tanakamasayuki/EspUsbHost): Fournit la fonctionnalité d'hôte USB pour l'ESP32.
- [ESP32-BLE-Keyboard](https://github.com/T-vK/ESP32-BLE-Keyboard): Facilite la communication Bluetooth HID pour l'ESP32.

Un grand merci aux auteurs des bibliothèques EspUsbHost et ESP32-BLE-Keyboard.
