// #include <TinyGPSPlus.h>
#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <BluetoothSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include "config.h" // Store API Key and WiFi credentials

// Initialize TinyGPS++ and BluetoothSerial objects
TinyGPSPlus gps;
BluetoothSerial btSerial;

// Set GPS module baud rate
const uint32_t GPSBaud = 9600;

#define LS_DIGIT_G 33
#define LS_DIGIT_CF 32

#define MS_DIGIT0 12
#define MS_DIGIT1 13
#define MS_DIGIT2 14 
#define MS_DIGIT3 27

// Define pins for GPS communication
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17

// WiFi credentials (stored in config.h)
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// HERE API Key (stored in config.h)
const String apiKey = HERE_API_KEY;

// The serial connection to the GPS device
SoftwareSerial ss(GPS_RX_PIN, GPS_TX_PIN);

// Function to convert degrees to cardinal direction
String getCardinalDirection(double heading) {
  const char* directions[] = {"N", "NE", "E", "SE", "S", "SW", "W", "NW"};
  int index = (int)((heading + 22.5) / 45.0) % 8;
  return String(directions[index]);
}

// Function to get speed limit using HERE API
String getSpeedLimit(double lat, double lng) {
    // parameters = {
    //     'at': f'{lat},{lng},{rad}',
    //     'maxResults': '1',
    //     'apiKey': API_KEY,
    //     'showNavAttributes': 'speedLimits',
    //     'types': 'street'
    // }
    // url = F("https://revgeocode.search.hereapi.com/v1/revgeocode?at={lat},{lng},50&maxResult=1&apiKey={API_KEY}&showNavAttributes=speedLimits&types=street");
    String url = "https://www.thunderclient.com/welcome";

  // String url = "https://router.hereapi.com/v8/routes?transportMode=car&origin=" + 
  //              String(lat, 6) + "," + String(lng, 6) + "&apikey=" + apiKey;
  
  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode == 200) {
    String payload = http.getString();
    // Parse the JSON response to get the speed limit (assumes response parsing implemented)
    // For simplicity, return raw payload. You can add a JSON library for more precise parsing.
    http.end();
    return payload; 
  } else {
    http.end();
    return "Error fetching speed limit.";
  }
}

void setup() {
  // Begin Serial communication for debugging
  Serial.begin(115200);
  Serial.println("Initializing...");
  
  ss.begin(GPSBaud);

  // Begin Bluetooth communication
  if (!btSerial.begin("ESP32")) { // Set Bluetooth device name
    Serial.println("Bluetooth initialization failed!");
    while (1); // Stay here in case of Bluetooth failure
  }
  Serial.println("Bluetooth initialized. Waiting for connections...");

  // Begin GPS communication
  Serial.println("GPS module initialized.");

  // Digital Output pin setup and init
  pinMode(LS_DIGIT_G, OUTPUT);
  pinMode(LS_DIGIT_CF, OUTPUT);
  pinMode(MS_DIGIT0, OUTPUT);
  pinMode(MS_DIGIT1, OUTPUT);
  pinMode(MS_DIGIT2, OUTPUT);
  pinMode(MS_DIGIT3, OUTPUT);
  digitalWrite(LS_DIGIT_G, HIGH);
  digitalWrite(LS_DIGIT_CF, LOW);
  digitalWrite(MS_DIGIT0, LOW);
  digitalWrite(MS_DIGIT1, LOW);
  digitalWrite(MS_DIGIT2, LOW);
  digitalWrite(MS_DIGIT3, LOW);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi.");
}

void loop() {
  // Check for GPS data and parse it
  while (ss.available() > 0) {
    gps.encode(ss.read());
  }

  // Check if GPS time and date are valid
  // if (gps.time.isValid() || gps.date.isValid() || gps.location.isValid()) {
    // Get the current minute and second from GPS
    int currentMinute = gps.time.minute();
    int currentSecond = gps.time.second();

    // Send data on even minutes and only once per cycle (close to 0 seconds)
    // if (currentMinute % 2 == 0 && currentSecond == 0) {
    if (currentSecond % 10 == 0) {
      double lat = gps.location.lat();
      double lng = gps.location.lng();
      double heading = gps.course.deg();
      String direction = getCardinalDirection(heading);

      // Get speed limit
      String speedLimit = getSpeedLimit(lat, lng);

      // Prepare the data string as a comma-separated list
      String data = String(gps.date.year()) + "/" + 
                    String(gps.date.month()) + "/" + 
                    String(gps.date.day()) + "," +
                    String(gps.time.hour()) + ":" + 
                    String(gps.time.minute()) + ":" + 
                    String(gps.time.second()) + "," +
                    String(lat, 6) + "," +
                    String(lng, 6) + "," +
                    direction + "," +
                    String(gps.speed.kmph()) + "kmh," + 
                    String(gps.course.age()) + "ms," +
                    String(gps.course.value()) + 
                    speedLimit;

      // Print data to Bluetooth and Serial monitor
      btSerial.println(data);
      Serial.println(data);

      // Wait for a few seconds to avoid duplicate transmissions within the same minute
      delay(3000);
    }
  // } 
  // else {
  //   btSerial.println("GPS data not valid yet...");
  //   Serial.println("GPS data not valid yet...");
  // }
}
