#include <WiFi.h>
#include <BluetoothSerial.h>
#include "time.h"

const int ledPin = 2; // LED pin
bool ledState = LOW;

BluetoothSerial SerialBT;

void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

void checkAndToggleLED()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }

  // Check if 10 seconds before an even minute
  if (timeinfo.tm_sec == 50 && timeinfo.tm_min % 2 != 0) {
    ledState = !ledState; // Toggle LED state
    digitalWrite(ledPin, ledState); // Update LED
    delay(10000);
    ledState = !ledState; // Toggle LED state
    digitalWrite(ledPin, ledState); // Update LED
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT); // Initialize LED pin as an output
  digitalWrite(ledPin, ledState); // Set initial LED state

  // Connect to WiFi
  const char* ssid       = "Stefan se Seintoring";
  const char* password   = "appels21";
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");

  // Initialize and get the time
  const char* ntpServer = "pool.ntp.org";
  const long  gmtOffset_sec = 7200;
  const int   daylightOffset_sec = 0;
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  // Disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);

  // Initialize Bluetooth
  SerialBT.begin("ESP32"); // Bluetooth device name
  Serial.println("Bluetooth Started. Pair with 'ESP32test'.");
}

void loop()
{
  delay(300); // Check every second
  checkAndToggleLED();

  if (SerialBT.available()) {
    char incomingChar = SerialBT.read();
    if (incomingChar == '1') {
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
      SerialBT.println("LED turned ON");
    } else if (incomingChar == '0') {
      ledState = LOW;
      digitalWrite(ledPin, ledState);
      SerialBT.println("LED turned OFF");
    }
  }
}
