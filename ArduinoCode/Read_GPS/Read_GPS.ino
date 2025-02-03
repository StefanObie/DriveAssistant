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

#define ONBOARD_LED 2

// Define pins for GPS communication
const uint32_t GPSBaud = 9600;
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17

// config.h
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const String apiKey = HERE_API_KEY;

// Global Variables
bool debuggingEnabled = true;
unsigned long lastMillis = 0;
TinyGPSPlus gps;
BluetoothSerial btSerial;

const String jsonResponse_Work = R"({
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

const String jsonResponse_WierdaRoad = R"({
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
                  "&maxResults=" + "1" + 
                  "&apiKey=" + apiKey + 
                  "&showNavAttributes=" + "speedLimits" + 
                  "&types=" + "street";

  return baseUrl + params;
}

int getMaxSpeedForDirection(const String& jsonResponse_WierdaRoad, String& targetDirection) {
  // Parse the JSON response
  StaticJsonDocument<2048> doc;
  DeserializationError error = deserializeJson(doc, jsonResponse_WierdaRoad);

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

  debugPrint("[ERROR] End of getMaxSpeedForDirection reached with no speed limit.");
  return -1;
}

// Function to get speed limit using HERE API
int getSpeedLimit(double lat, double lng, String& direction) {
  // String url = "https://www.thunderclient.com/welcome";
  String url = constructApiUrl(lat,lng) ;

  String debugLine = "[DEBUG] URL: " + url;
  debugPrint(debugLine);

  if (WiFi.status() != WL_CONNECTED) {
    debugPrint("[ERROR] No wifi connection.");
    return -1;
  };

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  debugLine = "[DEBUG] httpReturnCode: " + String(httpCode);
  debugPrint(debugLine);

  if (httpCode != 200) {
    http.end();
    debugPrint("[ERROR] HTTP GET Failed: Could not fetching speed limit.");
    return -1;
  }

  String payload = http.getString();
  // String payload = jsonResponse_WierdaRoad ;
  // String payload = jsonResponse_Work;
  debugPrint(payload);
  http.end();

  int result = getMaxSpeedForDirection(payload, direction);
  debugLine = "[DEBUG] getMaxSpeedForDirection result = " + String(result);
  debugPrint(debugLine);

  if (result == -1) {
    debugPrint("[ERROR] JsonParse: Could not extract the speed limit from the json.");
    return -1;
  }
  return result;
}

void displaySpeedlimit(int num) {
  int mod;
  while (num >= 10) {
    mod = num % 10;
    num /= 10; 
  }

  digitalWrite(MS_DIGIT0, (num & 0x1));
  digitalWrite(MS_DIGIT1, (num & 0x2) >> 1);
  digitalWrite(MS_DIGIT2, (num & 0x4) >> 2);
  digitalWrite(MS_DIGIT3, (num & 0x8) >> 3);

  if (mod == 0) {
    digitalWrite(LS_DIGIT_G, HIGH);
    digitalWrite(LS_DIGIT_CF, LOW);
  } else if (mod == 2) {
    digitalWrite(LS_DIGIT_G, LOW); 
    digitalWrite(LS_DIGIT_CF, HIGH); 
  } else { // Display 8 - Error
    digitalWrite(LS_DIGIT_G, LOW); 
    digitalWrite(LS_DIGIT_CF, LOW); 
  }
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
  // USB Serial Communication
  Serial.begin(115200);
  debugPrint("[SETUP] Initializing...");

  // GPS Communication
  Serial1.begin(GPSBaud, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  debugPrint("[SETUP] GPS module initialized.");

  // Bluetooth Communication
  if (!btSerial.begin("ESP32")) {  // Bluetooth Device Name
    debugPrint("[SETUP] Bluetooth initialization failed!");
    while (1)
      ;  // Stay here in case of Bluetooth failure
  }
  debugPrint("[SETUP] Bluetooth initialized. Waiting for connections...");

  // Digital Output Pin Setup and Initialization
  pinMode(LS_DIGIT_G, OUTPUT);
  pinMode(LS_DIGIT_CF, OUTPUT);
  pinMode(MS_DIGIT0, OUTPUT);
  pinMode(MS_DIGIT1, OUTPUT);
  pinMode(MS_DIGIT2, OUTPUT);
  pinMode(MS_DIGIT3, OUTPUT);
  pinMode(ONBOARD_LED, OUTPUT);
  digitalWrite(LS_DIGIT_G, HIGH);
  digitalWrite(LS_DIGIT_CF, LOW);
  digitalWrite(MS_DIGIT0, LOW);
  digitalWrite(MS_DIGIT1, LOW);
  digitalWrite(MS_DIGIT2, LOW);
  digitalWrite(MS_DIGIT3, LOW);
  digitalWrite(ONBOARD_LED, LOW);

  // WiFi Connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    debugPrint("[SETUP] Connecting to WiFi...");
  }
  debugPrint("[SETUP] Connected to WiFi.");
  debugPrint("[SETUP] Setup Completed.");
}

void loop() {
  checkBluetoothCommands();

  // Check for GPS data and parse it
  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }


  // Check if GPS time and date are valid
  if (gps.time.isValid() && gps.time.isUpdated() && 
      gps.date.isValid() && gps.date.isUpdated() &&
      gps.location.isValid() && gps.location.isUpdated()) {
  
    // Send data just before even minutes
    if (gps.time.minute() % 2 != 0 && gps.time.second() == 50) {
    // if (currentSecond % 30 == 0) {

      // String debugLine = "[DEBUG] " + String(gps.course.age()) + "," + String(gps.date.year()) + "/" + String(gps.date.month()) + "/" + String(gps.date.day()) + "," + String((gps.time.hour() +2) % 24) + ":" + String(gps.time.minute()) + ":" + String(gps.time.second()) + "\t" + String(gps.location.lat(), 6) + ", " + String(gps.location.lng(), 6);
      // debugPrint(debugLine);

      double lat = gps.location.lat();
      double lng = gps.location.lng();
      double heading = gps.course.deg();
      String direction = getCardinalDirection(heading);

      // Get speed limit
      int speedLimit = getSpeedLimit(lat, lng, direction);
      displaySpeedlimit(speedLimit);

      if (speedLimit < gps.speed.kmph()) {
        digitalWrite(ONBOARD_LED, HIGH);
      } else {
        digitalWrite(ONBOARD_LED, LOW);
      }

      // Prepare the data string as a comma-separated list
      String data = String(gps.course.age()) + "," + 
                    String(gps.date.year()) + "/" + 
                    String(gps.date.month()) + "/" + 
                    String(gps.date.day()) + "," + 
                    String((gps.time.hour() +2) % 24) + ":" + 
                    String(gps.time.minute()) + ":" + 
                    String(gps.time.second()) + "," + 
                    String(lat, 6) + "," + 
                    String(lng, 6) + "," + 
                    direction + "," + 
                    String(gps.speed.kmph(), 0) + "," + 
                    String(speedLimit);

      btSerial.println(data);
      Serial.println(data);
      delay(100);
    } 
  } // GPS Data not ready yet.
  else if ((millis() - lastMillis) > 5000) {
    lastMillis = millis(); 
    debugPrint("[DEBUG] GPS time and date are not valid yet.");
    String data = String(gps.date.value()) + "," +
                  String(gps.time.value());
    debugPrint(data);
  }
}
