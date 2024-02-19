#include <FastBot.h>

#include <ESP8266WiFi.h>

#define WIFI_SSID ""
#define WIFI_PASS ""
#define BOT_TOKEN ""

#define TEMPERATURE "/temperature"
#define HUMIDITY "/humidity"
#define PRESSURE "/pressure"
#define START "/start"

#define INCORRECT_VALUE "0.00"
#define UNITS (String[]){"C","mm","%"}

bool isConnected=false;

byte tries = 10;

FastBot bot(BOT_TOKEN);

void setup() {
  Serial.begin(115200);
  isConnected=connectToWifi();
  bot.attach(newMsg);
}

void loop() {
  bot.tick();
}

void newMsg(FB_msg& msg) {
  if (msg.text==START) {
    showMenu(msg);

    return;
  }

  if (msg.text==TEMPERATURE) {
    String value=getValue(TEMPERATURE);

    if (value!=INCORRECT_VALUE) {
      replyMsg("Last temperature: "+value+UNITS[0], msg);
    } else {
      replyCheck(msg);
    }

    return;
  }

  if (msg.text==HUMIDITY) {
    String value=getValue(HUMIDITY);

    if (value!=INCORRECT_VALUE) {
      replyMsg("Last humidity: "+value+UNITS[2], msg);
    } else {
      replyCheck(msg);
    }
      
    return;
  }

  if (msg.text==PRESSURE) {
    String value=getValue(PRESSURE);

    if (value!=INCORRECT_VALUE) {
      replyMsg("Pressure: "+value+UNITS[1], msg);
    } else {
      replyCheck(msg);
    }

    return;
  }

  deleteMsg(msg);
}

void printSerial(String com) {
  waitForWriteSerial();

  Serial.print(com);
}

String getValue(String com) {
  printSerial(com);

  return readStringSerial();
}

String readStringSerial() {
  waitAvailableSerial();

  return Serial.readString();
}
void waitForWriteSerial() {
  while (Serial.availableForWrite() == 0);
}

void waitAvailableSerial() {
  while (Serial.available() == 0);
}

void deleteMsg(FB_msg msg) {
    bot.deleteMessage(msg.messageID, msg.chatID);
}

void showMenu(FB_msg msg) {
  bot.showMenu(String(TEMPERATURE)+"\n"+String(HUMIDITY)+"\n"+String(PRESSURE)+"\n", msg.chatID);
}

void replyCheck(FB_msg msg) {
  replyMsg("Check BME280!!!", msg);
}

void replyOk(FB_msg msg) {
  replyMsg("Work!!!", msg);
}

void replyMsg(String reply, FB_msg msg) {
  bot.replyMessage(reply, msg.messageID, msg.chatID);
}

bool connectToWifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (--tries && WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  return WiFi.status() == WL_CONNECTED;
}
