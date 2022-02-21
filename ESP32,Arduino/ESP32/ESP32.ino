#include <WebServer.h>
#include <WiFi.h>
#include <esp32cam.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

unsigned int flag = 0;
const char* WIFI_SSID = "THANH_VNPT";
const char* WIFI_PASS = "77777777";
#define START_RUN 'a'
#define RESTART_ESP32 'r'
#define LED_BUILTIN 4
WebServer server(80);
 
 
static auto loRes = esp32cam::Resolution::find(320, 240);
static auto midRes = esp32cam::Resolution::find(350, 530);
static auto hiRes = esp32cam::Resolution::find(800, 600);
void serveJpg()
{
  
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("CAPTURE FAIL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("CAPTURE OK %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));
  
  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);
}
 
void handleJpgLo()
{
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("SET-LO-RES FAIL");
  }
  serveJpg();
  delay(100);

}
 
void handleJpgHi()
{
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }
  
  serveJpg();

}
 
void handleJpgMid()
{
  if (!esp32cam::Camera.changeResolution(midRes)) {
    Serial.println("SET-MID-RES FAIL");
  }
  serveJpg();
}
 
void FirstRun()
{
  Serial.println("GET from /FIRST");
  HTTPClient http;
  http.begin("http://192.168.1.3:2222/First");
  do
  {
    int httpResponseCode = http.GET();
    if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    String payload = http.getString();
    if (payload == "1") 
    {
      break;
    }
    }
    delay(5000);
  } while (true);
  http.end();
}
boolean SendJson(String s)
{
  Serial.println("POST local ip to /JSON");
  HTTPClient http;
  http.begin("http://192.168.1.3:2222/Json"); 
  http.addHeader("Content-Type", "application/json");  
  DynamicJsonDocument doc(1024); 
  doc["LocalIP"] = s;
  String requestBody;
  serializeJson(doc, requestBody);
  int httpResponseCode = http.POST(requestBody);
  if(httpResponseCode>0){
      String response = http.getString();                       
      Serial.println(httpResponseCode);   
      http.end();
      if (response == "1") return true;
    }
    http.end();
  return false;
}


void setup(){
  Serial.begin(9600);
  Serial.setDebugOutput(false);
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);  
    cfg.setJpeg(90);

    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "CAMERA OK" : "CAMERA FAIL");
  }

  Serial.print("http://");
  Serial.println(WiFi.localIP());
  Serial.println("  /cam-lo.jpg");
  Serial.println("  /cam-hi.jpg");
  Serial.println("  /cam-mid.jpg");
  server.on("/cam-mid.jpg", handleJpgMid);
  server.begin();
  FirstRun();
  delay(100);
  while (SendJson(WiFi.localIP().toString().c_str()) == false)
  {
    delay(100);
  }
  Serial.println("Init Completed");
}
 
void loop()
{
  char szData[3];
  if(usRead_Serial_Data(szData, sizeof(szData)) > 0)
  {
    if (szData[0] == START_RUN)
    {
      flag = 1;
    }
    else if (szData[0] == RESTART_ESP32)
    {
      flag = 0;
      ESP.restart();
    }
  }
  if (flag == 1)
  {server.handleClient();}
}

unsigned short usRead_Serial_Data(char *szData, short sDataSize)
{
  short i=0;
  
  if(Serial.available())
  {
    *(szData+i) = Serial.read();
    i++;
    delay(2);

    while(Serial.available())
    {
      *(szData+i) = Serial.read();
      i++;

      if(i >= sDataSize)
      {
        break;  
      }
      delay(2);      
    }
  }
  return i;
}