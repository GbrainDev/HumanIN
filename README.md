# HumanToHumanInterface 
## Arduino Settings

1. Click __File-Preferences__ to check the sketchbook location and download the code and libraries to that floder.

2. Copy and paste the following address in __File-Preferences-Additional Board Manager URLs__. https://dl.espressif.com/dl/package_esp32_index.json,http://arduino.esp8266.com/stable/package_esp8266com_index.json

3. Click __Tools-Board-Board Manager__ and install **esp32** and **esp8266**.

+ If you get an error like _"Multiple libraries were found for "WiFi.h"_,
 go to the path ***Not used :*** and delete the **WiFi** folder.

_________________________________
### Code execution

__[MosquittoAFinalOther.ino]__
1. Tools-Board-ESP32 Arduino-Lollin D32 Pro
2. Tools-Partition Scheme-Minimal SPIFFS

__[MosquittoBFinal.ino]__
1. Tools-Boards-ESP8266 Boards-Lollin D1 mini Pro
