# HumanToHumanInterface 
## 실행 순서
1. 파일- 환경 설정- 추가적인 보드 매니저 URLs에 아래 주소를 복사하여 넣어줍니다.
https://dl.espressif.com/dl/package_esp32_index.json,http://arduino.esp8266.com/stable/package_esp8266com_index.json

2. 툴- 보드- 보드 매니저를 누르고 'esp32'와 'esp8266'을 설치합니다.

3. 툴- 라이브러리 관리- 'PubSubClient'를 설치합니다.

4. 파일- 환경설정을 눌러 스케치북 위치를 확인하고 libraries 폴더를 그 경로에 다운받습니다.

#### 코드 실행
__[MosquittoAFinalOther.ino]__
1. 툴- 보드 - ESP32 Arduino - Lollin D32 Pro 
2. 툴- Partition Scheme - Minimal SPIFFS


__[MosquittoBFinal.ino]__
1. 툴- 보드 - ESP8266 Boards - Lollin D1 mini Pro
2. 툴- Partition Scheme - Minimal SPIFFS
