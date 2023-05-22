#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "env.h"
#include <OneWire.h>
#include <DallasTemperature.h>

const char* postendpoint = API_URL_POST;
const char* getendpoint = API_URL_GET;

#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);	

const int fPin = 22;
const int lPin = 23;
const int IrPin = 15;
int Irstate;

float float_rand(float min,float max)
{
    float scale = rand() / (float) RAND_MAX; /* [0, 1.0] */
    return min + scale * ( max - min );      /* [min, max] */
}

void setup() {
  sensors.begin();
  Serial.begin(9600);
  pinMode(fPin, OUTPUT);
  pinMode(lPin, OUTPUT);
  pinMode(IrPin,INPUT);
  pinMode(ONE_WIRE_BUS,INPUT);

  WiFi.begin(WIFI_USER, WIFI_PASS);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("The Bluetooth Device is Ready to Pair");
  Serial.println("Connected @");
  Serial.print(WiFi.localIP());
}


void loop() {

//READ TEMPERATURE
// Send the command to get temperatures
  sensors.requestTemperatures(); 

  //print the temperature in Celsius
  Serial.print("Temperature: ");
  float temp = sensors.getTempCByIndex(0);
  temp = sensors.getTempCByIndex(0);
  Serial.print(temp);
  Serial.print((char)176);//shows degrees character
  Serial.print("C  |  "); 
  Irstate = digitalRead(IrPin);
  Serial.print("\n");
  Serial.print("");
  Serial.print(Irstate);
  Serial.println("");
  
//POST Request
  if(WiFi.status()== WL_CONNECTED){   
    
    HTTPClient http;
    String http_response;

    //POST REQUEST
    http.begin(postendpoint);
    http.addHeader("Content-Type", "application/json");
    http.addHeader("content-length", "69");

    StaticJsonDocument<1024> postdoc; // Empty JSONDocument
    String httpRequestData; // Emtpy string to be used to store HTTP request data string
    
    postdoc["temperature"]=temp;
    postdoc["presence"]=!Irstate;
    serializeJson(postdoc, httpRequestData);

    int POSTResponseCode = http.POST(httpRequestData);


    if (POSTResponseCode>0) {
        Serial.print("Response:");
        Serial.print(POSTResponseCode);}

    else {
        Serial.print("Error: ");


        Serial.println(POSTResponseCode);}
      
      http.end();
      
    //GET REQUEST
    http.begin(getendpoint);
  

    int httpResponseCode = http.GET();


    if (httpResponseCode>0) {
        Serial.print("Response:");
        Serial.print(httpResponseCode);
        http_response = http.getString();
        Serial.println(http_response);}
      else {
        Serial.print("Error: ");
        Serial.println(httpResponseCode);}
      http.end();

      
      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, http_response);

      if (error) 
      { Serial.print("deserializeJson() failed:");
        Serial.println(error.c_str());
        return;}
      
      bool lightstate = doc["light"];
      bool fanstate = doc["fan"];
  
  
      Serial.println("Light:");
      Serial.println(lightstate);
      Serial.println("Fan:");
      Serial.println(fanstate);

      digitalWrite(fPin, fanstate);
      digitalWrite(lPin,lightstate);
      
      Serial.println("Light and Fan Switched Successfully");
      
      delay(1000);   
  }
  
  else {Serial.println("Connected failed");}

}