# Installation

- enable SPI via raspi-config &rarr; Interfacing Options
- install nRF24's RF24 driver
  - `git clone https://github.com/pfink/RF24 -b v1.3.4`
  - `./configure --driver=RPi`
  - `make install`
- Download a pre-built version of this application from https://drive.google.com/file/d/1ZsX9KftmbgQ-2vkDxB5ncHiwg2fpLLrb/view?usp=sharing (alternatively, you could build it on your own, see "Development" section)
- Create `/opt/settings.json` with the following content (example, the commented parts usually have to be adjusted, comments have to be removed afterwards):
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
   "wifi_static_ip":"192.168.0.31",             // IP address of the pi
   "wifi_static_ip_gateway":"192.168.0.1",      // IP address of the router
   "wifi_static_ip_netmask":"255.255.255.0",    // network mask
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
      333               // For each milight bridge to be emulated add an arbitrary 3-digit device id here
      // add more comma-seperated device id's here if required
   ],
   "gateway_configs":[
      [                 // For each milight bridge, a config has to be added here
         333,           // This is your device id from above
         5987,          // This is the port the emulated bridge will run
         6              // The version of the emulated bridge, currently only 6 is supported/tested
      ]
      // add more comma-seperated bridge configurations here if required
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
         "fut096",      // Type of the milight remote that is emulated. This depends on the milight lamp you're using. See https://github.com/sidoh/esp8266_milight_hub#supported-remotes
         333,           // Device id of the emulated bridge thislamp belongs to
         1              // Group #1 (value has to be between 1 and 4)
      ],
      "livingroom":[
         "fut096",      // Type of the milight remote that is emulated. This depends on the milight lamp you're using. See https://github.com/sidoh/esp8266_milight_hub#supported-remotes
         333,           // Device id of the emulated bridge thislamp belongs to
         2              // Group #2 (value has to be between 1 and 4)
      ]
      // add more comma-seperated configs here if required
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
