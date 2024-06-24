#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>
#define DHTPIN 4
// #define LED 16
// #define LED2 5
String str;
const char* ssid = "DGESRASs";
const char* password = "dockatt2237";
// const char* ssid = "TP-LINK_BEFE";
// const char* password = "77763501";
// const char* ssid = "SK Pixel 6";
// const char* password = "12345678";
bool requestSent = false;
String dateTimeString;
File dataFile;
void setup() {
  Serial.begin(115200);
  Serial1.begin(115200);
  while (!Serial) {
    ;
  }
  delay(10);
  if (!SD.begin(SS)) {
    Serial.println("Card failed, or not present");
    return;
  }
  Serial.println("Card initialized.");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}
void SaveData(int rfid) {
    dataFile = SD.open("RFIDData.txt", FILE_WRITE);
    if (dataFile) {
      String combinedString = String(rfid) + "      " + dateTimeString;
      dataFile.println(combinedString);
      Serial.println(combinedString);
      dataFile.close();
      Serial.println("RFID code saved to SD card.");
    } else {
      Serial.println("Error opening RFIDData.txt for writing.");
    }
  }
void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    Serial.println(input);
    int spaceIndex = input.indexOf(' ');
    if (spaceIndex != -1) {
      String rfidString = input.substring(0, spaceIndex);
      int rfid = rfidString.toInt();
      dateTimeString = input.substring(spaceIndex + 1);
      Serial.print("RFID code: ");
      Serial.println(rfid);
      Serial.print("Date/time: ");
      Serial.println(dateTimeString);
      SaveData(rfid);
     // deleteRecord(); // API sed and delete recode
    }
     else
     {
     Serial.print("Triggered");
     deleteRecord();
     }
  }
}
bool sendData(int rfid, String dt) {
  HTTPClient http;
  String apiURL = "https://esystems.cdl.lk/backend-Test/RFIDAttendanceWithMemory/RFID/postRFID?RFIDCode=" + String(rfid) + "&Device=2&DeviceDT=" + String(dt) + "&InOut=O";  // for esystem
  WiFiClientSecure client;                                                                                                                                                   // for esystem
  client.setInsecure();                                                                                                                                                      // for esystem
  http.begin(client, apiURL);
  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println("HTTP response: " + payload);
    return true;  // Return true if data was successfully sent
  } else {
    Serial.print("HTTP request failed with error: ");
    Serial.println(http.errorToString(httpCode).c_str());
    return false;  // Return false if data failed to send
  }
}
void deleteRecord() {
    File originalFile = SD.open("RFIDData.txt", FILE_READ);
  if (!originalFile) {
    Serial.println("Error opening RFIDData.txt for reading");
    return;
  }
  File tempFile = SD.open("temp.txt", FILE_WRITE);
  if (!tempFile) {
    Serial.println("Error creating temporary file");
    originalFile.close();
    return;
  }
while (originalFile.available()) {
    String line = originalFile.readStringUntil('\n');
    if (!line.isEmpty()) {
        // Split the line based on whitespace
        int spaceIndex = line.indexOf(' ');
        String rfidStr = line.substring(0, spaceIndex); // Extract RFID
        rfidStr.trim();
       // String dt = line.substring(spaceIndex + 1); // Extract timestamp
       String dt = line.substring(spaceIndex + 1); // Extract timestamp
       dt.trim();
        int rfid = rfidStr.toInt();
          Serial.println(rfidStr);
          Serial.println(dt);
        if (!sendData(rfid, dt)) {
            tempFile.println(line);
        }
    }
}
  originalFile.close();
  tempFile.close();
  // Remove original file
  SD.remove("RFIDData.txt");
  // Rename temp file to original file name
  SD.rename("temp.txt", "RFIDData.txt");
}
[3:48 PM] DTS IOT
#include <Arduino.h>
#include "HidProxWiegand.h"
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Wire.h>
#include <TwiLiquidCrystal.h>
#include <RTClib.h>
#define PIN_DATA0 2
#define PIN_DATA1 3
#define LED_PINR 7
#define buzzer 8
#define LED_PING 9
SoftwareSerial espSerial(5, 6);
const int chipSelect = 10;
const uint8_t LCD_ADDRESS = 0x27;
const uint8_t LCD_ROWS = 4;
const uint8_t LCD_COLS = 16;
unsigned long startMillis;
unsigned long currentMillis;
const unsigned long interval = 1000;
bool isBuzzerOn = false;
unsigned long displayTimer = 0;
unsigned long BuzzerTimer = 0;
unsigned long lastCardReadTime = 0;
const unsigned long displayInterval = 2000;
const unsigned long BuzzerInterval = 1000;
unsigned long RFID_Data;
bool isDisplayOn = false;
char server[] =  "10.0.13.48";
//bool NoonTriggered  = false;
IPAddress ip(172, 30, 30, 118);
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetClient client;
TwiLiquidCrystal lcd(LCD_ADDRESS);
String dbTime;
RTC_DS3231 rtc;
String str;
ProxReaderInfo* reader1;
void cardReadHandler(ProxReaderInfo* reader) {
  DateTime now = rtc.now();
  now = now + TimeSpan(0, 0, 0, 24);
  char db[] = "DD-MMM-YYYY"
              ","
              "hh:mm:ss";
  dbTime = now.toString(db);
  lcd.backlight();
  lcd.setCursor(4, 3); //(Display Adjesment)
  if (String(reader->facilityCode, DEC).length() == 2) {
    lcd.print("0" + String(reader->facilityCode) + ":" + String(reader->cardCode));
    RFID_Data = (reader->cardCode);
    //SupportAPI();
    Serial.println(String(RFID_Data) + " " + dbTime);
    //espSerial.println(RFID_Data);
   // espSerial.println(dbTime);
   // espSerial.println(String(RFID_Data) + " " + dbTime);
    Sending_To_phpmyadmindatabase();
  } else {
    lcd.print(String(reader->facilityCode) + ":" + String(reader->cardCode));
    RFID_Data = (reader->cardCode);
    //SupportAPI();
    Serial.println(String(RFID_Data) + " " + dbTime);
    //espSerial.println(RFID_Data);
    //espSerial.println(dbTime);
    Sending_To_phpmyadmindatabase();
  }
  displayTimer = millis();
  isDisplayOn = true;
  BuzzerTimer = millis();
  digitalWrite(buzzer, HIGH);
}
void UPtime() {
  DateTime now = rtc.now();
  if (now.hour() == 15 && now.minute() == 10 && now.second() == 1) {   /////////// one time
   Serial.println("Send to ESP OK");
   espSerial.println("OK");
  }
  now = now + TimeSpan(0, 0, 0, 24);
  char dateBuffer[] = "DD-MMM-YYYY DDD";
  char timeBuffer[] = "hh:mm:ss";
  char db[] = "DD-MMM-YYYY"
              ","
              "hh:mm:ss";
  lcd.setCursor(6, 0);
  lcd.print("WELCOME !");
  lcd.setCursor(0, 1);
  lcd.print(now.toString(dateBuffer));
  lcd.setCursor(4, 2);
  lcd.print(now.toString(timeBuffer));
}
void handleInterrupt0() {
  if (reader1 != NULL) {
    reader1->ISR_Data0();
  }
}
void handleInterrupt1() {
  if (reader1 != NULL) {
    reader1->ISR_Data1();
  }
}
void setup() {
  Serial.begin(115200);
  pinMode(LED_PING, OUTPUT);
  pinMode(LED_PINR, OUTPUT);
  pinMode(buzzer, OUTPUT);
  lcd.begin(LCD_COLS, LCD_ROWS);
  lcd.backlight();
  lcd.clear();
  startMillis = millis();
  while (!Serial) {
    delay(10);
  }
  espSerial.begin(115200);
  while (!Serial) {
    delay(10);}
  reader1 = HidProxWiegand.addReader(PIN_DATA0, PIN_DATA1, cardReadHandler);
  HidProxWiegand_AttachReaderInterrupts(PIN_DATA0, PIN_DATA1, handleInterrupt0, handleInterrupt1);
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Ethernet.begin(mac, ip);
    digitalWrite(LED_PINR, HIGH);
  }
  digitalWrite(LED_PING, HIGH);
  delay(1000);
  rtc.begin();
}
void loop() {
  HidProxWiegand.loop();
  unsigned long currentMillis = millis();
  static unsigned long lastUpdateTime = 0;
  const unsigned long updateTimeInterval = 1000;  // Update time every 1 second
  if (currentMillis - lastUpdateTime >= updateTimeInterval) {
    UPtime();
    lastUpdateTime = currentMillis;
  }
  if (isDisplayOn && (millis() - displayTimer >= displayInterval)) {
    lcd.setCursor(4, 3);
    lcd.print("                         ");
    isDisplayOn = false;
  }
    if (BuzzerTimer && (millis() - displayTimer >= BuzzerInterval)) {
        digitalWrite(buzzer, LOW);
       isBuzzerOn = false;
  }
}
void Sending_To_phpmyadmindatabase() {
  if (client.connect(server, 80)) {
    digitalWrite(LED_PINR, LOW);
    digitalWrite(LED_PING, HIGH);
    Serial.println("connected");
    Serial.print("GET /RFID/postRFID?Device=2&InOut=I&RFIDCode=");
    client.print("GET /RFID/postRFID?Device=2&InOut=I&RFIDCode=");
    // Serial.print("GET /RFID/postRFID?Device=8&InOut=O&RFIDCode=");
    // client.print("GET /RFID/postRFID?Device=8&InOut=O&RFIDCode=");
    client.print(RFID_Data);
    Serial.println(RFID_Data);
    Serial.println(dbTime);
    client.print(" ");  // SPACE BEFORE HTTP/1.1
    client.print("HTTP/1.1");
    client.println();
    //client.println("Host: 172.29.10.103");  // Backend PC
    client.println("Host: 10.0.13.48");  // Backend PC
    //client.println("Host: 192.168.1.48");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("connection failed");
    lcd.backlight();
    lcd.setCursor(4, 3);
    lcd.print("Connection Failed");
    espSerial.println(String(RFID_Data) + " " + dbTime);
    digitalWrite(LED_PINR, HIGH);
    digitalWrite(LED_PING, LOW);
    displayTimer = millis();
    isDisplayOn = true;
  }
}
