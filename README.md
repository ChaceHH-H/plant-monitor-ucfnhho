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

