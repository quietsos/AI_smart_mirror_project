#include <ESP8266WiFi.h>        // library for wifi access
#include <WiFiClient.h>         // library for wifi client create
#include <ESP8266WebServer.h>   // library for creating webserver and webpage
#include <time.h>

//wether section

#include <ESP8266HTTPClient.h>
#include <Arduino_JSON.h>

String openWeatherMapApiKey = "fa1af2d32fbc3a660eefc4ead07e0d96";
// Replace with your country code and city
String city = "Kushtia";
String countryCode = "BD";

// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 10 seconds (10000)
unsigned long timerDelay = 10000;

String jsonBuffer;

double temp, humidity, pressure, speed;



//location section

#include <ArduinoJson.h>
//Credentials for Google GeoLocation API...
const char* Host = "www.googleapis.com";
String thisPage = "/geolocation/v1/geolocate?key=";

// --- Get a google maps ap key here : https://developers.google.com/maps/documentation/geolocation/intro
String key = "AIzaSyA8a_QtE0V8T-clnvv-oRnw5UNorgButzE";

int status = WL_IDLE_STATUS;
String jsonString = "{\n";

double latitude    = 23.0;
double longitude   = 89.0;
double accuracy    = 98.0;
int more_text = 1; // set to 1 for more debug output






//for microphone

#include <SoftwareSerial.h>       //library for software serial communication
#include "VoiceRecognitionV3.h"   //library for microphone operate voice record and identification
/**
  Connection
  NodeMCU(ESP8266)    VoiceRecognitionModule
   D2   ------->     TX
   D3   ------->     RX
*/
VR myVR(D2, D3);   // 2:RX 3:TX,
uint8_t records[7]; // save record
uint8_t buf[64];

#define onRecord    (0)   // onrecord to record the voice and store for further recognition
#define onRecordDht11 (1) // onRecordDhat11 store the command "show weather" and recognize it to show the weather information
#define onLocation (2)    // onLocation store the command "show location" and recognize it to show the weather information




#define motionPin D0   // defining for motion sensor to connect with this.
#define ledPin D1     // green led light that conform motion is detect and now microphone is ready to recognized voice

int motionState = 0; // initailly motion state remain null or zero value.



// Replace with your network credentials
const char* ssid = "ICE_Innovation_Lab"; //Enter Wi-Fi SSID
const char* password =  "beinnovative#"; //Enter Wi-Fi Password

ESP8266WebServer server(80);   //instantiate server at port 80 (http port)

// seclecting the time zone for your own region
int timezone = 6 * 3600;  // replace "6" as your time zone mine here is 6.
int dst = 0;


//variables for storing data
String page = "";
String text = "";
int data;


// variable to store realtime value of date and time
int day, month, year;       //variable to store day, month, year
int hour, minute, second;   // variable to store hour,minute and seconds



// this function is for defining wifi connection established conformation.

void wifiConnection() {

  WiFi.begin(ssid, password); //begin WiFi connection
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

}


// this function is used to check the current date time response.

void timeZoneResponse() {

  configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nWaiting for Internet time");

  while (!time(nullptr)) {
    Serial.print("*");
    delay(1000);
  }
  Serial.println("\nTime response....OK");

}


// creating webserver and webpage that show the data

void serverRunning() {

  server.on("/data.txt", []() {
    text = "Date: " + (String)day + "/" + (String)month + "/" + (String)year + " Time: " + (String)hour + ":" + (String)minute + ":" + (String)second + " Temperature: " + (String)temp + " Humidity: " + (String)humidity + " Pressure:" + (String)pressure + " Wind Speed:" + (String)speed +  " GeoLocation: Latitude:" + (String)latitude + " Longitude: " + (String)longitude;
    server.send(200, "text/html", text);
  });
  server.on("/", []() {
    page = "<h1>AI Smart Mirror</h1><h1>Status:</h1> <h1 id=\"data\">""</h1>\r\n";
    page += "<script>\r\n";
    page += "var x = setInterval(function() {loadData(\"data.txt\",updateData)}, 1000);\r\n";
    page += "function loadData(url, callback){\r\n";
    page += "var xhttp = new XMLHttpRequest();\r\n";
    page += "xhttp.onreadystatechange = function(){\r\n";
    page += " if(this.readyState == 4 && this.status == 200){\r\n";
    page += " callback.apply(xhttp);\r\n";
    page += " }\r\n";
    page += "};\r\n";
    page += "xhttp.open(\"GET\", url, true);\r\n";
    page += "xhttp.send();\r\n";
    page += "}\r\n";
    page += "function updateData(){\r\n";
    page += " document.getElementById(\"data\").innerHTML = this.responseText;\r\n";
    page += "}\r\n";
    page += "</script>\r\n";
    server.send(200, "text/html", page);
  });

  server.begin();
  Serial.println("Web server started!");

}


// update current date and time here.

void updateDateTime() {

  for ( int i = 0; i < 5; i++) {

    time_t now = time(nullptr);
    struct tm* p_tm = localtime(&now);
    //    Serial.print(p_tm->tm_mday);
    //    Serial.print("/");
    day = p_tm->tm_mday;

    //    Serial.print(p_tm->tm_mon + 1);
    //    Serial.print("/");
    month = p_tm->tm_mon + 1;

    //    Serial.print(p_tm->tm_year + 1900);
    year = p_tm->tm_year + 1900;

    Serial.print(" ");

    //    Serial.print(p_tm->tm_hour);
    //    Serial.print(":");
    hour = p_tm->tm_hour;
    //
    //    Serial.print(p_tm->tm_min);
    //    Serial.print(":");
    minute = p_tm->tm_min;

    //    Serial.println(p_tm->tm_sec);
    second = p_tm->tm_sec;


    server.handleClient();

    delay(500);

  }
}


// call this function after 5 seconds of motion detect that erase all the data of the webpage.

void eraseData() {
  day = 0;
  month = 0;
  year = 0;
  hour = 0;
  minute = 0;
  second = 0;
  temp = 0;
  humidity = 0;
  pressure = 0;
  speed = 0;
  latitude = 0;
  longitude = 0;

  server.handleClient();

}




void setup() {
  Serial.begin(115200);       // defining the boadrate of serial communication
  pinMode(motionPin, INPUT);  // defining the pin mode of motion pin as input
  pinMode(ledPin, OUTPUT);    // defining the led mode as output
  digitalWrite(ledPin, LOW);  // initially keeps the led as power off condition



  wifiConnection();      // calling wifi connection establishment function
  timeZoneResponse();   // calling timezone response function
  serverRunning();      // check weather the webserver is running well or not.



  /** initialize */
  myVR.begin(9600);     // initialize the microphone

  if (myVR.load((uint8_t)onRecord) >= 0) {
    Serial.println("onRecord loaded");
  }

  if (myVR.load((uint8_t)onRecordDht11) >= 0) {
    Serial.println("onRecordDht11 loaded");
  }

  if (myVR.load((uint8_t)onLocation) >= 0) {
    Serial.println("onLocation loaded");
  }





}

// this function detect voice and identify and when command is recognized then it call "updateDateTime()" function to send date and time to the webserver
void voiceDetect() {
  int ret;
  ret = myVR.recognize(buf, 50);
  if (ret > 0) {
    switch (buf[1]) {
      case onRecord:

        digitalWrite(ledPin, HIGH);
        updateDateTime();           // call this function to send the current date time to the webserver and shown in webpage.
        break;

      case onRecordDht11:
        digitalWrite(ledPin, HIGH);
        weatherApi();
        break;

      case onLocation:
        digitalWrite(ledPin, HIGH);
        locationApi();
        break;

      default:
        Serial.println("Record function undefined");
        break;
    }

  }

}


void weatherApi() {


  for (int i = 0; i < 5; i++) {

    // Send an HTTP GET request
    if ((millis() - lastTime) > timerDelay) {
      // Check WiFi connection status
      if (WiFi.status() == WL_CONNECTED) {
        String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;

        jsonBuffer = httpGETRequest(serverPath.c_str());
        Serial.println(jsonBuffer);
        JSONVar myObject = JSON.parse(jsonBuffer);

        // JSON.typeof(jsonVar) can be used to get the type of the var
        if (JSON.typeof(myObject) == "undefined") {
          Serial.println("Parsing input failed!");
          return;
        }


        temp = myObject["main"]["temp"];
        humidity = myObject["main"]["humidity"];
        pressure = myObject["main"]["pressure"];
        speed = myObject["wind"]["speed"];


        //      Serial.print("JSON object = ");
        //      Serial.println(myObject);
        //      Serial.print("Temperature: ");
        //      Serial.println(myObject["main"]["temp"]);
        //      Serial.print("Pressure: ");
        //      Serial.println(myObject["main"]["pressure"]);
        //      Serial.print("Humidity: ");
        //      Serial.println(myObject["main"]["humidity"]);
        //      Serial.print("Wind Speed: ");
        //      Serial.println(myObject["wind"]["speed"]);
      }
      else {
        //      Serial.println("WiFi Disconnected");
      }
      lastTime = millis();
    }

    delay(1000);

  }


}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;

  // Your IP address with path or Domain name with URL path
  http.begin(client, serverName);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    //    Serial.print("HTTP Response code: ");
    //    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    //    Serial.print("Error code: ");
    //    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}



void locationApi() {


  for( int i=0; i<5; i++){


  char bssid[6];

  DynamicJsonDocument doc(1024);

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();

  // now build the jsonString...
  jsonString = "{\n";
  jsonString += "\"homeMobileCountryCode\": 234,\n"; // this is a real UK MCC
  jsonString += "\"homeMobileNetworkCode\": 27,\n";  // and a real UK MNC
  jsonString += "\"radioType\": \"gsm\",\n";         // for gsm
  jsonString += "\"carrier\": \"Vodafone\",\n";      // associated with Vodafone
  jsonString += "\"wifiAccessPoints\": [\n";
  for (int j = 0; j < n; ++j)
  {
    jsonString += "{\n";
    jsonString += "\"macAddress\" : \"";
    jsonString += (WiFi.BSSIDstr(j));
    jsonString += "\",\n";
    jsonString += "\"signalStrength\": ";
    jsonString += WiFi.RSSI(j);
    jsonString += "\n";
    if (j < n - 1)
    {
      jsonString += "},\n";
    }
    else
    {
      jsonString += "}\n";
    }
  }
  jsonString += ("]\n");
  jsonString += ("}\n");
  //--------------------------------------------------------------------

  //  Serial.println("");

  WiFiClientSecure client;

  //Connect to the client and make the api call
  //  Serial.print("Requesting URL: ");
  // ---- Get Google Maps Api Key here, Link: https://developers.google.com/maps/documentation/geolocation/intro
  //  Serial.println("https://" + (String)Host + thisPage + "AIzaSyA8a_QtE0V8T-clnvv-oRnw5UNorgButzE");
  //  Serial.println(" ");
  if (client.connect(Host, 443)) {
    //    Serial.println("Connected");
    client.println("POST " + thisPage + key + " HTTP/1.1");
    client.println("Host: " + (String)Host);
    client.println("Connection: close");
    client.println("Content-Type: application/json");
    client.println("User-Agent: Arduino/1.0");
    client.print("Content-Length: ");
    client.println(jsonString.length());
    client.println();
    client.print(jsonString);
    delay(500);
  }

  //Read and parse all the lines of the reply from server
  while (client.available()) {
    String line = client.readStringUntil('\r');
    if (more_text) {
      //      Serial.print(line);
    }
    //    JsonObject& root = jsonBuffer.parseObject(line);
    DeserializationError error = deserializeJson(doc, line);
    if (error) {
      return;
    }
    latitude    = doc["location"]["lat"];
    longitude   = doc["location"]["lng"];
    accuracy   = doc["accuracy"];
  }

  //  Serial.println("closing connection");
  //  Serial.println();
  client.stop();

  //  Serial.print("Latitude = ");
  //  Serial.println(latitude, 6);
  //  Serial.print("Longitude = ");
  //  Serial.println(longitude, 6);
  //  Serial.print("Accuracy = ");
  //  Serial.println(accuracy);
  //
  //  delay(10000);
  //
  //  delay(2000);


  delay(1000);
    
  }

}






void loop() {

  eraseData();   // initailly keep the screen date, time data as "0" zero value

  motionState = digitalRead(motionPin);   // read motion value

  if (motionState == 1) {               // if motion is detected then green led will turn on and ask for voice command
    digitalWrite(ledPin, HIGH);

    voiceDetect();      // function for detecting voice and recognition of voice command

  }


  delay(1000);


}
