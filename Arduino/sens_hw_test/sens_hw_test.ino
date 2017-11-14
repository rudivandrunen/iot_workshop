
#include <ESP8266WiFi.h>
#include <DHT.h>

#define wifi_ssid ""
#define wifi_password ""

#define DHTTYPE DHT11
#define DHTPIN  14

WiFiClient espClient;
DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266
char macName[18];                // the MAC address of your ESPmodule

void setup() {
  Serial.begin(115200);
  Serial.println("");
  Serial.println("");
  Serial.println(" ------------------------ ");
  Serial.println(" ESP8266 T/H IoT demo RvD ");
  Serial.println(" ------------------------ ");
  dht.begin();
  setup_wifi();
  pinMode(BUILTIN_LED, OUTPUT);

  connecttest();
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
}

void connecttest() {
  const char* host = "www.google.com";
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
    client.print(String("GET ") + "www.google.com/index.html" + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
}


void loop() {
  // put your main code here, to run repeatedly:
    
    float temp;
    float hum;

     temp = dht.readTemperature();
     hum = dht.readHumidity();
     digitalWrite(BUILTIN_LED, HIGH);
     Serial.print("Temperature:");
     Serial.println(String(temp).c_str());
     delay (5000);
     Serial.print("Humidity:");
     Serial.println(String(hum).c_str());     
     digitalWrite(BUILTIN_LED, LOW);
     delay (5000);

}
