#include <PubSubClient.h>
#include <adc.h>
#include <adcfilter.h>
#include "esp_wifi.h"
#include "ESPAsyncWebServer.h"
#include "BluetoothSerial.h"
using namespace std;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#define WIFI_CREDENTIAL_SZ 16

char bl_count = 0;
uint16_t len;
bool wifiCredsRcvd = false;
bool deviceRcvd = false;
char t_count = 0;

// Connected Devices
#define maxConn 30
string connList[maxConn];
short int connCount = 0;

// Update these with values suitable for your network.
const char* apSSID = "gbrainHHI";
const char* apPASS = "gbrain1908";
const char* ssid = "SSID";
const char* password = "PASSWORD";
const char* mqtt_server = "test.mosquitto.org";
const char* TopicID = "Gbrain";
const char* clientName = "GbrainMain3"; // GbrainMain 1~99 겹치지 않게.
const char* BluetoothName = "GH3";

AsyncWebServer wbServer(80);

typedef struct wifi_creds {
  char ssid[WIFI_CREDENTIAL_SZ + 1];
  char passwd[WIFI_CREDENTIAL_SZ + 1];
} wifi_creds_t;

char DeviceNum = 0; // device 개수
char DeviceList[10][WIFI_CREDENTIAL_SZ + 1]; // device name
char conDevice = 0;
long lastMsg = 0;
char msg[50];
int value = 0;
int threshold = 1;

adcFilter lpfFilt;
adcFilter envFilt;
unsigned long flag_time;

WiFiClient espClient;
PubSubClient client(espClient);

BluetoothSerial SerialBT;

void alternateDelay(int delayMS)
{
  int currentTime = millis();
  while ((millis() - currentTime) < delayMS) {
    // Empty loop
  }
}

// broker에서 message 수신시 topic, payload값 출력
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// MQTT 브로커에 연결하고 특정 토픽 subscribe
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientName)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish(TopicID, "Reconnected");
      // ... and resubscribe
      for (int i = 0; i < DeviceNum; i++)
      {
        Serial.print("Connect Device : ");
        Serial.println(DeviceList[i]);
        client.subscribe(DeviceList[i]);
      }
      digitalWrite(BUILTIN_LED, LOW); // LED ON
      MQTTOff();
    } else {
      digitalWrite(BUILTIN_LED, HIGH); // LED OFF
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
} 

void callbackBT(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {

  if (event == ESP_SPP_SRV_OPEN_EVT) {
    Serial.println("Client Connected");
  }
}

// 릴레이 모듈 제어
void MQTTOn()
{
  snprintf (msg, 75, "%d", 1);
  for (int i = 0; i < DeviceNum; i++) {
    String SendMsg(String(DeviceList[i]));
    char TopicMsg[20];
    SendMsg.toCharArray(TopicMsg, 20);
    Serial.print(TopicMsg);
    Serial.print("-Send Data : ");
    Serial.println(msg);
    client.publish(TopicMsg, msg);
  }
  //  digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on, Send the ok
}

void MQTTOff()
{
  snprintf (msg, 75, "%d", 0); // msg에 0을 숫자로
  for (int i = 0; i < DeviceNum; i++) {
    String SendMsg(String(DeviceList[i]));

    char TopicMsg[20];
    SendMsg.toCharArray(TopicMsg, 20);
    Serial.print(TopicMsg);
    Serial.print("-Send Data : ");
    Serial.println(msg);
    client.publish(TopicMsg, msg);
  }
  //  digitalWrite(BUILTIN_LED, HIGH);   // Turn the LED on, Send the ok
}


void waitClient(void)
{
  char state[5];

  wifi_sta_list_t wifi_sta_list;
  tcpip_adapter_sta_list_t adapter_sta_list;

  memset(&wifi_sta_list, 0, sizeof(wifi_sta_list));
  memset(&adapter_sta_list, 0, sizeof(adapter_sta_list));
  Serial.println("wait Client");

  while (1) {

    // Get list of connected devices (stations associated with the soft AP)
    // soft AP와 관련된 스테이션 목록을 가져옴.
    esp_wifi_ap_get_sta_list(&wifi_sta_list);

    // Get ip address of connected stations
    // esp32의 ap 인터페이스에 연결된 모든 스테이션의 ip 정보를 얻어옴
    tcpip_adapter_get_sta_list(&wifi_sta_list, &adapter_sta_list);

    if (SerialBT.available()) {
      String txt = SerialBT.readString();
      Serial.println(txt);
      txt.toCharArray(state, txt.length() + 1);

      // cali가 입력되면 softAP 상태에서 빠져나옴.
      if (strcmp(state, "cali") == 0) break;
    }

    // Loop through connected devices
    for (int i = 0; i < adapter_sta_list.num; i++) {
      // station : connected deivce info (struct)
      tcpip_adapter_sta_info_t station = adapter_sta_list.sta[i];

      // Save MAC address to string macStr
      char macAddr[18];
      sprintf(macAddr, "%02X:%02X:%02X:%02X:%02X:%02X", MAC2STR(station.mac));
      string macStr = macAddr;

      // Save IP address to ipAddr -> convert to (string) ipStr
      char ipAddr[18];
      sprintf(ipAddr, "%s", ip4addr_ntoa(&(station.ip)));
      string ipStr = ipAddr;

      // Check if deivce is already recorded
      bool alreadyExists;

      for (int j = 0; j < maxConn; j++) {
        if (connList[j] == macStr) {
          alreadyExists = true;
          break;
        } else {
          alreadyExists = false;
        }
      }

      // If not, add macStr, ipAddr to list.
      if (alreadyExists == false) {
        connList[connCount] = macStr;
        //ipList[connCount] = ipAddr;
        connCount += 1;
        SerialBT.println(macAddr); // 스마트폰에 연결된 디바이스 출력
        Serial.println(macAddr);
      }
      alternateDelay(500);
    }
  }
}

void propogateWiFiInfo(const char* ssid, const char* password, const char* TopicID)
{
  String wsData(String(ssid) + "\n" + String(password) + "\n" + String(TopicID));
  wbServer.on("/propogate", HTTP_GET, [wsData](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", wsData);
  });
  wbServer.begin();
}

// WiFi AP setup
void SoftAPSetup(const char* ssid, const char* password, const char* TopicID)
{
  Serial.println("SoftAPSetup");
  WiFi.softAP(apSSID, apPASS);
  alternateDelay(100);
  // Static WiFi IP
  WiFi.softAPConfig(IPAddress(192, 168, 9, 9), IPAddress(192, 168, 9, 9), IPAddress(255, 255, 255, 0));
  IPAddress myIP = WiFi.softAPIP();

  propogateWiFiInfo(ssid, password, TopicID);

}

// 입력받은 device 정보 저장
void device_conn()
{
  Serial.println("Waiting Device Connection...");
  deviceRcvd = false;

  while (!deviceRcvd)
  {
    if (SerialBT.available())
    {
      String txt = SerialBT.readString();
      txt.toCharArray(DeviceList[DeviceNum], txt.length() + 1);
      Serial.println(DeviceList[DeviceNum]);

      if (strcmp(DeviceList[DeviceNum], "end") == 0)
      {
        deviceRcvd = true;
        break;
      }
      DeviceNum++;
    }
  }
}

void parse_wifi_info(wifi_creds_t* wifi_creds) {

  Serial.print("WiFi SSID : ");
  Serial.println(wifi_creds->ssid);
  Serial.print("WiFi Passwd : ");
  Serial.println(wifi_creds->passwd);

  device_conn();

  Serial.print("DeviceList : ");
  Serial.println((int)DeviceNum);

  for (int i = 0; i < DeviceNum ; i++)
  {
    Serial.print("Device Name: ");
    Serial.println(DeviceList[i]);
  }
}

// WiFi 정보 받아오기
wifi_creds_t* read_wifi_creds(void) {

  // WiFi Credential struct
  wifi_creds_t* wifi_creds = (wifi_creds_t*) malloc(sizeof(wifi_creds_t));
  wifiCredsRcvd = false;
  bl_count = 0;

  Serial.println("Waiting WiFi Connection...");

  while (!wifiCredsRcvd)
  {
    if (SerialBT.available())
    {
      String txt = SerialBT.readString();
      Serial.println(txt);

      if (sizeof(txt) > 2)
      {
        if (bl_count == 0)
        {
          txt.toCharArray(wifi_creds->ssid, txt.length() + 1);
          bl_count = 1;
        }
        else if (bl_count == 1)
        {
          txt.toCharArray(wifi_creds->passwd, txt.length() + 1);
          wifiCredsRcvd = true;
          bl_count = 0;
        }
      }
    }
    delay(50);
  }

  parse_wifi_info(wifi_creds);

  delay(2000);

  Serial.println("send wifi info to phone");
  
  // to smartphone
  SerialBT.println((char *)(wifi_creds->ssid));
  delay(50);
  SerialBT.println((char *)(wifi_creds->passwd));

  return wifi_creds;
}

void setup() {

  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);

  // ADC
  adc1_config_channel_atten(ADC1_CHANNEL_4, ADC_ATTEN_DB_11);
  adc1_config_width(ADC_WIDTH_BIT_12);

  // Start Ble
  SerialBT.begin(BluetoothName);
  Serial.println("The device started, now you can pair it with bluetooth!");
  SerialBT.register_callback(callbackBT);

  wifi_creds_t* wifi_creds = read_wifi_creds();
  SoftAPSetup((const char*) wifi_creds->ssid, (const char*) wifi_creds->passwd, TopicID);
  waitClient(); // 핫스팟 상태에서 디바이스 접속 대기 cali누르기 전까지

  wbServer.end();
  delay(5000);

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  WiFi.begin(wifi_creds->ssid, wifi_creds->passwd);

  while (WiFi.status() != WL_CONNECTED) { // wifi 네트워크에 연결되지 않았을 때
    delay(500);
    Serial.print(wifi_creds->ssid);
    Serial.print(",");
    Serial.println(wifi_creds->passwd);
    digitalWrite(BUILTIN_LED, HIGH);
  }

  Serial.println("WiFi Connected!");
  Serial.println(wifi_creds->ssid);
  Serial.println(wifi_creds->passwd);
  digitalWrite(BUILTIN_LED, LOW);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float num[] = {1.2345, 1.2133, 1.3457, 1.3463, 1.2356};
  float den[] = {1.0, 1.0, 1.0, 1.0};
  lpfFilt.init(num, den, 1.0, 1);
  envFilt.init(num, den, 1.0, 1);

  int flag = 1;

  int startTime = 0;
  bool relayState = false;
  int valSave = 0;
  int OnCount = 0;
  int OffCount = 0;
  int MaxCount = 4;
  bool Start = true;
  char s_state[6];
  threshold = 80; // 기본 - 50 힘안줄때 - 20이하

  while (true) {

    if (SerialBT.available()) {
      String txt = SerialBT.readString();
      Serial.println(txt);
      txt.toCharArray(s_state, txt.length() + 1);

      // 입력받은 것이 숫자이면 그 값을 threshold 값으로 지정.
      for (int k = 0 ; k < strlen(s_state); k++) {
        if (!isDigit(s_state[k])) break;
        
        if (k == strlen(s_state) - 1){
          threshold = atoi(s_state);
          Serial.print("threshold : ");
          Serial.println(threshold);
        }
      }

      // app에서 start 버튼 눌렀을 때
      if (strcmp(s_state, "start") == 0) Start = true;
    }

    while (Start) {

      if (SerialBT.available()) {
        String txt = SerialBT.readString();
        Serial.println(txt);
        txt.toCharArray(s_state, txt.length() + 1);

        for (int k = 0 ; k < strlen(s_state); k++) {
          if (!isDigit(s_state[k])) break;
          
          if (k == strlen(s_state) - 1){
            threshold = atoi(s_state);
            Serial.print("threshold : ");
            Serial.println(threshold);
          }
        }

        // stop 버튼 눌렀을 때
        if (strcmp(s_state, "stop") == 0) {
          MQTTOff();
          flag = 1;
          Start = false;
          break;
        }
      }

      if (!client.connected()) {
        Serial.println("reconnect");
        reconnect();
      }

      // delayMicroseconds(1900);

      int valSave = 0;
      for (int i = 0; i < 3; i++) {
        int val = envFilt.envelope(lpfFilt.lpf(adc1_get_raw(ADC1_CHANNEL_4)));
        valSave += (int) val;
        delayMicroseconds(1900 * 3);
      }

      if (valSave > 1000)
      {
        if (flag == 0) {
          MQTTOff();
          flag = 1;
        }

        OnCount = 0;
        OffCount = 0;
        continue;
      }

      if (valSave >= 0) {
        SerialBT.println(valSave); // 전송
        Serial.println(valSave);
        delay(40);
      }

      if (valSave > threshold && OnCount <= MaxCount)
      {
        OnCount++;
        OffCount = 0;
        continue;
      }
      else if (valSave < threshold && OffCount <= MaxCount)
      {
        OffCount++; // 끌 때
        OnCount = 0;
        continue;
      }

      if (valSave > threshold && flag == 1 && OnCount > MaxCount) {
        MQTTOn();
        flag = 0; // 릴레이 state
        OnCount = 0;
      }
      else if (valSave < threshold && flag == 0 && OffCount > MaxCount) {
        MQTTOff();
        flag = 1;
        OffCount = 0;
      }

    }
  }
}
