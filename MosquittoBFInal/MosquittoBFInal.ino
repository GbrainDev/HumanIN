#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>

const char* apSSID = "gbrainHHI";
const char* apPASS = "gbrain1908";
char ssid[20];
char passwd[20];
const char* mqtt_server = "test.mosquitto.org";
char TopicID[20] = "GH7";  // 안드로이드 앱상에서 디바이스 이름 수정
const char* clientName = "ICRecv7";// GbrainSub 1~99 겹치지않게 수정

WiFiClient espClient;
PubSubClient client(espClient);
HTTPClient http; 

long lastMsg = 0;
char msg[50];
int value = 0;

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  pinMode(D1, OUTPUT);
  Serial.begin(115200);
  digitalWrite(BUILTIN_LED, LOW);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(apSSID);

  WiFi.begin(apSSID, apPASS);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("wifi first");
  
  http.begin("http://192.168.9.9/propogate");
  int httpResponseCode = http.GET();
  delay(10);
  
  if (httpResponseCode > 0) {
    String payload = http.getString();
    delay(10);
    Serial.println(payload);

    int first = payload.indexOf("\n");// 첫 번째 콤마 위치
    int second = payload.indexOf("\n", first + 1); // 두 번째 콤마 위치
    int len = payload.length(); // 문자열 길이

    String ID = payload.substring(0, first); // 첫 번째 토큰 (0, 3)
    String PASS = payload.substring(first + 1, second); // 두 번째 토큰 (4, 7)
    String TOPIC = payload.substring(second + 1, len); // 세 번째 토큰(8, 10)

    ID.toCharArray(ssid, 50);
    PASS.toCharArray(passwd, 50);
    //TOPIC.toCharArray(TopicID, 50);
    
    Serial.print("SSID - ");
    Serial.println(ssid);
    Serial.print("PASSWD - ");
    Serial.println(passwd);
    Serial.print("TOPIC - ");
    Serial.println(TopicID);
    Serial.println("get 성공!");
  }

  WiFi.disconnect();
  WiFi.begin(ssid, passwd);

  Serial.println("wifi Second");
  
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(BUILTIN_LED, LOW);
    delay(250);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(250);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message Length : ");
  Serial.println(length);
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  //strcmp(topic,"pir1Status")==0)
  if ((char)payload[0] == '0') {
    digitalWrite(D1, LOW);
//    digitalWrite(BUILTIN_LED, HIGH);
    Serial.println("Relay ON");
  } else {
    digitalWrite(D1, HIGH);
//    digitalWrite(BUILTIN_LED, LOW);
    Serial.println("Relay OFF");  // Turn the LED off by making the voltage HIGH
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientName)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("Gbrain", "Reconnected");
      // ... and resubscribe
      client.subscribe(TopicID);
      digitalWrite(BUILTIN_LED, LOW);
    } else {
      digitalWrite(BUILTIN_LED, HIGH);
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() {

  if (!client.connected()) {
    digitalWrite(BUILTIN_LED, HIGH);
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000)
  {
    lastMsg = now;
  }
  
}
