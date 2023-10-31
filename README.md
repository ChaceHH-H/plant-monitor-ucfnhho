# Plant-monitor-ucfnhho
Use ESP8266, DHT22, SH1106-OLED to make a plant monitor that detects plant Temperature-Humidity-Moisture, and upload the data to MQTT. The plant monitor comes with an OLED screen that displays temperature and humidity. Plant monitor can send messages through Telegram Bot to obtain plant data. When the temperature of the plant is too high or the moisture is too low, an email will be sent through IFTTT to remind.
## Change
Based on the workshop, additional functions have been added to send data to email and Telegram, and the use of OLED screens to display data.  
![Image text](https://github.com/ChaceHH-H/Image/blob/main/2776d6a2b9b1b5141828e33d0985b78.jpg)
## Request sensor readings using Telegram
Send a message to the Telegram bot and will receive the sensor data
![Image text](https://github.com/ChaceHH-H/Image/blob/main/cabbc236a81d5f87f4f99fdef912b7b.jpg)  
Download Telegram and create a Telegram bot by searching for BotFather to get the link to access the bot and the bot token  
![Image text](https://i0.wp.com/randomnerdtutorials.com/wp-content/uploads/2020/06/Telegram-Botfather.png?w=362&quality=100&strip=all&ssl=1)  
Use IDBot to get the Telegram user ID (used to make the plant monitor only read messages sent by the specified user  
![Image text](https://i0.wp.com/randomnerdtutorials.com/wp-content/uploads/2020/06/Telegram-ID-Bot.png?w=348&quality=100&strip=all&ssl=1)  
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

## Create email notifications using IFTTT
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
    if(t>30||h<30||Moisture<50){
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

## Display data using SH1106 oled
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
