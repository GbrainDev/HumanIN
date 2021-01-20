# HumanToHumanInterface 
## Order of execution

1. Copy and paste the following address in __File-Preferences-Additional Board Manager URLs__. https://dl.espressif.com/dl/package_esp32_index.json,http://arduino.esp8266.com/stable/package_esp8266com_index.json

2. Click __Tools-Board-Board Manager__ and install **esp32** and **esp8266**.

3. Click __Tools- Manage__ Libraries and install**PubSubClient**.

4. Click __File- Preferences__ to check the sketchbook location and download the **libraries** folder to that path.

_________________________________
### Code execution

__[MosquittoAFinalOther.ino]__
1. Tools-Board-ESP32 Arduino-Lollin D32 Pro
2. Tools-Partition Scheme-Minimal SPIFFS

__[MosquittoBFinal.ino]__
1. Tools-Boards-ESP8266 Boards-Lollin D1 mini Pro
