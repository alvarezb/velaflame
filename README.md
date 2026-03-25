# velaflame

Copyright (C) 2026 Brian Alvarez. Licensed under the [GNU General Public License v3.0](LICENSE).
ESPHome firmware for the beautiful Velaflame candles:
https://www.velaflame.com/

To flash the new firmware, follow these [Instructions](FLASH_FIRMWARE.md)

Once you have ESPHome firmware installed on your Velaflame (such as the
barebones setup from the firmware flashing), you should update it to use the [config.yaml](config.yaml) from this repo as your main config file.

Make sure to copy the generated passwords from your minimal config into the
substitutions block in this config.yaml, or your device will no longer connect
to Home Assistant.

You are welcome to change the
`device_name` and `friendly_name` to meet  your needs, for example `device_name:
velaflame06` or `friendly_name:  upstairs hallway sconce`

Once you have set up your config file, you can use ESPHome Builder to install it
onto your velaflame, using the `Install` -> `Wireless` flow.

Most of the logic gets imported from [velaflame.yaml](esphome/velaflame.yaml),
keeping the config file that you need to interact with pretty minimal and
focused on things you may want to change. You can obviously add additional
ESPHome configurations in this `config.yaml` if desired.

If you make any improvements (especially to the LED or Wick flicker code), please open a PR so we can improve things for everyone.
