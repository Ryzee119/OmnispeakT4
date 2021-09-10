# OmnispeakT4
A port of Commander Keen "Commander Keen in Goodbye Galaxy!" to the Teeny4.1 microcontroller. :gun:

* Note: Requires game files from the original game. Ref https://github.com/sulix/omnispeak. The shareware version of Keen4 is linked there too.
* Save game support support to an SD Card.
* USB input driver (hardcoded for Xbox360 type controllers).
* Save and load SRAM (Press Y to save).
* Genuine OPL ADLIB Audio supported!. Requires [YM3812 OPL2 FM-synthesizer](https://www.tindie.com/products/cheerful/opl2-audio-board/).

## Needed Parts
| Qty | Part Description | Link |
|--|--|--|
| 1 | Teensy 4.1 | https://www.pjrc.com/store/teensy41.html |
| 1 | USB Host Cable | https://www.pjrc.com/store/cable_usb_host_t36.html |
| 3 | 0.1" Pin Header | https://www.pjrc.com/store/header_24x1.html |
| 1 | 64Mbit PSRAM  SOIC-8 | https://www.pjrc.com/store/psram.html |
| 1 | ILI9341 TFT LCD Display (320x240) | [AliExpress](https://www.aliexpress.com/wholesale?catId=0&SearchText=ili9341%20tft) |
| 1 | YM3812 OPL2 FM-synthesizer (Optional for audio support) | [Tindie](https://www.tindie.com/products/cheerful/opl2-audio-board/) |

## Wire-up
* Install PSRAM chip on designation footprint on the Teensy41. Ref https://www.pjrc.com/store/psram.html.
* Solder pin header to the USB Host port on the Teensy41. Ref https://www.pjrc.com/store/cable_usb_host_t36.html.
* Wire the TFT and optionally the FM synthesizer module as per below:

### TFT Wiring (LED and Power connected to 3V pin)
| TFT Pin | Teensy41 Pin |
|--|--|
| DC | 38 |
| CS | 40 |
| MOSI | 26 |
| SCK | 27 |
| MISO | 39 |
| RST | 41 |

### YM3812 OPL2 FM-synthesizer Wiring
| OPL Pin | Teensy41 Pin |
|--|--|
| RESET | 8 |
| A0 | 9 |
| LATCH | 10 |
| DATA | 11 |
| SHIFT | 13 |

## Compile/Program/Usage
* Copy Omnispeak and Keen game files to the root of a FAT32 formatted SD card. Ref https://github.com/sulix/omnispeak. The shareware version of Keen4 is readily available.
* Hook up a Xbox 360 wired controller to the USB host connector.
* Download and install [Visual Studio Code](https://code.visualstudio.com/).
* Install the [PlatformIO IDE](https://platformio.org/platformio-ide) plugin.
* Clone this repo recursively `git clone --recursive https://github.com/Ryzee119/OmnispeakT4.git`.
* In Visual Studio Code `File > Open Folder... > OmnispeakT4`.
* The build process defaults to Episode 4. To compile episode 5 or 6 edit `platformio.ini` and change the `-DEP4` to `-DEP5` or `-DEP6`.
* Hit build on the Platform IO toolbar (`✓`).
* Hit the program button on the Platform IO toolbar (`→`).
