#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <Servo.h>
#include <WebServer.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define IR_SENSOR_PIN                A1
#define SERVO_PIN                     9
#define LED_GREEN                     7
#define LED_RED                       5
#define PUSH_BUTTON_PIN               8

#define START_RUN                     'a'
#define RESTART_ESP32                 'r'
#define RESTART_SCAN                  'n'

#define CLOSE_DOOR                    180
#define OPEN_DOOR                     90
#define DOOR_LOCK                     0
#define OPEN_DOOR_TIME                4000
#define CLOSE_DOOR_TIME               1000
#define RESTART_TIME                  5000

#define ON                            0
#define OFF                           1

#define MASK_DETECT_TH                80 //in percent %
#define SCAN_ERROR_COUNT              3

#define SCAN_NO_OBJECT                 0
#define SCAN_PASS                      1
#define SCAN_ERROR                     2

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
Servo myServo;

const char* WIFI_SSID = "THANH_VNPT";
const char* WIFI_PASS = "77777777";

unsigned short gusMask_Detect = 0;
char gszIP_Add[20];
unsigned short gusIsSensor_Detect_Bef = 0;
unsigned short gusIsSensor_Detect_Time_Bef = 0;
unsigned long glStart_Timer_LCD = 0;
unsigned long glRestart_Timer = 0;
unsigned short gusLCD_Index = 0;
unsigned short gusIsNeedDisp = 1;
unsigned short gusIsNeed_Restart = 0;
unsigned short gusIsSend_Request = 0;

void setup()
{
  char szData[30];
  unsigned short usExit = 0;
  unsigned short usData_Len = 0;
  short a = 0;
  
  Serial.begin(9600);
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  lcd.init();                      // initialize the lcd 
  lcd.backlight();
  
  pinMode(IR_SENSOR_PIN,INPUT_PULLUP);
  pinMode(PUSH_BUTTON_PIN,INPUT_PULLUP);
  pinMode(LED_GREEN,OUTPUT);
  pinMode(LED_RED,OUTPUT);

  vLED_Control(SCAN_NO_OBJECT);
  vServo_Control(DOOR_LOCK);
  
  lcd.clear();
  lcd.print("Wifi");
  lcd.setCursor(0,1);
  lcd.print("connecting...");

  memset(szData, '\0', sizeof(szData));
  memset(gszIP_Add, '\0', sizeof(gszIP_Add));
 
  do
  { 
    usData_Len = usRead_Serial_Data(szData, sizeof(szData));
    
    if(usData_Len > 0)
    {
      for(short i=0; i<usData_Len; i++)
      {
        if(szData[i] == '#')
        {
          i++;
          while(szData[i] != ',')
          {
            gszIP_Add[a] = szData[i++];
            a++;
          }
          usExit = 1;
          break;
        }
      }
    }
    else
    {
      if(gusIsNeed_Restart == 0)
      {
        Serial.println(RESTART_ESP32);
        gusIsNeed_Restart = 1;
      }
      
      if((millis() - glRestart_Timer) > RESTART_TIME)
      {
        gusIsNeed_Restart = 0;
        glRestart_Timer = millis();
      }
    }
  }while(usExit == 0);

  vLCD_Disp_Ip(gszIP_Add);
  glStart_Timer_LCD = millis();
}

void loop()
{   
  char szData[30];
  unsigned short usRescan = 0; 
  unsigned short usData_Len = 0;

  if(digitalRead(IR_SENSOR_PIN) == ON) 
  {
    Serial.println(START_RUN);
    vDisp_Scanning();

  while (isResult() == false)
  {
    vDisp_NoMask();
    vServo_Control(DOOR_LOCK);
    vLED_Control(SCAN_ERROR);
    delay(1000);
  }
  vDisp_Mask();
  vLED_Control(SCAN_PASS);
  vServo_Control(OPEN_DOOR);
  delay(OPEN_DOOR_TIME); 
  vServo_Control(CLOSE_DOOR);
  delay(CLOSE_DOOR_TIME);
  }    
}

void vDisp_Scanning()
{
  lcd.clear();
  lcd.print("SCANNING..."); 
  lcd.setCursor(0,1);
  lcd.print("Pls wait & hold.");
}
void vDisp_NoMask()
{
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("PLEASE WEAR MASK");
}

void vDisp_Mask()
{
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Enter Allowed");
}

//CONTROL SERVO BASED ON RESULTS
void vServo_Control(int sDoor_Status)
{
  myServo.attach(SERVO_PIN);
  
  if(sDoor_Status == OPEN_DOOR) //open door
  {
    for(int i = CLOSE_DOOR; i > OPEN_DOOR; i--)
    {
      myServo.write(i);
      delay(20);
    }
  }
  else if(sDoor_Status == CLOSE_DOOR)
  {
    for(int i = OPEN_DOOR; i < CLOSE_DOOR; i++)
    {
      myServo.write(i);
      delay(20);
    }
  }
  else if(sDoor_Status == DOOR_LOCK)
  {
    myServo.write(CLOSE_DOOR);
  }

  myServo.detach();
}
//READ SERIAL DATA FROM ESP32-CAM
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
  
  Serial.flush();

  return i;
}

//GET RESULT FROM WEB API
boolean isResult()
{
  Serial.println("GET from /Result");
  HTTPClient http;
  http.begin("http://192.168.1.3:2222/Result");
  int httpResponseCode = http.GET();
  if (httpResponseCode>0) {
  Serial.print("HTTP Response code: ");
  Serial.println(httpResponseCode);
  String payload = http.getString();
  if (payload == "1") 
    {
      http.end();
      return true;
    }
  else
    {
      http.end();
      return false;
    }
  }
  http.end();
  return false;
}
