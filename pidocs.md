# Installation

- enable SPI via raspi-config &rarr; Interfacing Options
- install nRF24's RF24 driver
  - `git clone https://github.com/nRF24 -b v1.3.2`
  - `./configure --driver=RPi`
  - `make install`
- `/opt/settings.json` (example):
```json
{
   "admin_username":"",
   "admin_password":"",
   "ce_pin":25,
   "csn_pin":8,
   "reset_pin":0,
   "led_pin":-2,
   "radio_interface_type":"nRF24",
   "packet_repeats":200,
   "http_repeat_factor":1,
   "auto_restart_period":0,
   "mqtt_server":"",
   "mqtt_username":"",
   "mqtt_password":"",
   "mqtt_topic_pattern":"milight/:device_id/:device_type/:group_id",
   "mqtt_update_topic_pattern":"milight/updates/:hex_device_id/:device_type/:group_id",
   "mqtt_state_topic_pattern":"milight/states/:hex_device_id/:device_type/:group_id",
   "mqtt_client_status_topic":"",
   "simple_mqtt_client_status":true,
   "discovery_port":48899,
   "listen_repeats":3,
   "state_flush_interval":10000,
   "mqtt_state_rate_limit":500,
   "mqtt_debounce_delay":500,
   "packet_repeat_throttle_sensitivity":0,
   "packet_repeat_throttle_threshold":200,
   "packet_repeat_minimum":3,
   "enable_automatic_mode_switching":false,
   "led_mode_wifi_config":"Fast toggle",
   "led_mode_wifi_failed":"On",
   "led_mode_operating":"Slow blip",
   "led_mode_packet":"Flicker",
   "led_mode_packet_count":3,
   "hostname":"ML00",
   "rf24_power_level":"MAX",
   "rf24_listen_channel":"LOW",
   "wifi_static_ip":"192.168.0.31",
   "wifi_static_ip_gateway":"192.168.0.1",
   "wifi_static_ip_netmask":"255.255.255.0",
   "packet_repeats_per_loop":6,
   "home_assistant_discovery_prefix":"homeassistant/",
   "wifi_mode":"n",
   "default_transition_period":500,
   "rf24_channels":[
      "LOW",
      "MID",
      "HIGH"
   ],
   "device_ids":[
		333
   ],
   "gateway_configs":[
      [         
         333,
         5987,
         6
      ]
   ],
   "group_state_fields":[
      "state",
      "brightness",
      "mode",
      "color_temp",
      "bulb_mode",
      "computed_color"
   ],
   "group_id_aliases":{
      "bedroom":[
         "fut096",
         333,
         1
      ]
   }
}
```

# Usage

- run `./program`
- pairing works e.g. with [milight 3.0 smartphone app from iOS app store](https://apps.apple.com/de/app/mi-light-3-0/id1156007632)
# Development

On Pi:

- `apt install git build-essential libssl-dev libffi-dev python3-dev python3-pip`
- `pip3 install -U platformio`
- `apt purge python3-cryptography`
- Start agent: `pio remote agent start` (2x)

On dev env:
- `pio remote --agent raspberrypi run -e raspberrypi -r`
- above command will fail at the first time. Following files have to be adjusted:

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

- Run `pio remote --agent raspberrypi run -e raspberrypi` again
- Executable can be found in `~/.platformio/remote/projects/esp8266_milight_hub-<SOME_ID>/.pio/build/raspberrypi/program`
