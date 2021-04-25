#include <ArduinoJson.h>
#include <Arduino.h>
#include <analogWrite.h>

#include "OneWire.h"
#include "DallasTemperature.h"

#define TRUE 1
#define FALSE 0
String receivedStr; 

// Numero de pins a modifié suivant votre carte. 
// les pins actuels correspond au branchement pour une carte ESP32 clone(chinois)

const int tempPin = 32;               
const int redLedPin = 5;        
const int blueLedPin = 17;     
const int lightSensorPin = 14; 

OneWire oneWire(tempPin); 
DallasTemperature tempSensor(&oneWire); 

StaticJsonDocument<200> jsonBuffer;
DeserializationError JsonError;

void setup() {
  Serial.begin(9600);  
  tempSensor.begin(); 
  pinMode(redLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);  
}

void loop() {
  float t;            // variable qui va recevoir la valeur du capteur de temperature
  int lightSensor;    //    capteur de lumiére
  
  int redState = LOW, blueState = LOW;  
  int command = 99;
  
  
  tempSensor.requestTemperaturesByIndex(tempPin);
  t = tempSensor.getTempCByIndex(0); 
  
  lightSensor = analogRead(lightSensorPin);   // read analog input : light

  Serial.print(F("{\n\"temperature\": \"")) ; 
  Serial.print(t);
  Serial.print(F("\",\n\"light\": \"")); 
  Serial.print(String(lightSensor));
  Serial.print(F("\",\n\"greenLed\": \"" )); 
  Serial.print(String(blueState));
  Serial.print(F("\",\n\"redLed\": \"" )); 
  Serial.print(String(redState));
  Serial.print(F("\",\n}\n")) ;


  while(Serial.available() > 0) {
    receivedStr =  Serial.readStringUntil('\n');
  }
  Serial.print(receivedStr);
  JsonError = deserializeJson(jsonBuffer,receivedStr);
  command = jsonBuffer["command"];
  Serial.println(command);
  if ((command != 99) and (command == 1))   // using the command instruction found in the JSON file received,
                                            // determine which LED to light up
  {        
    blueState = HIGH; // room is hot 
    redState = LOW;
  } else if ((command != 99) and (command == 2))
  {
    blueState = LOW;  // room is cold 
    redState = HIGH;      
  } else if ((command == 99) or JsonError)
  {
    blueState = HIGH; // never recieved any signal or json parser error
    redState = HIGH;    
  } else {
    blueState = LOW;  // in the perfect range of temperature
    redState = LOW;       
  }
    
  digitalWrite(blueLedPin, blueState);
  digitalWrite(redLedPin, redState);
  delay(5000); 
}
