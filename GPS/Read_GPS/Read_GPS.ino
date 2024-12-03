#include <TinyGPS++.h>
#include <BluetoothSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"

// Digit Pin
#define LS_DIGIT_G 33
#define LS_DIGIT_CF 32

#define MS_DIGIT0 12
#define MS_DIGIT1 13
#define MS_DIGIT2 14
#define MS_DIGIT3 27

// Define pins for GPS communication
const uint32_t GPSBaud = 9600;
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17

// config.h
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const String apiKey = HERE_API_KEY;
bool debuggingEnabled = true;

TinyGPSPlus gps;
BluetoothSerial btSerial;

const String jsonResponseWork = R"({
    "items": [
        {
            "title": "Sonae House, Sandton, 2191, South Africa",
            "id": "here:af:streetsection:3vZcFpKVTtbYw0MuaE24KD",
            "resultType": "street",
            "address": {
                "label": "Sonae House, Sandton, 2191, South Africa",
                "countryCode": "ZAF",
                "countryName": "South Africa",
                "state": "Gauteng",
                "county": "Johannesburg",
                "city": "Sandton",
                "district": "The Woodlands",
                "street": "Sonae House",
                "postalCode": "2191"
            },
            "position": {
                "lat": -26.05849,
                "lng": 28.08845
            },
            "distance": 100,
            "mapView": {
                "west": 28.08845,
                "south": -26.05849,
                "east": 28.0892,
                "north": -26.05839
            },
            "navigationAttributes": {
                "speedLimits": [
                    {
                        "maxSpeed": 20,
                        "direction": "W",
                        "speedUnit": "kph",
                        "source": "derived"
                    },
                    {
                        "maxSpeed": 20,
                        "direction": "E",
                        "speedUnit": "kph",
                        "source": "derived"
                    }
                ]
            }
        }
    ]
})";

const String jsonResponse = R"({
        "items": [
            {
                "title": "Wierda Rd, Tshwane, 0137, South Africa",
                "navigationAttributes": {
                    "speedLimits": [
                        {
                            "maxSpeed": 60,
                            "direction": "NE",
                            "speedUnit": "kph"
                        },
                        {
                            "maxSpeed": 60,
                            "direction": "SW",
                            "speedUnit": "kph"
                        }
                    ]
                }
            }
        ]
    })";

void debugPrint(const String& debugLine) {
  if (debuggingEnabled) {
    Serial.println(debugLine);
    btSerial.println(debugLine);
  }
}

// Function to convert degrees to cardinal direction
String getCardinalDirection(double heading) {
  const char* directions[] = { "N", "NE", "E", "SE", "S", "SW", "W", "NW" };
  int index = (int)((heading + 22.5) / 45.0) % 8;
  return String(directions[index]);
}

String constructApiUrl(double lat, double lng) {
  const String baseUrl = "https://revgeocode.search.hereapi.com/v1/revgeocode";

  String params = "?at=" + String(lat, 6) + "," + String(lng, 6) + ",50" +  //at={lat},{lng},{radius=50}
                  "&maxResults=" + "1" + "&apiKey=" + apiKey + "&showNavAttributes=" + "speedLimits" + "&types=" + "street";

  return baseUrl + params;
}

int getMaxSpeedForDirection(const String& jsonResponse, String& targetDirection) {
  // Parse the JSON response
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, jsonResponse);

  if (error) {
    String debugLine = "[ERROR] JSON deserialization failed: " + String(error.c_str());
    debugPrint(debugLine) ;
    return -1;
  }

  // Navigate through the JSON structure
  JsonArray items = doc["items"];
  if (items.isNull()) {
    debugPrint("[ERROR] No items found in JSON response.");
    return -1;
  }

  for (JsonObject item : items) {
    JsonArray speedLimits = item["navigationAttributes"]["speedLimits"];
    if (speedLimits.isNull()) {
      debugPrint("[ERROR] No speed limits found.");
      return -1;
    }

    // Iterate through speed limits
    int fallbackSpeed = 999;
    for (JsonObject speedLimit : speedLimits) {
      String direction = speedLimit["direction"].as<String>();
      int maxSpeed = speedLimit["maxSpeed"].as<int>();

      String debugLine = "[DEBUG] Direction: " + direction + ", maxSpeed: " + String(maxSpeed);
      debugPrint(debugLine);

      if (fallbackSpeed > maxSpeed) {
        //Fallback speed is the min of all speed limits found
        fallbackSpeed = maxSpeed;
      }

      if (direction == targetDirection) {
        return maxSpeed;  // Return the maxSpeed for the matching direction
      }
    }
    String debugLine = "[DEBUG] No matching direction found, using fallback speed: " + String(fallbackSpeed) + " km/h.";
    debugPrint(debugLine);

    return fallbackSpeed;
  }

  debugPrint("[ERROR] End reached with no speed limit, getMaxSpeedForDirection.");
  return -1;
}

// Function to get speed limit using HERE API
String getSpeedLimit(double lat, double lng, String& direction) {
  // String url = "https://www.thunderclient.com/welcome";
  String url = constructApiUrl(lat,lng) ;

  String debugLine = "[DEBUG] URL: " + url;
  debugPrint(debugLine);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    debugLine = "[DEBUG] httpCode: " + String(httpCode);
    debugPrint(debugLine);

    if (httpCode == 200) {
      String payload = http.getString();
      // String payload = jsonResponse ;
      // String payload = jsonResponseWork;
      debugPrint(payload);
      http.end();

      int result = getMaxSpeedForDirection(payload, direction);
      debugLine = "[DEBUG] getMaxSpeedForDirection Result = " + String(result);
      debugPrint(debugLine);

      if (result != -1) {
        return String(result);
      }

      debugPrint("[ERROR] JsonParse: Could not extract the speed limit from the json.");

    } else {
      http.end();
      debugPrint("[ERROR] HTTP GET Failed: Could not fetching speed limit.");
    }
  } else {
    debugPrint("[ERROR] No wifi connection.");
  }

  return "ERROR";
}

void checkBluetoothCommands() {
  if (btSerial.available()) {
    String command = btSerial.readStringUntil('\n');
    command.trim();

    if (command.equalsIgnoreCase("debugOn")) {
      debuggingEnabled = true;
      debugPrint("[INFO] Debugging enabled.");
    } else if (command.equalsIgnoreCase("debugOff")) {
      debugPrint("[INFO] Debugging disabled.");
      debuggingEnabled = false;
    } else {
      debugPrint("[WARN] Unknown command: " + command);
    }
  }
}

void setup() {
  // Begin Serial communication for debugging
  Serial.begin(115200);
  debugPrint("Initializing...");

  // Begin GPS communication
  Serial1.begin(GPSBaud, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  debugPrint("GPS module initialized.");

  // Begin Bluetooth communication
  if (!btSerial.begin("ESP32")) {  // Set Bluetooth device name
    debugPrint("Bluetooth initialization failed!");
    while (1)
      ;  // Stay here in case of Bluetooth failure
  }
  debugPrint("Bluetooth initialized. Waiting for connections...");

  // Begin GPS communication
  debugPrint("GPS module initialized.");

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
    debugPrint("Connecting to WiFi...");
  }
  debugPrint("Connected to WiFi.");
  debugPrint("[DEBUG] Setup Completed.");
}

void loop() {
  checkBluetoothCommands();

  // Check for GPS data and parse it
  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }

  // Check if GPS time and date are valid
  // if (gps.time.isValid() || gps.date.isValid() || gps.location.isValid()) {
  if (gps.time.isValid() && gps.time.isUpdated() && 
      gps.date.isValid() && gps.date.isUpdated() &&
      gps.location.isValid() && gps.location.isUpdated()) {

    int currentMinute = gps.time.minute();
    int currentSecond = gps.time.second();

    // Send data on even minutes
    if (currentMinute % 2 == 0 && currentSecond == 0) {
    // if (currentSecond % 30 == 0) {
      double lat = gps.location.lat();
      double lng = gps.location.lng();
      double heading = gps.course.deg();
      String direction = getCardinalDirection(heading);

      // Get speed limit
      String speedLimit = getSpeedLimit(lat, lng, direction);

      // Prepare the data string as a comma-separated list
      String data = String(gps.course.age()) + "," + 
                    String(gps.date.year()) + "/" + 
                    String(gps.date.month()) + "/" + 
                    String(gps.date.day()) + "," + 
                    String(gps.time.hour()) + ":" + 
                    String(gps.time.minute()) + ":" + 
                    String(gps.time.second()) + "," + 
                    String(lat, 6) + "," + 
                    String(lng, 6) + "," + 
                    direction + "," + 
                    String(gps.speed.kmph()) + "," + 
                    speedLimit;

      btSerial.println(data);
      Serial.println(data);
      delay(100);
    }
  }
}
