// Version date: 20170618
//
// D7C1 - free
// 2DA3 - breadboard 
// 
// Get ESP8266 going with Arduino IDE
// - https://github.com/esp8266/Arduino#installing-with-boards-manager
// add http://arduino.esp8266.com/stable/package_esp8266com_index.json
// Required libraries (sketch -> include library -> manage libraries)
// - PubSubClient by Nick ‘O Leary
// - DHT sensor library by Adafruit (version 1.2.3)
//
// u8g2 display LIB, #5=SDA #4=SCK
//

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

U8G2_SH1106_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, 4, 5, U8X8_PIN_NONE);   // All Boards without Reset of the Display

// values to match your local network
#define wifi_ssid "xlexit-guest"
#define wifi_password "AppelTaartje01"

#define mqtt_server "iot.xlexit.com"
#define mqtt_user "rudivd"
#define mqtt_password "rudivd"
#define mqtt_port 1883

#define humidity_topic "sensor/humidity"
#define temperature_topic "sensor/temperature"
#define ledtopic "sensor/inTopic"

//ms to deadman force publish
#define DEADMAN 200000

// we use a DHT22 (with pullup) on pin 11 of the HUZZAZ
#define DHTTYPE DHT22
#define DHTPIN  14

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266

char macName[18];                // the MAC address of your ESPmodule

char h_topic[32];
char t_topic[32];
char i_topic[32];

void setup() {
   
  u8g2.begin();
  u8g2.clearBuffer();          // clear the internal memory

    
  Serial.begin(115200);
  Serial.println("");
  Serial.println("");
  Serial.println(" ------------------------ ");
  Serial.println(" ESP8266 T/H IoT demo RvD ");
  Serial.println(" ------------------------ ");
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  pinMode(BUILTIN_LED, OUTPUT);
  client.setCallback(callback);
}

void setup_wifi() {

byte mac[6];                     // the MAC address of your Wifi shield
  
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  WiFi.macAddress(mac);
  sprintf(macName, "SNOW-%2X%2X%2X%2X%2X%2X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.println("*");
  Serial.println("");
  Serial.print("ClientName: ");
  Serial.println(macName);

  Serial.println("");
  Serial.print("WiFi connect: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  Serial.print("IP address  : ");
  Serial.println(WiFi.localIP());
  Serial.print("Subnet mask : ");
  Serial.println(WiFi.subnetMask());
  Serial.print("Gateway IP  : ");
  Serial.println(WiFi.gatewayIP());
  Serial.print("DNS IP      : ");
  Serial.print(WiFi.dnsIP());
  Serial.print(" - ");
  Serial.println(WiFi.dnsIP(1));
  Serial.println("");

  sprintf(h_topic,"%2X%2X/%s",mac[4], mac[5], humidity_topic);
  sprintf(t_topic,"%2X%2X/%s",mac[4], mac[5], temperature_topic);
  sprintf(i_topic,"%2X%2X/%s",mac[4], mac[5], ledtopic);    
      
  Serial.printf("htopic: %s\n",h_topic);
  Serial.printf("ttopic: %s\n",t_topic);
  Serial.printf("itopic: %s\n",i_topic);

  Serial.println("");
 
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
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    // if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
    // added Last will and Testament changed to unique clientname
      if (client.connect(macName, mqtt_user, mqtt_password, "sensor/humidity", 1, 1, "9999.99")) {
      Serial.println("connected");
      display_connect (0);
//      client.subscribe(ledtopic);
      client.subscribe(i_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      display_connect (client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return !isnan(newValue) &&
         (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}

void notification (char* l) {
      u8g2.setFont(u8g2_font_6x10_tr);
      u8g2.drawStr(120, 63, l); 
      u8g2.sendBuffer();
      u8g2.drawStr(120, 63, " ");
      u8g2.sendBuffer();
}

void display_connect (int connect) {
  u8g2.setFont(u8g2_font_6x10_tr);
  if (connect == 0 ) {
      u8g2.drawStr(2, 63, "MQTT ok         "); 
  } else {
    u8g2.drawStr(2, 63, "MQTT Err ");
    u8g2.drawStr(66, 63, String(connect).c_str());
  }
  u8g2.sendBuffer();
}

long lastMsg = 0;
long lastForceMsg = 0;
bool forceMsg = false;

float temp = 0.0;
float hum = 0.0;
float difft = 0.2;
float diffh = 1.0;

void loop() {

int pos, h;

  
  if (!client.connected()) {
    display_connect(0);
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;

    // MQTT broker could go away and come back at any time
    // so doing a forced publish to make sure something shows up
    // within the first 5 minutes after a reset
    if ( now - lastForceMsg > DEADMAN ) {
      lastForceMsg = now;
      forceMsg = true;
      Serial.println("Forcing publish ...");
    }

    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();

    if (checkBound(newTemp, temp, difft) || forceMsg) {
      temp = newTemp;
      Serial.printf("Publishing to %s New temp: %s \n ", t_topic, String(temp).c_str());
      client.publish(t_topic, String(temp).c_str(), true);

      if (forceMsg) notification ("*"); else notification ("T");
    }

    if (checkBound(newHum, hum, diffh) || forceMsg ) {
      hum = newHum;
      Serial.printf("Publishing to %s New hum : %s \n ", h_topic, String(hum).c_str());
      client.publish(h_topic, String(hum).c_str(), true);

      if (forceMsg) notification ("*"); else notification ("H");
    }

    pos = 10;
    h = pos+10;
    
    u8g2.clearBuffer();          // clear the internal memory
  
    u8g2.setFont(u8g2_font_9x15_te);  // choose a suitable font
    u8g2.drawStr(2,h,"Temp: ");
    u8g2.drawStr(57,h,String(temp).c_str());
    u8g2.drawGlyph(104,h,0x00b0);
    u8g2.drawStr(112,h,"C");
    u8g2.drawStr(2,h+15,"Hum : ");
    u8g2.drawStr(57,h+15,String(hum).c_str());
    u8g2.drawStr(105,h+15,"%");
//    u8g2.sendBuffer();

    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(2, h+35, macName);
    u8g2.sendBuffer();

    forceMsg = false;
  }
}
