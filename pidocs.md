# openmilight_pi

- enable SPI via raspi-config &rarr; Interfacing Options
- install nRF24's RF24 driver: https://nrf24.github.io/RF24/RPi.html
- install openmilight_pi (drivers can be skipped): https://github.com/bakkerr/openmilight_pi
- Anpassung Code:
--- https://github.com/bakkerr/openmilight_pi/blob/master/openmilight.cpp#L379
--- https://github.com/bakkerr/openmilight_pi/blob/master/openmilight.cpp#L690



# Development

On Pi:

- apt install git build-essential libssl-dev libffi-dev python3-dev python3-pip
- pip3 install -U platformio
- apt purge python3-cryptography
-
/root/.platformio/platforms/linux_arm/boards/raspberrypi_3b.json

```json
"frameworks": [
    "arduino"
  ]
```
/root/.platformio/platforms/linux_arm/platform.json

Add 

```json
"arduino": {
      "package": "framework-arduinoespressif8266",
      "script": "builder/frameworks/arduino.py"
    }
  },
```

to frameworks

Add

```json
    "framework-arduinoespressif8266": {
      "type": "framework",
      "optional": true,
      "version": "~2.20402.0"
    }
```
to packages

- git clone https://github.com/nRF24 -b v1.3.2
- ./configure --driver=RPi
- make install
- pio remote agent start
