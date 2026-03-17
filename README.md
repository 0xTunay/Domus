# Domus

Domus is a system for monitoring indoor temperature, pressure, and humidity.
Each ESP32-C3-based device collects data from a BME280 sensor and sends it to an MQTT broker running on a Raspberry Pi 5. The data is also displayed locally on an SPI TFT display.
Telemetry is stored in the InfluxDB database and visualized through a web interface using Grafana.

# Testing IoT system
I'll add GitHub Actions for the testing system and implement a self-hosted runner on my Raspberry Pi 5.
To start testing, run these commands:

```bash
python3 -m venv venv

source venv/bin/activate
```
```bash
pip install -r requirements.txt
```


_Currently all screenshot are reading from a DHT sensor, because Aliexpress delayed the delivery of my BME280 and ESP32-C3. Sorry._
