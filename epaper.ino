#include <GxEPD.h>
#include <GxGDEP015OC1/GxGDEP015OC1.h>
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>


#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>


GxIO_Class io(SPI, SS/* CS */,  D3/* DC */,  D4/* RST */);
GxEPD_Class display(io);

//Schriftarten einbinden
#include <Adafruit_GFX.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans24pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <Fonts/FreeSansBold24pt7b.h>


#include "bilder.h"
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;



const char* ssid = "DEINE_WLAN_SSID";
const char* password = "DEIN_GEHEIMES_WLAN_PASSWORT";

const char* mqttClient = "DEIN_HOSTNAME";
const char* mqtt_server = "IP_VOM_MQTT_SERVER";
const char* mqttUser = "BENUTZERNAME_FUER_MQTT";
const char* mqttPassword = "PASSWORT_FUER_MQTT";

const char* mqttTopic_RaumTempIST = "Buero/Loxone/RaumTempIST";
const char* mqttTopic_RaumTempSOLL = "Buero/Loxone/RaumTempSOLL";
const char* mqttTopic_RaumHum = "Buero/Loxone/RaumHum";




WiFiClient espClient;
PubSubClient client(espClient);


void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(mqttClient);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("WiFi connected - ESP IP address: ");
  Serial.println(WiFi.localIP());
}


void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if(topic=="NodeRED/Uhrzeit"){
    display.fillRect(60, 138, 140, 62, GxEPD_WHITE);
    display.setFont(&FreeSansBold24pt7b);
    display.setTextSize(1);
    display.setCursor(70, 185);
    display.print(messageTemp);
    display.updateWindow(0, 0, 200, 200, false);    
  }

  if(topic=="NodeRED/KWL_Stufe"){
    display.fillRect(57, 90, 43, 40, GxEPD_WHITE);
    display.setFont(&FreeSansBold24pt7b);
    display.setTextSize(1);
    display.setCursor(57, 126);
    display.print(messageTemp);
    display.updateWindow(0, 0, 200, 200, false);    
  }

  if(topic=="NodeRED/KWL_Bypass"){
    display.fillRect(36, 70, 63, 20, GxEPD_WHITE);

    display.setFont(&FreeSans9pt7b);
    display.setTextSize(1);
    display.setCursor(36, 84);
    if(messageTemp == "bypass"){
      display.print("Bypass");
    }
    else if(messageTemp == "waermetauscher"){
      display.print("Warm");
    }
    display.updateWindow(0, 0, 200, 200, false);    
  }

  if(topic==mqttTopic_RaumTempIST){
    display.fillRect(0, 20, 72, 44, GxEPD_WHITE);
    display.setFont(&FreeSansBold18pt7b);
    display.setTextSize(1);
    display.setCursor(5, 55);
    display.print(messageTemp);
    display.updateWindow(0, 0, 200, 200, false);
  }

  if(topic==mqttTopic_RaumTempSOLL){
    display.fillRect(115, 20, 72, 44, GxEPD_WHITE);
    display.setFont(&FreeSansBold18pt7b);
    display.setTextSize(1);
    display.setCursor(118, 55);
    display.print(messageTemp);
    display.updateWindow(0, 0, 200, 200, false);
  }

  if(topic==mqttTopic_RaumHum){
    //messageTemp += "%";
    display.fillRect(138, 90, 62, 40, GxEPD_WHITE);
    display.setFont(&FreeSansBold24pt7b);
    display.setTextSize(1);
    display.setCursor(138, 126);
    display.print(messageTemp);
    display.updateWindow(0, 0, 200, 200, false);
  }

}


void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect(mqttClient, mqttUser, mqttPassword)) {
      Serial.println("connected");  
      client.subscribe("NodeRED/Uhrzeit");
      client.subscribe("NodeRED/KWL_Stufe");
      client.subscribe("NodeRED/KWL_Bypass");
      client.subscribe(mqttTopic_RaumTempIST);
      client.subscribe(mqttTopic_RaumTempSOLL);
      client.subscribe(mqttTopic_RaumHum);
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  MDNS.begin(mqttClient);
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.klenzel.net/update in your browser\n", mqttClient);
  
  display.init();
  display.fillScreen(GxEPD_WHITE);
  display.update();
  
  display.setRotation(3);
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
  display.fillRect(100, 66, 2, 66, GxEPD_BLACK);
  display.fillRect(0, 66, 200, 2, GxEPD_BLACK);
  display.fillRect(0, 133, 200, 2, GxEPD_BLACK);

  display.drawExampleBitmap(fan, 0, 76, 46 , 49, GxEPD_BLACK);
  display.drawExampleBitmap(uhr, 0, 140, 56 , 56, GxEPD_BLACK);
  display.drawExampleBitmap(temp2, 88, 4, 24 , 56, GxEPD_BLACK);
  display.drawExampleBitmap(hum2, 105, 73, 18 , 56, GxEPD_BLACK);

  display.setFont(&FreeSans9pt7b);
  display.setTextSize(1);
  display.setCursor(30, 17);
  display.print("IST");
  display.setCursor(135, 17);
  display.print("SOLL");
  display.setCursor(138, 84);
  display.print("Luft %");

  display.fillCircle(80, 30, 5, GxEPD_BLACK);  //Xpos,Ypos,r,Farbe
  display.fillCircle(80, 30, 2, GxEPD_WHITE);  //Xpos,Ypos,r,Farbe

  display.fillCircle(193, 30, 5, GxEPD_BLACK);  //Xpos,Ypos,r,Farbe
  display.fillCircle(193, 30, 2, GxEPD_WHITE);  //Xpos,Ypos,r,Farbe

  display.update();
  Serial.println("setup done");

}

void loop() {
  httpServer.handleClient();
  MDNS.update();
  
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect(mqttClient);
} 
