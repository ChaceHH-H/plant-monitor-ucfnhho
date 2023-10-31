# Plant-monitor-ucfnhho
Use ESP8266, DHT22, SH1106-OLED to make a plant monitor that detects plant Temperature-Humidity-Moisture, and upload the data to MQTT. The plant monitor comes with an OLED screen that displays temperature and humidity. Plant monitor can send messages through Telegram Bot to obtain plant data. When the temperature of the plant is too high or the moisture is too low, an email will be sent through IFTTT to remind.
## Change
Based on the workshop, additional functions have been added to send data to email and Telegram, and the use of OLED screens to display data.  
![Image text](https://github.com/ChaceHH-H/Image/blob/main/2776d6a2b9b1b5141828e33d0985b78.jpg)
## Request sensor readings using Telegram
Download Telegram and create a Telegram bot by searching for BotFather to get the link to access the bot and the bot token  
![Image text](https://i0.wp.com/randomnerdtutorials.com/wp-content/uploads/2020/06/Telegram-Botfather.png?w=362&quality=100&strip=all&ssl=1)  
Use IDBot to get the Telegram user ID (used to make the plant monitor only read messages sent by the specified user  
![Image text](https://i0.wp.com/randomnerdtutorials.com/wp-content/uploads/2020/06/Telegram-ID-Bot.png?w=348&quality=100&strip=all&ssl=1)  
### Code
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

![Image text](https://github.com/ChaceHH-H/Image/blob/main/cabbc236a81d5f87f4f99fdef912b7b.jpg)  
