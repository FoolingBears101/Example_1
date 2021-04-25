#include <ArduinoJson.h>
#include <Arduino.h>
#include <analogWrite.h>

#include "OneWire.h"
#include "DallasTemperature.h"

#define TRUE 1
#define FALSE 0
String receivedStr; 

// the pin numbers does not correspond to the original card's pins 
// i used a clone ESP32 card. change the pins to appropriate ones

const int tempPin = 32;                
const int redLedPin = 5;         
const int blueLedPin = 17;      
const int lightSensorPin = 14;  

OneWire oneWire(tempPin); 
DallasTemperature tempSensor(&oneWire); 

StaticJsonDocument<200> jsonBuffer;
DeserializationError JsonError;

void setup() {
  Serial.begin(9600); // init de la laison filaire 
  tempSensor.begin(); // Init du capteur et de l'entite OneWire
  pinMode(redLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);  
}

void loop() {
  float t;            // variable that will recieve the heat sensor's value
  int lightSensor;    //    /         /            /    light sensor's value
  int redState = LOW, blueState = LOW; // will have the led's values 
  int command = 99;
  
  
  tempSensor.requestTemperaturesByIndex(tempPin);
  t = tempSensor.getTempCByIndex(0); 
  
  lightSensor = analogRead(lightSensorPin);   // read analog input : light

  while(Serial.available() > 0) {  // check if there is any data on the serial port
    receivedStr =  Serial.readStringUntil('\n'); // read the data
    JsonError = deserializeJson(jsonBuffer,receivedStr); // parse the Json data
    command = jsonBuffer["command"]; // get the command from the parsed Json
  }

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
    blueState = HIGH; // no command recieved or json parser error
    redState = HIGH;    // mostly for debugging :p i won't heat and cool the room at the same time
  } else {
    blueState = LOW;  // in the perfect range of temperature
    redState = LOW;       
  }
  // light up the leds using the command we recieved
  digitalWrite(blueLedPin, blueState);
  digitalWrite(redLedPin, redState);

  // make a json declaration containing our sensor's and led's states
  StaticJsonDocument<256> outputJson;
  outputJson["temperature"] = t;
  outputJson["light"] = lightSensor;
  outputJson["blueLed"] = blueState;
  outputJson["redLed"] = redState;
  // convert the Json into a table of chars and send it on the serial port
  char payload[256];
  serializeJson(outputJson,payload);
  Serial.println(payload);
  
  delay(5000); 
}
