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
The first prototype consists of 4 parts. Feel free to edit, adapt and print those parts for your needs.

- LED_Grid_stl is designed to fit 2 8x8 led matrixes quite tightly
- Cap_Display.stl has a cutout for the OLED display and the wifi antenna with a hexagonal cutout
- ESP_devboard_holder.stl is the middle part to hold the esp loosely in place (upright, USB connector facing downwards), here is definitely room for improvement
- Base_small.stl is the foot/base for the whole setup. In future iterations I could imagine to make this part larger to fit a powerbank directly into the base

## Connection Diagram
Pin 12: Data-Pin LED Matrix
Pin 13: Reset Button
Pin 21: SDA Pin (OLED Screen)
Pin 22: SCL Pin (OLED Screen)

It is very well possible to power the LED Matrices (at least 2x 8x8 Matrices) via the 5V Pin of the ESP. You could also power the matrices directly via a separate +5V source (such as parallel to the the ESP from a USB Powerbank)

The following diagram shows the connectio (note that the LED matrix depicted here is a 4 Pin, I used a 3 Pin with just +5V, GND and DATA).

![Screenshot 2024-10-13 143249](https://github.com/user-attachments/assets/5a947ef3-50ed-4b9f-8873-23b75017b2aa)

## How to use the Light
### Network connection
You will need to set up a Wifi network that is also part of the PC you're running FinishLynx on. The Wifi-Router / AccessPoint you are using should be using DHCP to allocate IP addresses to new devices.

On the first start, the Light will go into access point (AP) Mode, as there are no known networks around. It will automatically broadcast an own wifi network named "ReadyLight_AP". Connect to the network with device of your choice - the default passwort is "12345678". Both is adaptable in Line 312 of the code. 
If your device is not directly redirected to the settings page, open a browser and type 192.168.4.1 (as the default Wifi-Manager IP).
The SSID and the IP address of the settings page are also displayed on the OLED display during AP-mode.
In the settings page you can scan for available networks. Chose your network from the list, enter the password if needed and save the configuration. The System will reboot and always connect automatically to the assigned network, if available. It will then also show it's current IP address on the OLED Screen as well as the Port the Light is listening for connection from FinishLynx.

To change the network properties or the FinishLynx Port, you need to re-enter the AP-mode. This is done in pressing the button within three seconds of the boot-up sequence.
If the last configured network is not available, the Light will also boot into AP-mode.

<img src="https://github.com/user-attachments/assets/8c436c62-eeb5-4024-8277-384b65616b37" width=30%>


### FinishLynx Setup
The Light is configured as TCP server. To add the Light, follow these steps:
1. In FinishLynx, go to **Scoreboard | Options…**
2. Click **New**.
3. Script: **VeriLight_Ready.lss**
4. Serial Port: **Network(connect)**
   1. Port: **10000** (default, can be changed in the Configuration-Page of the Light, shown on the OLED screen)
   2. IP address: **depending on your network**, shown on the OLED screen
5. Running Time: **Normal**
6. Results: **Off**
7. Click **Ok** and restart FinishLynx.

### Operation
As long as the Light is connected to the network, it will display it's IP address and Port on the OLED display. 
If there is no TCP connection established, the LEDs blink yellow in an one second interval. When FinishLynx connects to that light, the blinking stops and the OLED displays "CLIENT CONNECTED". If, for any reason (network out of range, FinishLynx is closed etc.), the TCP connection breaks, the display says "CLIENT DISCONNECTED" and the LEDs start blinking yellow again. So you an anyone can tell immediately, that something is wrong.

<img src="https://github.com/user-attachments/assets/bd4451d7-8c73-4243-b0cb-e0c2dc458583" width=30%> <img src="https://github.com/user-attachments/assets/f87dfded-4aa5-4c8e-9dde-fd3793c251fa" width=30%> <img src="https://github.com/user-attachments/assets/b49795e9-cca9-4137-956a-a1d3d8715dac" width=30%>

The behaviour of the Light is set through the original VeryLight.lss from Finishlynx. You can adapt this lss-script to your needs. In the lss you will find commands that look similar to this:

~~~
;;TimeArmed
\11\00\=0\41\42\44\00\00\07\04\04\04\04\04\01\00
\11\00\=1
~~~

Note that the bytes after `\11\00\=0\` contain the relevant information for the DIY as well as the original VeriLynx Hardware by Patlite. Look in the orignal manual for deeper context: [Patlite LA6-POE Manual](https://docs.rs-online.com/ff50/0900766b8167c578.pdf) 

The seventh byte (in the case above, the first **04**) sets the color for the entire DIY Light according to the following table (same as original Patlite color palette):

| Seventh byte      | Color |
| ----------- | ----------- |
| 00   | Black (off)        |
| 01   | Red       |
| 02   | Gold / Amber        |
| 03   | Green-Yellow / Lemon        |
| 04   | Green        |
| 05   | Sky-blue        |
| 06   | Blue        |
| 07   | Purple        |
| 08   | Pink        |
| 09   | White        |
| other   | Black (off)        |

Additionaly you can enable blinking of the light with the status of the 12th byte. If it is a **01**, blinking is enabled, if it is a **00**, blinking is disabled. Please note, that currently blinking does not work, when FinishLynx updates the output of the "scoreboard" every second or faster.
