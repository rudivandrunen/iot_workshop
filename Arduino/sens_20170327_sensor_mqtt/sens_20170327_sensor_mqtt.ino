// Version date: 20170327
//
// Get ESP8266 going with Arduino IDE
// - https://github.com/esp8266/Arduino#installing-with-boards-manager
// add http://arduino.esp8266.com/stable/package_esp8266com_index.json
// Required libraries (sketch -> include library -> manage libraries)
// - PubSubClient by Nick ‘O Leary
// - DHT sensor library by Adafruit (version 1.2.3)

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// values to match your local network
#define wifi_ssid ""
#define wifi_password ""

#define mqtt_server ""
#define mqtt_user ""
#define mqtt_password ""
#define mqtt_port 1883

#define humidity_topic "sensor/humidity"
#define temperature_topic "sensor/temperature"
#define ledtopic "sensor/inTopic"

// we use a DHT22 (with pullup) on pin 14 of the HUZZAZ
#define DHTTYPE DHT11
#define DHTPIN  14

WiFiClient espClient;
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
      if (client.connect(macName, mqtt_user, mqtt_password, "sensor/humidity", 1, 1, "9999.99")) {
      Serial.println("connected");
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
