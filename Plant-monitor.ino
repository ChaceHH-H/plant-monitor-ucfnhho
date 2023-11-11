#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <ezTime.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <DHT_U.h>
#include "SH1106Wire.h"

//Change Dn to Arduino pin number
static const uint8_t D0   = 16;
static const uint8_t D1   = 5;
static const uint8_t D2   = 4;
static const uint8_t D3   = 0;
static const uint8_t D4   = 2;
static const uint8_t D5   = 14;
static const uint8_t D6   = 12;
static const uint8_t D7   = 13;
static const uint8_t D8   = 15;
static const uint8_t D9   = 3;
static const uint8_t D10  = 1;
#include "Graphic_esp8266_dht22_oledi2c.h"
//Initialize the OLED display using Wire library
SH1106Wire display(0x3c, D2, D1);


#define DHTTYPE DHT22   

uint8_t DHTPin = 12;      
uint8_t soilPin = 0;      
float Temperature;
float Humidity;
int Moisture = 1; 
int sensorVCC = 13;
int blueLED = 2;
DHT dht(DHTPin, DHTTYPE);  


// Wifi and MQTT
#include "arduino_secrets.h" 
const char* ssid     = SECRET_SSID;
const char* password = SECRET_PASS;
const char* mqttuser = SECRET_MQTTUSER;
const char* mqttpass = SECRET_MQTTPASS;

// Date and time
Timezone GB;

ESP8266WebServer server(80);
const char* mqtt_server = "mqtt.cetools.org";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


//connect to telegram-Telegram User ID-Bot Token 
#define CHAT_ID "My-ID"
#define BOTtoken "Bot Token"
#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif
//new WiFi client
WiFiClientSecure teleclient;
//Creat a bot
UniversalTelegramBot bot(BOTtoken, teleclient);
//check for new Telegram messages-Delay
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;


//IFTTT-host and key
const char* host = "maker.ifttt.com";
const char* apiKey = "IFTTT-Key";


//auto send delay
int period = 1800000;
unsigned long time_now = 0;


void setup() {
  // Set up LED to be controllable via broker
  // Initialize the BUILTIN_LED pin as an output
  // Turn the LED off by making the voltage HIGH
  pinMode(BUILTIN_LED, OUTPUT);     
  digitalWrite(BUILTIN_LED, HIGH);  

  // Set up the outputs to control the soil sensor
  // switch and the blue LED for status indicator
  pinMode(sensorVCC, OUTPUT); 
  digitalWrite(sensorVCC, LOW);
  pinMode(blueLED, OUTPUT); 
  digitalWrite(blueLED, HIGH);

  // open serial connection for debug info
  Serial.begin(115200);
  delay(100);

  // start DHT sensor
  pinMode(DHTPin, INPUT);
  dht.begin();

  // run initialisation functions
  startWifi();
  startWebserver();
  syncDate();

  // start MQTT server
  client.setServer(mqtt_server, 1884);
  client.setCallback(callback);

    //
  #ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      
    teleclient.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif

//Initialising the UI will init the display too
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_LEFT);

}

void loop() {
  // handler for receiving requests to webserver
  server.handleClient();

  if (minuteChanged()) {
    readMoisture();
    sendMQTT();
    Serial.println(GB.dateTime("H:i:s")); // UTC.dateTime("l, d-M-y H:i:s.v T")
  }

//Check for new messages every second. When new messages are received, call the numNewMessages function
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
  //Check value，call sendemail,warning delay
  float t, h;
  t = dht.readTemperature();
  h = dht.readHumidity();
  //Check the condition of plant every half hour,if the data is incorrect, call the sendemail function
  if(millis() >= time_now + period){
    if(t>38||t<20||h<60||h>40){
    time_now += period;
    sendemail();
    }

  }

  client.loop();
  display.clear(); // clearing the display
  displayTempHumid();
  delay(2000);  
}



//function requests temperature and humidity from dht22,returns the results as a string variable for Telegram bot.
String getReadings(){
  float temperature, humidity;
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  String message = "Warning \n";
  message += "Temperature: " + String(temperature) + " ºC \n";
  message += "Humidity: " + String (humidity) + " % \n";
  message += "Moisture: " + String (Moisture) + " % \n";
  return message;
}
//Check received Telegram messages and edit text and data needed to be sent
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));
  //Checks available messages
  for (int i=0; i<numNewMessages; i++) {
    //Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);
    String from_name = bot.messages[i].from_name;
    if (text == "/start") {
      String welcome = "Welcome, " + from_name + ".\n";
      welcome += "Use the following command to get current readings.\n\n";
      welcome += "/readings \n";
      bot.sendMessage(chat_id, welcome, "");
    }
    if (text == "/readings") {
      String readings = getReadings();
      bot.sendMessage(chat_id, readings, "");
    }  
  }
}

//send warning message to email and telegram
void sendemail(){
  Serial.println("send warning");
    String readings = getReadings();
    bot.sendMessage(CHAT_ID, readings, "");
  //Send email reminders of plant status values
    float T, H;
    T = dht.readTemperature();
    H = dht.readHumidity();
    Serial.print("connecting to ");
    Serial.println(host);
    WiFiClient client;
   const int httpPort = 80;
    if (!client.connect(host, httpPort)) {
      Serial.println("connection failed");
      return;
    }
    //Make a request to IFTTT and output three values
    String url = "/trigger/plant_monitor/with/key/";
    url += apiKey;
    Serial.print("Requesting URL: ");
      Serial.println(url);
      client.print(String("POST ") + url + " HTTP/1.1\r\n" +
                          "Host: " + host + "\r\n" + 
                          "Content-Type: application/x-www-form-urlencoded\r\n" + 
                          "Content-Length: 13\r\n\r\n" +
                          "value1=" + T + "\r\n" +
                          "value2=" + H + "\r\n" +
                          "value2=" + Moisture + "\r\n"
                          );
}


//Display data on OLED
void displayTempHumid() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  //Check if any reads failed and exit early
  if (isnan(h) || isnan(t) ) {   // || isnan(f)
    display.clear(); // clearing the display
    display.drawString(5, 0, "Failed DHT");
    display.display();
    return;
  }

  display.setColor(WHITE);

  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "TEMPERATURA");
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 26, String(t) + " C");
  display.drawXbm(100, 20, 20, 40, temp_logo);
  display.display();
  delay(5000);

  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "HUMIDITY");
  display.setFont(ArialMT_Plain_24);
  display.drawString(0, 26, String(h) + " %");
  display.drawXbm(94, 20, 29, 40, humi_logo);
  display.display();
  delay(5000);

  display.clear();
  display.drawXbm(31, 0, 67, 64, ok_01);
  display.display();
  delay(3000);
}



void readMoisture(){
  
  // power the sensor
  digitalWrite(sensorVCC, HIGH);
  digitalWrite(blueLED, LOW);
  delay(100);
  // read the value from the sensor:
  Moisture = analogRead(soilPin);         
  digitalWrite(sensorVCC, LOW);  
  digitalWrite(blueLED, HIGH);
  delay(100);
  Serial.print("Wet ");
  Serial.println(Moisture);   // read the value from the nails
}

void startWifi() {
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);


  // check to see if connected and wait until you are
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void syncDate() {
  // get real date and time
  waitForSync();
  Serial.println("UTC: " + UTC.dateTime());
  GB.setLocation("Europe/London");
  Serial.println("London time: " + GB.dateTime());

}

void sendMQTT() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  Temperature = dht.readTemperature(); // Gets the values of the temperature
  snprintf (msg, 50, "%.1f", Temperature);
  Serial.print("Publish message for t: ");
  Serial.println(msg);
  client.publish("student/CASA0014/plant/ucfnhho/temperature", msg);

  Humidity = dht.readHumidity(); // Gets the values of the humidity
  snprintf (msg, 50, "%.0f", Humidity);
  Serial.print("Publish message for h: ");
  Serial.println(msg);
  client.publish("student/CASA0014/plant/ucfnhho/humidity", msg);

  //Moisture = analogRead(soilPin);   // moisture read by readMoisture function
  snprintf (msg, 50, "%.0i", Moisture);
  Serial.print("Publish message for m: ");
  Serial.println(msg);
  client.publish("student/CASA0014/plant/ucfnhho/moisture", msg);

}



void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);  
    // but actually the LED is on; this is because it is active low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect with clientID, username and password
    if (client.connect(clientId.c_str(), mqttuser, mqttpass)) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe("student/CASA0014/plant/ucfnhho/inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void startWebserver() {
  // when connected and IP address obtained start HTTP server  
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  server.begin();
  Serial.println("HTTP server started");  
}

void handle_OnConnect() {
  Temperature = dht.readTemperature(); // Gets the values of the temperature
  Humidity = dht.readHumidity(); // Gets the values of the humidity
  server.send(200, "text/html", SendHTML(Temperature, Humidity, Moisture));
}

void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML(float Temperaturestat, float Humiditystat, int Moisturestat) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>ESP8266 DHT22 Report</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}\n";
  ptr += "p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<div id=\"webpage\">\n";
  ptr += "<h1>ESP8266 Huzzah DHT22 Report</h1>\n";

  ptr += "<p>Temperature: ";
  ptr += (int)Temperaturestat;
  ptr += " C</p>";
  ptr += "<p>Humidity: ";
  ptr += (int)Humiditystat;
  ptr += "%</p>";
  ptr += "<p>Moisture: ";
  ptr += Moisturestat;
  ptr += "</p>";
  ptr += "<p>Sampled on: ";
  ptr += GB.dateTime("l,");
  ptr += "<br>";
  ptr += GB.dateTime("d-M-y H:i:s T");
  ptr += "</p>";

  ptr += "</div>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
