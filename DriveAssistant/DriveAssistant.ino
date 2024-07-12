#include <WiFi.h>
#include "time.h"

const char* ssid       = "Stefan se Seintoring";
const char* password   = "appels21";

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 7200;
const int   daylightOffset_sec = 0;

const int ledPin = 2; // LED pin
bool ledState = LOW;

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

  // Print current time
  // Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

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
  Serial.printf("Connecting to %s ", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  Serial.println(" CONNECTED");

  // Initialize and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();

  // Disconnect WiFi as it's no longer needed
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
}

void loop()
{
  delay(300); // Check every second
  checkAndToggleLED();
}
