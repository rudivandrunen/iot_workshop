// Version date: 20170913a 
// tls setup test using server cert and checks fingerprint.
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

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <DHT.h>

#define wifi_ssid "xlexit-guest"
#define wifi_password "AppelTaartje01"

#define mqtt_server "iot.xlexit.com"
#define mqtt_user "rudivd"
#define mqtt_password "rudivd"
#define mqtt_port 1883
#define mqtt_port_tls 8883

#define humidity_topic "sensor/humidity"
#define temperature_topic "sensor/temperature"
#define ledtopic "sensor/inTopic"

// we use a DHT22 (with pullup) on pin 11 of the HUZZAZ
#define DHTTYPE DHT22
#define DHTPIN  14

const char* fingerprint = "14 91 35 5A 0E 18 06 FD 3E 85 4B 59 66 48 42 C2 CD 35 0D 76";

WiFiClientSecure espClient;
PubSubClient client(espClient);

DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266

char macName[18];                // the MAC address of your ESPmodule

char h_topic[32];
char t_topic[32];
char i_topic[32];

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("");
  Serial.println(" ------------------------ ");
  Serial.println(" ESP8266 T/H IoT demo RvD ");
  Serial.println(" ------------------------ ");
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port_tls);
  pinMode(BUILTIN_LED, OUTPUT);
  client.setCallback(callback);
}


// https://gist.github.com/chaeplin/3223074601733fa46d4a



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

if (espClient.verify(fingerprint, mqtt_server)) {
    Serial.println("certificate matches");
  } else {
    Serial.println("certificate doesn't match");
  }
      
//      client.subscribe(ledtopic);
      client.subscribe(i_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
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

long lastMsg = 0;
long lastForceMsg = 0;
bool forceMsg = false;

float temp = 0.0;
float hum = 0.0;
float difft = 0.2;
float diffh = 1.0;

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;

    // MQTT broker could go away and come back at any time
    // so doing a forced publish to make sure something shows up
    // within the first 5 minutes after a reset
    if (now - lastForceMsg > 200000) {
      lastForceMsg = now;
      forceMsg = true;
      Serial.println("Forcing publish every 200 sec...");
    }

    float newTemp = dht.readTemperature();
    float newHum = dht.readHumidity();

    if (checkBound(newTemp, temp, difft) || forceMsg) {
      temp = newTemp;
      Serial.printf("Publishing to %s New temp: %s \n ", t_topic, String(temp).c_str());
      client.publish(t_topic, String(temp).c_str(), true);
    }

    if (checkBound(newHum, hum, diffh) || forceMsg ) {
      hum = newHum;
      Serial.printf("Publishing to %s New hum : %s \n ", h_topic, String(hum).c_str());
      client.publish(h_topic, String(hum).c_str(), true);
    }

    forceMsg = false;
  }
}
