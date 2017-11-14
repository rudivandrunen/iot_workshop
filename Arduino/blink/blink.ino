// blink.ino

#include <ESP8266WiFi.h>

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
}
 
void loop() {
  digitalWrite(BUILTIN_LED, HIGH);
    // Turn the LED off by making the voltage HIGH
  delay(500);
  digitalWrite(BUILTIN_LED, LOW);
    // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  delay(500);
}
