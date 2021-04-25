/* DESCRIPTION TODO
Vous réalisez une régulation de température ambiante pour une pièce d’habitation.
Bien sûr, les seuils de régulation varient selon que l’on est en "Jour" ou en "Nuit".
➣ C’est le capteur de lumière (ADC1 Channel 0) qui donnera l’information "Jour/Nuit".
Un seuil (SJN) sera nécessaire.
Vous contrôlez l’évolution de la température grâce à une climatisation (modélisée par une LED verte GPIO
19 ou une ventilateur) et un radiateur (modélisé par une LED rouge GPIO 20).
✔ Si la température (GPIO 23) est supérieure à un seuil haut (SHJ et SHN) vous climatisez.
✔ Si la température (GPIO 23) est inférieure à un seuil bas (SBJ et SBN) vous chauffez
 */

#include <Arduino.h>
#include <analogWrite.h>

#include "OneWire.h"
#include "DallasTemperature.h"


const int tempPin = 32;         //pin IO26 on the board
const int redLedPin = 5;         //pin IO26 on the board
const int blueLedPin = 17;         //pin IO25 on the board
const int lightSensorPin = 14; //pin IO14

OneWire oneWire(tempPin); // Pour utiliser une entite oneWire sur le port 32
DallasTemperature tempSensor(&oneWire); // Cette entite est utilisee par le capteur

void setup() {
  Serial.begin(9600); // starts the serial port at 9600
  tempSensor.begin(); // Init du capteur et de l'entite OneWire
  pinMode(redLedPin, OUTPUT);
  pinMode(blueLedPin, OUTPUT);  
}

void loop() {
  // temp sensor
  float t;
  tempSensor.requestTemperaturesByIndex(0);
  // light sensor
  int sensorValue;
  const int SJN = 100;
  const float SHJ = 26;
  const float SHN = 28;
  const float SBJ = 24;
  const float SBN = 22;
  int range = 0;
  
  sensorValue = analogRead(lightSensorPin);
  t = tempSensor.getTempCByIndex(0);        // Read analog input on ADC1_CHANNEL_5 (GPIO 33)
  if (sensorValue> SJN) {
    if (t> SHJ) {
        digitalWrite(blueLedPin, HIGH);
        digitalWrite(redLedPin, LOW);
      } 
    else if (t< SBJ) {
        digitalWrite(blueLedPin, LOW);
        digitalWrite(redLedPin, HIGH);   
      }
    }
  else {
    if (t> SHN) {
        digitalWrite(blueLedPin, HIGH);
        digitalWrite(redLedPin, LOW);
      } 
    else if (t< SBN) {
        digitalWrite(blueLedPin, LOW);
        digitalWrite(redLedPin, HIGH);    
      }
      else     
  }
  delay(1000); // delay 1 s
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" C\n");
  Serial.println(sensorValue);
}
