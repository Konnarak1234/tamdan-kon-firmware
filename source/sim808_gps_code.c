#include <HardwareSerial.h>

#include <WiFi.h>
#include <HTTPClient.h>

#define SIM808_TX    16
#define SIM808_RX    17
#define SIM808_BAUD  9600

// define wifi credential
#define SSID "iPhone"
#define PWD  "22224444"

// define url 
#define URL ""

// initial UATRs port 2 of esp32 to communicate with sim808
HardwareSerial modem(2);

// --------------- function declaration ---------------------

// do http post requesting
void httpPostRequest(String data);
// connection to wifi
void connectWifi();
// sendAT command function 
String sendAT(String cmd);
// turning on gps function
int turnGPSOn();
// get gps function
String getGPS();



void setup() {
  Serial.begin(115200);
  // invoke connectWifi() to initial the wifi connection
  connectWifi();
  // begin the seriel communication port2 of esp32 to SIM808
  modem.begin(SIM808_BAUD, SERIAL_8N1, SIM808_RX, SIM808_TX);

  delay(500);

  int gpsStatusOn = 0;
  // keep running until gps is turn on
  while(!gpsStatusOn){
    gpsStatusOn = turnGPSOn();
  }     // wait for SIM808 boot
}

void loop() {
  String gps = getGPS();
  // if there is available of gps value
  if (gps != "") {
    int idx = gps.indexOf(',');
    Serial.println("GPS: " + gps);
    String latitude = gps.substring(0, idx);
    String longitude = gps.substring(idx + 1);
    String coordinate_data = "latitude=" + latitude + "&" + "longitude=" + longitude + "&busId=1";
    httpPostRequest(coordinate_data);
  }
  else {
    Serial.println("No fix yet...");
  }

  delay(125);
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

String sendAT(String cmd) {

  // esp32 to send the cmd to SIM808, to doing specific staff
  modem.println(cmd);

  unsigned long t = millis();
  String resp = "";


  // SIM808 is response taking time, so we allocate 2s get the answer from SIM808 to esp32
  while (millis() - t < 2000) {
    while (modem.available()) {
      // read income data, byte by byte
      resp += (char) modem.read();
    }

      // .indexOf() is c build-in method of type String -- to find the occurance of the givin substr, by return it position
    if (resp.indexOf("OK") >= 0 || resp.indexOf("ERROR") >= 0)
      break;
  }

  // output the resp to through seriel communication device
  Serial.print("CMD: ");
  Serial.print(cmd);
  Serial.print(" => ");
  Serial.println(resp);

  return resp;
} // overall this function take around 2s to complete it task

int turnGPSOn() {
  sendAT("ATE0");             // turn off echo
  delay(200);

  sendAT("AT+CGNSPWR=1");     // power GPS engine
  delay(200);

  sendAT("AT+CGNSMOD=1");     // set GNSS standalone mode
  delay(500);

  // verify the gps module to turn on, 
  String r = sendAT("AT+CGNSPWR?");

  if (r.indexOf("+CGNSPWR: 1") >= 0){
    Serial.println("GPS powered ON");
    return 1;
  }
  else{
    Serial.println("GPS failed to start!");
    return 0;
  }
    
}

String getGPS() {
  // start request gps information, here is it example output +CGNSINF: 1,1,20260421...,11.5564,104.9282,...
  String resp = sendAT("AT+CGNSINF");

  int p = resp.indexOf("+CGNSINF:");
  if (p < 0) return "";

  // extract the data, only store the string that contain number 1,1,20260421...,11.5564,104.9282,...
  String d = resp.substring(p + 10);

  // clean the OK replying string
  d.replace("OK", "");
  // cleaning leading and trailing white space, tab and newline
  d.trim();

  // indexing the string by storing the position of comma
  int idx[10];
  idx[0] = d.indexOf(',');
  for (int i = 1; i < 10; i++) {
    idx[i] = d.indexOf(',', idx[i-1] + 1);
  }


  // checking the responce of gps to be ok, which is 1
  String fix = d.substring(idx[0] + 1, idx[1]);
  Serial.println("GPS Fix: " + fix);

  if (fix != "1") return "";

  String lat = d.substring(idx[2] + 1, idx[3]);
  String lon = d.substring(idx[3] + 1, idx[4]);

  return lat + "," + lon;
}