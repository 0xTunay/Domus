# Domus 


This project is an intelligent system for monitoring temperature, pressure, and humidity in rooms  

In each room you can install box with ESP32-C3, a BME280 sensor and an SPI display. The ESP32-C3 will send data from sensor to an MQTT broker, which is implemented on a Raspberry Pi 5 for processing and then forwarding it  to a web-site with all logs about the room, the sensor and other releted information.

**Currently(25.02.2026), I’m using a DHT sensor and the ESP32’s built-in sensor because I don’t have a BME280. I’m working with an ESP32-C3**

https://docs.espressif.com/projects/esp-faq/en/latest/software-framework/protocols/mqtt.html# 
