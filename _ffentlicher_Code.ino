
#include <ESP8266WiFi.h>
#include "HTTPSRedirect.h"
#include <AccelStepper.h>

const int stepsPerRevolution = 2048;  // change this to fit the number of steps per revolution

// ULN2003 Motor Driver Pins
#define IN1 5
#define IN2 4
#define IN3 0
#define IN4 2


// initialize the stepper library
AccelStepper stepper(AccelStepper::HALF4WIRE, IN1, IN3, IN2, IN4);

//Connection Settings
const char* host = "script.google.com";
const char* googleRedirHost = "script.googleusercontent.com";
const int httpsPort = 443;

unsigned long entryCalender, entryPrintStatus, entryInterrupt, heartBeatEntry, heartBeatLedEntry;
String url;

#define UPDATETIME 10000

//Network credentials
const char*  ssid = "Put your Wifi Name here";
const char* password = "Put your Wifi Password here"; //replace with your password
//Google Script ID
const char *GScriptIdRead = "Put Your Google Calendar ID here"; //replace with you gscript id for reading the calendar
const char *GScriptIdWrite = "..........."; //replace with you gscript id for writing the calendar

String  possibleEvents[6] = {"Stufe1", "Stufe2",  "Stufe3", "Stufe4", "Stufe5", "Stufe6"};

HTTPSRedirect* client = nullptr;

String calendarData = "";
bool calenderUpToDate;

bool eventHere(int temp) {
  if (calendarData.indexOf(possibleEvents[temp], 0) >= 0 ) {
    return true;
  } else {
    return false;
  }
}

void setTemperature(){
  if(eventHere(0)==true){
    Serial.print("Heizung Stufe 1");
    stepper.runToNewPosition(0);
    delay(100);
  }
  else if(eventHere(1)==true){
    Serial.print("Heizung Stufe 2");
    stepper.runToNewPosition(336);
    delay(100);
  }
  else if(eventHere(2)==true){
    Serial.print("Heizung Stufe 3");
    stepper.runToNewPosition(672);
    delay(100);
  }
  else if(eventHere(3)==true){
    Serial.print("Heizung Stufe 4");
    stepper.runToNewPosition(1008);
    delay(100);
  }
  else if(eventHere(4)==true){
    Serial.print("Heizung Stufe 5");
    stepper.runToNewPosition(1344);
    delay(100);
  }
  else if(eventHere(5)==true){
    Serial.print("Heizung Stufe 6");
    stepper.runToNewPosition(1680);
    delay(100);
  }
}
//Connect to wifi
void connectToWifi() {
  Serial.println();
  Serial.print("Connecting to wifi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected ");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Use HTTPSRedirect class to create a new TLS connection
  client = new HTTPSRedirect(httpsPort);
  client->setInsecure();
  client->setPrintResponseBody(true);
  client->setContentTypeHeader("application/json");

  Serial.print("Connecting to ");
  Serial.println(host);

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i = 0; i < 5; i++) {
    int retval = client->connect(host, httpsPort);
    if (retval == 1) {
      flag = true;
      break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }

  if (!flag) {
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("Exiting...");
    ESP.reset();
  }
  Serial.println("Connected to Google");
}

void getCalendar() {
  //  Serial.println("Start Request");
  // HTTPSRedirect client(httpsPort);
  unsigned long getCalenderEntry = millis();

  // Try to connect for a maximum of 5 times
  bool flag = false;
  for (int i = 0; i < 5; i++) {
    int retval = client->connect(host, httpsPort);
    if (retval == 1) {
      flag = true;
      break;
    }
    else
      Serial.println("Connection failed. Retrying...");
  }
  if (!flag) {
    Serial.print("Could not connect to server: ");
    Serial.println(host);
    Serial.println("Exiting...");
    ESP.reset();
  }
  //Fetch Google Calendar events
  String url = String("/macros/s/") + GScriptIdRead + "/exec";
  client->GET(url, host);
  calendarData = client->getResponseBody();
  Serial.print("Calendar Data---> ");
  Serial.println(calendarData);
  calenderUpToDate = true;
  yield();
}


void setup() {
  // set the speed at 50 rpm
  stepper.setSpeed(50);
  stepper.setAcceleration(500);
  
  Serial.begin(115200);
  Serial.println("Reminder_V2");
  connectToWifi();
  getCalendar();
  entryCalender = millis();
}

void loop() {
    if (millis() > entryCalender + UPDATETIME) {
    getCalendar();
    entryCalender = millis();
    setTemperature();
  }
}
