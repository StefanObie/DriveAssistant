#include <WiFi.h>
#include <HTTPClient.h>

// Replace with your network credentials
const char* ssid = "";
const char* password = "";

// Replace with the URL of the server you want to request
const char* serverURL = "";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  // Wait for WiFi connection
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) { // Check WiFi connection status
    HTTPClient http;

    Serial.println("Sending HTTP GET request...");
    http.begin(serverURL); // Specify the URL
    int httpResponseCode = http.GET(); // Send the GET request

    if (httpResponseCode > 0) { // Check for the returning code
      String payload = http.getString();
      Serial.println("HTTP Response Code: " + String(httpResponseCode));
      Serial.println("Response Payload: " + payload);
    } else {
      Serial.println("Error on sending request: " + String(httpResponseCode));
    }

    http.end(); // Free resources
  } else {
    Serial.println("WiFi not connected!");
  }

  delay(10000); // Send a request every
}
