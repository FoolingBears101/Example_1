/* 
 * Adaptation de :
 => https://raw.githubusercontent.com/RuiSantosdotme/ESP32-Course/master/code/WiFi_Web_Server_DHT/WiFi_Web_Server_DHT.ino
 => https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-web-server-arduino-ide/
*/

// Import required libraries
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include "classic_setup.h"
#include "sensors.h"
#include "OneWire.h"
#include "DallasTemperature.h"

#include <ArduinoJson.h>

extern const char page_html[];

/* ---- Set timer ---- */
unsigned long loop_period = 10L * 1000; /* =>  10000ms : 10 s */

/* ---- LED ---- */
const int Red_Led_pin = 21;
const int Green_Led_pin = 18;

String Red_Led_State = "on";
String Green_Led_State = "on";

/* ---- Light ----*/
const int LightPin = A5; // Read analog input on ADC1_CHANNEL_5 (GPIO 33)

/* ---- TEMP ---- */
OneWire oneWire(32); // Pour utiliser une entite oneWire sur le port 23
DallasTemperature tempSensor(&oneWire) ;

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

/*---- added vars -----*/
  boolean overRide_Leds = false;
  String  overRide_state = "off";
  const String on_str = "off";          // bricolage :/ pour que "on" allume et "off" eteint
  const String off_str = "on";          // aucune idée pourquoi j'ai pourtant essayer plein de trucs
                                        // les delais pour le rendu sont trop cours pour m'approfondir sur le pourquoi ..... ca marche pour l'instant

void setup(){
  Serial.begin(9600);
  while (!Serial); // wait for a serial connection. Needed for native USB port only
  
  connect_wifi(); // Connexion Wifi  
  print_network_status();
  
  // Initialize the LED 
  setup_led(Red_Led_pin, OUTPUT, LOW);
  setup_led(Green_Led_pin, OUTPUT, LOW);
  
  // Init temperature sensor 
  tempSensor.begin();
  
  server.begin(); // Lancement du serveur

  // Route for root / web page
  auto root_handler = server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){

    // if some variable values are sent over the URL
    // the following code will read them and interpret them.
    // variables that can be passed : 
    const String red = "Red_Led_State";
    const String green = "Green_Led_State";
    const String override_str = "override";

    int paramNb = request->params();
    for(int i=0;i<paramNb;i++){
      AsyncWebParameter* p = request->getParam(i);
      
      // check if the variable passed is the red led state
      if(strcmp(p->name().c_str(),red.c_str()) )  
      {
        //save the state that was sent
        Red_Led_State = p->value();

      // read and apply the new state of the red led
      if (strcmp(Red_Led_State.c_str(),on_str.c_str())) 
        digitalWrite(Red_Led_pin, HIGH);
      else if (strcmp(Red_Led_State.c_str(), off_str.c_str()))
        digitalWrite(Red_Led_pin, LOW);  
        
        overRide_Leds = true;
        overRide_state = "on";
      }
      
      else if(strcmp(p->name().c_str(),green.c_str()) ) 
      {
        // Same process. Save, read and apply changes to the green led.
        Green_Led_State = p->value();
        if (strcmp(Green_Led_State.c_str(),on_str.c_str())) 
          digitalWrite(Green_Led_pin, HIGH);
        else if (strcmp(Green_Led_State.c_str(),off_str.c_str())) 
          digitalWrite(Green_Led_pin, LOW);
        
        overRide_Leds = true;
        overRide_state = "on";
      }

      else if(strcmp(p->name().c_str(),override_str.c_str()) )
      {
        // Same process. Save, read and apply changes to the override state.
        overRide_state = p->value();
        if (strcmp(overRide_state.c_str(),on_str.c_str())) 
        {
          overRide_Leds = false;
          overRide_state = "off";
        }
        else if (strcmp(overRide_state.c_str(),off_str.c_str())) 
        {
          overRide_Leds = true; 
          overRide_state = "on";
        }
      }
    }
    // send the html page when the page root is opened 
    request->send_P(200, "text/html", page_html, processor);
  });
  
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", get_temperature(tempSensor).c_str());
  });
  server.on("/light", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", get_light(LightPin).c_str());
  });

    server.on("/redLed", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", Red_Led_State.c_str());
  });

    server.on("/greenLed", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", Green_Led_State.c_str());
  });
    server.on("/override", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", overRide_state.c_str());
  });

// post request response : 
server.on(
    "/POST",
    HTTP_POST,
    [](AsyncWebServerRequest * request){},
    NULL,
    [](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total) {
        // create the JSON object containing all the data on the sensors and leds
        StaticJsonDocument<255> outputJson;
        outputJson["temperature"] = get_temperature(tempSensor);
        outputJson["light"] = get_light(LightPin);
        outputJson["greenLed"] = Green_Led_State;
        outputJson["redLed"] = Red_Led_State;
        outputJson["override"] = overRide_state;
        // transforme the object into a string 
        char payload[256];
        serializeJson(outputJson,payload);
        // send the HTTP POST response to the client
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", payload);
        request->send(response);
  });

  // Start server
  server.begin();
}
 
void loop(){  

  const int SJN = 100;
  const float SHJ = 26;
  const float SHN = 28;
  const float SBJ = 24;
  const float SBN = 22;
  tempSensor.requestTemperaturesByIndex(0);
  float t = tempSensor.getTempCByIndex(0);
  
  if (overRide_Leds == false) { // if the user didn't force the leds to a particular state
                                // apply the normal behavior from TP1's TODO
      if (analogRead(LightPin)> SJN) {
        if (t> SHJ) {
            digitalWrite(Green_Led_pin, HIGH); Green_Led_State = "on";
            digitalWrite(Red_Led_pin, LOW);    Red_Led_State = "off";
          } 
        else if (t< SBJ) {
            digitalWrite(Green_Led_pin, LOW);  Green_Led_State = "off"; 
            digitalWrite(Red_Led_pin, HIGH);   Red_Led_State = "on";
          } else {
            digitalWrite(Green_Led_pin, LOW);  Green_Led_State = "off"; 
            digitalWrite(Red_Led_pin, LOW);   Red_Led_State = "off";
          }
        }
    else {
        if (t> SHN) {
            digitalWrite(Green_Led_pin, HIGH); Green_Led_State = "on";
            digitalWrite(Red_Led_pin, LOW);    Red_Led_State = "off";
          } 
        else if (t< SBN) {
            digitalWrite(Green_Led_pin, LOW);  Green_Led_State = "off";   
            digitalWrite(Red_Led_pin, HIGH);   Red_Led_State = "on"; 
          }else{
            digitalWrite(Green_Led_pin, LOW);  Green_Led_State = "off"; 
            digitalWrite(Red_Led_pin, LOW);   Red_Led_State = "off";
          }
    }
  }
}

String processor(const String& var){
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return get_temperature(tempSensor);
  }
  else if(var == "LIGHT"){
    return get_light(LightPin);
  }
  else if(var == "REDLIGHT"){
    return Red_Led_State;
  }
  else if(var == "GREENLIGHT"){
    return Green_Led_State;
  }  
  else if(var == "OVERRIDE"){
   return overRide_state;
  } else
  return String();
}

/**========== HTML ==========**/
// C++11 standard introduced  raw string literal :
// Raw string literals look like  : R"token(text)token"
const char page_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    div.b {
    text-align: center;
    }
  
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .sensors-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
    .button {
      background-color: #4CAF50; /* Green */
      border: none;
      color: white;
      padding: 15px 32px;
      text-align: center;
      text-decoration: none;
      display: inline-block;
      font-size: 16px;
      margin: 4px 2px;
      cursor: pointer;
    }
    .button1 {background-color: #f44336;} /* Red */ 
    .button2 {background-color: #4CAF50;} /* green */ 
    .button3 {background-color: #555555;} /* Black */
    
  </style>
</head>
<body>
  <h2>ESP32 Temperature regulation</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="sensors-labels">Temperature sensor :</span> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-sun" style="color:#FFFF00;"></i> 
    <span class="sensors-labels">Light sensor :</span>
    <span id="light">%LIGHT%</span>
    <sup class="units">Lumen</sup>
  </p>
  
    <p>
    <i class="far fa-lightbulb" style="color:#FF0000;"></i> 
    <span class="sensors-labels">Red Led state :</span>
    <span id="RedLed">%REDLIGHT%</span>
    <button class="button button1" onclick="redLedFunction()">Turn on/off</button>
  </p>
  
    <p>
    <i class="far fa-lightbulb" style="color:#00FF00;"></i> 
    <span class="sensors-labels">Green Led state :</span>
    <span id="GreenLed">%GREENLIGHT%</span>
    <button class="button button2" onclick="greenLedFunction()">Turn on/off</button>
  </p>

    <p>
    <i class="far fa-times-circle" style="color:#000000;"></i>  
    <span class="sensors-labels"> Led override  </span>
        <span id="LedOverride">%OVERRIDE%</span>
    <button class="button button3" onclick="greenLedFunction()">stop</button>
  </p>
    
    <div class = "b">
    <p class= "sensors-labels">
    when a led state is forced to "on" or "off" the override value will become "on". assign "off" value to go back to a normal working behavior. 
  </p>
  </div>
  
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("light").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/light", true);
  xhttp.send();
}, 10000 ) ;


setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("GreenLed").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/greenLed", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("RedLed").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/redLed", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("LedOverride").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/override", true);
  xhttp.send();
}, 10000 ) ;


</script>
</body>
</html>)rawliteral";
