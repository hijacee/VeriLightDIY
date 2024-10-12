# VeriLightDIY
A DIY Wifi Version of the FinishLynx "VeriLight" that is controllable via the original .lss file. 

## Hardware
You'll need:

- an ESP32 with onboard Wifi, I used an ESP32-WROOM-32U DevKit board from AliExpress
- Some LED Matrices, I used two WS2812b 8x8 5V Grids
- an optional external Wifi antenna to enhance the wifi reception
- an optional mini OLED Screen to see the current configuration, I used an 0.96" OLED Display SSD1306 I2C 128x64 module 4 pin
- an optional momentary push button

## 3D CAD files
The files are from the first prototype and consists of 4 parts. Feel free to edit, adapt and print those parts for your needs.

- LED_Grid_stl is designed to fit 2 8x8 led matrixes quite tightly
- Cap_Display.stl has a cutout for the OLED display and the wifi antenna with a hexagonal cutout
- ESP_devboard_holder.stl is the middle part to hold the esp loosely in place (upright, USB connector facing downwards), here is definitely room for improvement
- Base_small.stl is the foot/base for the whole setup. In future iterations I could imagine to make this part larger to fit a powerbank directly into the base

## How to use the Light
