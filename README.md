# Plant-Monitor-ucfnhho
This repository records the code and method for making a plant monitor using ESP8266, DHT22, SH1106-OLED to detect plant temperature and humidity, and the detected data will be automatically uploaded to the mqtt server. The plant monitor comes with an OLED screen that displays temperature and humidity. At the same time, when the temperature in the factory is too high or the humidity is too low, the monitor will send an email through IFTTT to alert the plant owner. In addition, plant owners can also send messages through Telegram Bot to obtain plant data.  
The project is divided into three parts：

1. Design idea
2. Develop and build
3. Placed in CE lab
## Plant
The plant detected by the monitor is Parlour Palm (or Chamaedorea elegans）, a small palm native to Central America. It is a temperate woody plant that requires an average temperature of 20 to 38°C to grow. Although this palm can tolerate dry indoor air, it will be healthier with a relative humidity of 40-60%. Parlor Palms are sensitive to overwatering and cannot tolerate being waterlogged or sitting in a saturated potting mix  

![Image text](https://github.com/ChaceHH-H/Image/blob/main/place2.jpg) 
## Design idea
The basic function of the factory monitor is to make it according to the standard workshop process, detect data and transmit it to the MQTT server. On this basis, five additional functions are proposed.

- Use a buzzer to remind when the value is below a certain threshold
- Use LED to remind when the value is lower than a certain threshold
- Add a screen to the plant monitor to display detected data
- Get plant data via Telegram bot
- Automatically send warning email

The first two ideas are easier to implement than the latter, but the operation of these two requires people to be present to remind people, and may operate at inappropriate times.As for email reminders and telegrams, plant owners can receive data and warnings no matter where they are, which is more convenient and real-time than buzzers fixed on plant monitors. Adding a screen can also provide people with options other than mobile phones and the Internet to observe plant data.


## Develop and build
Equipment needed:
- Adafruit Huzzah ESP8266 WiFi board
- DHT22 Digital Sensor
- Raspberry Pi
- SH1106-OLED Display
- 2x 10k Ohm resistors
- 1x 200 Ohm resistor
- 1x 100 Ohm resistor
- two nails
- four cables

### How to build the plant monitor
First need to connect the DHT22 sensor and two nails to the ESP8266 WiFi board. We use a Huzzah shield and solder two 10k Ohm resistors, a 200 Ohm resistor, a 100 Ohm resistor and the DHT22 sensor to the Huzzah shield so no wires are needed. In addition, we use open wires to weld two nails to the Huzzah shield, and finally weld the Huzzah shield to the ESP8266 WiFi board. We have completed the construction of the plant sensor.
![Image text](https://github.com/ChaceHH-H/Image/blob/main/e38e251e2e837937.png) 

### Code for Data detection and transmission
Plant Monitor uses multiple libraries for wifi connection, transmitting data to mqtt, etc.  

```
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
```

First define the pins of the sensor and nail,also the variables needed, then call setup to initialize the pins.  

```
uint8_t DHTPin = 12;
uint8_t soilPin = 0;
float Temperature;
float Humidity;
int Moisture = 1;
int sensorVCC = 13;
int blueLED = 2;
DHT dht(DHTPin, DHTTYPE);
#include "arduino_secrets.h" 
const char* ssid     = SECRET_SSID;
const char* password = SECRET_PASS;
const char* mqttuser = SECRET_MQTTUSER;
const char* mqttpass = SECRET_MQTTPASS;

```

Then Start the WiFi and MQTT networks functions to Set variables for Wifi and MQTT connections

```  
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
```

Finally, call the readMoisture() function to open the soil sensor, add voltage to it to detect and obtain data, and then use the data and DHT22 data in the sendMQTT() function to push to the CASA MQTT server.

```
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
```

![Image text](https://github.com/ChaceHH-H/Image/blob/main/mqtt.png) 

### Setting up the Raspberry Pi
Install a Raspberry Pi to acquire Plant Monitor data and visualize the data using Telegraf, InfluxDB and Grafana.
![Image text](https://github.com/ChaceHH-H/Image/blob/main/b9e8c2c4761ee81cb10846f4a65a1c9.png) 

### Code for Telegram bot message
Send a message to the Telegram bot and will receive the sensor data

Download Telegram and create a Telegram bot by searching for BotFather to get the link to access the bot and the bot token  
![Image text](https://github.com/ChaceHH-H/Image/blob/main/ddd1bf09e1a49f0c9e9728c22582abc.jpg) 
Use IDBot to get the Telegram user ID (used to make the plant monitor only read messages sent by the specified user  

Add Telegram bot and ArduinoJson library  
Insert user ID and Bot Token  
```
#define CHAT_ID "My-ID"
#define BOTtoken "Bot Token"
```
Build a WiFi client and use the token to create a robot  

```
WiFiClientSecure teleclient;
UniversalTelegramBot bot(BOTtoken, teleclient);
```

The getReadings() function requests three data from the sensor and returns the result as a string

```
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
```

The handleNewMessages() function will process new messages, check whether the id of the sender of the message is correct, if it is incorrect, ignore the message, if the id is correct, identify the message sent, and edit the content according to different messages.

```
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
```

In loop(), check for new messages every second，When new messages are received, call the handleNewMessages() function.

```
if (millis() > lastTimeBotRan + botRequestDelay)  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);

    while(numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
```

![Image text](https://github.com/ChaceHH-H/Image/blob/main/b6ac854731c6fcc887e491bee9e16e7.jpg) 
### Code for Send email via IFTTT
Plant Monitor sends email alerts when sensor data exceeds normal values
![Image text](https://github.com/ChaceHH-H/Image/blob/main/4473b1d861ba45fcd15dce3146ad3a7.jpg)  
Create an event in IFTTT
![Image text](https://github.com/ChaceHH-H/Image/blob/main/73f7c22f0ae2355ad3f06dc0261fa4c.png) 
Insert IFTTT API key and host

```
const char* host = "maker.ifttt.com";
const char* apiKey = "IFTTT-Key";
```

Check whether the sensor data is in the normal range in loop()，if the data exceeds the normal value, call sendemail() to send an email. Use millis to have the program check every half hour

```
if(millis() >= time_now + period){
    if(t>38||t<20||h<60||h>40){
    time_now += period;
    sendemail(); }
  }
```

Last, sendemail() make a request to IFTTT and output three values to email

```
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
```

### Code for Display data using SH1106 oled
Connect the oled to the four pins of ESP8266 (GND, 3V, SCL, SDA)
![Image text](https://github.com/ChaceHH-H/Image/blob/main/f7b19fc3c5a75c9397f8a53e57ff7c2.jpg) 
First Change Dn to Arduino pin number, otherwise the oled cannot be read.

```
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
```

Initialize the OLED display using Wire library

```
SH1106Wire display(0x3c, D2, D1);
```

Clear the screen and call displayTempHumid in the loop

```
display.clear();
displayTempHumid();
```

Finally, create the displayTempHumid() function, read the sensor data and call the image in Graphic_esp8266_dht22_oledi2c.h and display it

```
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
```
## Placed in CE lab
