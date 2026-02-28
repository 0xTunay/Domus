# Domus 


This project is an intelligent system for monitoring temperature, pressure, and humidity in rooms  

In each room you can install box with ESP32-C3, a BME280 sensor and an SPI display. The ESP32-C3 will send data from sensor to an MQTT broker, which is implemented on a Raspberry Pi 5 for processing and then forwarding it  to a web-site with all logs about the room, the sensor and other releted information.

This branch is used for emulating BME280 and sending telemetry to the MQTT broker

This repository serves as the ESP-GATEWAY between ESP32-C3 nodes.
For the ESP32-C3 slave nodes, I'll create a separate repository.
 

https://docs.espressif.com/projects/esp-faq/en/latest/software-framework/protocols/mqtt.html# 
