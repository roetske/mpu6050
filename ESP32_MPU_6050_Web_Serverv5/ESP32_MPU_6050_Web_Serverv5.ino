/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-mpu-6050-web-server/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
  
  
  board esp32 dev mod
  begint al goe met arduino 2 kan je geen files laden
  #include <ArduinoJson.h> is ook anders
 "{\"accX\":\"000.000\",\"accY\":000.000\",\"accZ\":\"000.000\"}"
  StaticJsonDocument<64> doc;
  doc.set("{\"accX\":\"000.000\",\"accY\":000.000\",\"accZ\":\"000.000\"}");
  serializeJson(doc, output);

  verdomme das zoeken mag niet zou mooi geweest zijn geen string gedoe maar werkt niet moet string zijn
  
  "{\"accX\":000.000,\"accY\":000.000,\"accZ\":000.000}"
  uploaden code
  dan spiffs dat moet je maar 1 keer doen 
  bij verandering code moet je spiffs niet meer doen
  doc.clear();
  eenmaal geschreven moet je clearen om opnieuw te schrijven
  dit marcheert maar heb drift op gyro
  nu calibreren
*/

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
//json
StaticJsonDocument<64> doc;
StaticJsonDocument<64> doc2;

// Replace with your network credentials
const char* ssid = "kelder";
const char* password = "Brugge1966";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Timer variables
unsigned long lastTime = 0;  
unsigned long lastTimeTemperature = 0;
unsigned long lastTimeAcc = 0;
unsigned long gyroDelay = 10;// ?
unsigned long temperatureDelay = 1000;
unsigned long accelerometerDelay = 200;

// Create a sensor object
Adafruit_MPU6050 mpu;

sensors_event_t a, g, temp;

float gyroX, gyroY, gyroZ;
float accX, accY, accZ;
float temperature;

//Gyroscope sensor deviation
float gyroXerror = 0.07;
float gyroYerror = 0.03;
float gyroZerror = 0.01;


void setup() {
  Serial.begin(115200);
  initWiFi();
  initSPIFFS();
  initMPU();

  //config
  //mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  //mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  
  // Handle Web Server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
 
  
  server.serveStatic("/", SPIFFS, "/");
  
  
  
  server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){
    gyroX=0;
    gyroY=0;
    gyroZ=0;
    request->send(200, "text/plain", "OK");   
  });
  
  server.on("/resetX", HTTP_GET, [](AsyncWebServerRequest *request){
    gyroX=0;
    request->send(200, "text/plain", "OK");
  });
 
  server.on("/resetY", HTTP_GET, [](AsyncWebServerRequest *request){
    gyroY=0;
    request->send(200, "text/plain", "OK");
  });
 
  server.on("/resetZ", HTTP_GET, [](AsyncWebServerRequest *request){
    gyroZ=0;
    request->send(200, "text/plain", "OK");
  });

  // Handle Web Server Events
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    // and set reconnect delay to 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
 
  server.addHandler(&events);
   
  //jason init
   doc["gyroX"] = "0.00";
   doc["gyroY"] = "0.00";
   doc["gyroZ"] = "0.00";
   doc2["accX"] = "0.00";
   doc2["accY"] = "0.00";
   doc2["accZ"] = "0.00";
//  //alles als string
//    doc.set("{\"gyroX\":\"0.00\",\"gyroY\":0.00\",\"gyroZ\":\"0.00\"}"); 
//    doc2.set("{\"accX\":\"0.00\",\"accY\":0.00\",\"accZ\":\"0.00\"}");
  //index string waarde float mag niet alles moet tekst zijn
//  doc.set("{\"accX\":000.000,\"accY\":000.000,\"accZ\":000.000}");
//  doc2.set( "{\"accX\":000.000,\"accY\":000.000,\"accZ\":000.000}");
 
  server.begin();
}

void loop() {
  if ((millis() - lastTime) > gyroDelay) {
    // Send Events to the Web Server with the Sensor Readings
    events.send(getGyroReadings().c_str(),"gyro_readings",millis());
    lastTime = millis();
  }
  if ((millis() - lastTimeAcc) > accelerometerDelay) {
    // Send Events to the Web Server with the Sensor Readings
    events.send(getAccReadings().c_str(),"accelerometer_readings",millis());
    lastTimeAcc = millis();
  }
  if ((millis() - lastTimeTemperature) > temperatureDelay) {
    // Send Events to the Web Server with the Sensor Readings
    events.send(getTemperature().c_str(),"temperature_reading",millis());
    lastTimeTemperature = millis();
  }
}
