#include <Wire.h>
#include <Adafruit_PN532.h>

#include <WiFi.h>
#include <HTTPClient.h>

#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_PN532 nfc(SDA_PIN, SCL_PIN);

// define wifi credential
#define SSID "iPhone"
#define PWD  "22224444"

// define url 
#define URL ""

// do http post requesting
void httpPostRequest(String data);
// connection to wifi
void connectWifi();

int id = 1;

void setup() {
  Serial.begin(115200);
  
  // invoke connectWifi() to initial the wifi connection
  connectWifi();
  
  Wire.begin(SDA_PIN, SCL_PIN);

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (!versiondata) {
    Serial.println("PN532 not found");
    while (1);
  }

  Serial.println("PN532 ready");
  nfc.SAMConfig();
}

void loop() {
  uint8_t uid[7];
  uint8_t uidLength;
  

  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
	 Serial.print(id);
   String studentId = "studentId=" + String(id);
   Serial.print(studentId);
   httpPostRequest(studentId);
	 id++;
    Serial.print("UID: ");
    for (int i = 0; i < uidLength; i++) {
      Serial.print(uid[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
    delay(1000);
  }
}

// ---------------- function defination -----------------------

void connectWifi() {
  Serial.print("connect to wifi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PWD);

  while(WiFi.status() != WL_CONNECTED){
    Serial.print('.');
    delay(200);
  }

  if(WiFi.status() == WL_CONNECTED){
    Serial.println("connected!");
    Serial.println(WiFi.localIP());
  }
}

void httpPostRequest(String data) {
  HTTPClient http;
  http.begin(URL);

  http.addHeader("Accept", "application/json");
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.POST(data);

  String responseBody = http.getString();

  Serial.println("status code: "); Serial.print(httpCode);
  Serial.println("body: "); Serial.print(responseBody);

}