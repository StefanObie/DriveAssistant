#include <TinyGPS++.h>
#include <WiFi.h>
#include <HTTPClient.h>
// #include <ArduinoJson.h>
#include <Regexp.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

// Define pins for GPS communication
const uint32_t GPSBaud = 9600;
#define GPS_LOW 4
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define GPS_HIGH 5
TinyGPSPlus gps;

// LCD
#define LCD_HIGH 19 
#define LCD_LOW 18
LiquidCrystal_I2C lcd(0x27, 20, 4); 

// config.h
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const String apiKey = HERE_API_KEY;

// Global Variables
#define ONBOARD_LED 2
unsigned long lastMillis = 0;

const String jsonResponse_Work = R"({"items":[{"title":"Sonae House, Sandton, 2191, South Africa","navigationAttributes":{"speedLimits":[{"maxSpeed":20,"direction":"W","speedUnit":"kph","source":"derived"},{"maxSpeed":20,"direction":"E","speedUnit":"kph","source":"derived"}]}}]})";

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

  // Serial.printf("[DEBUG] URL: %s\n", baseUrl + params);
  Serial.println("[DEBUG] URL: " + baseUrl + params);
  return baseUrl + params;
}

int getMaxSpeedForDirection(String jsonResponse, char* targetDirection) {
  // Regex pattern for extracting speed limits
  char* pattern = R"(\"maxSpeed\":\s*(\d+),.*?\"direction\":\s*\"(.*?)\")";
  MatchState ms;
  ms.Target(jsonResponse.c);

  int index = 0;
  int fallbackSpeed = 999; // Arbitrary high value for fallback
  char result = ms.Match(pattern, index);
  while (result == REGEXP_MATCHED) {
    char c_maxSpeed[3];
    char direction[2];
    ms.GetCapture(c_maxSpeed, 0);
    ms.GetCapture(direction, 1);
    Serial.printf("[DEBUG] Direction: %s, maxSpeed: %s\n", direction, c_maxSpeed);

    int i_maxSpeed = atoi(c_maxSpeed);
    if (fallbackSpeed > i_maxSpeed) {
        fallbackSpeed = i_maxSpeed; // Fallback speed is the min of all speed limits found
        Serial.printf("[DEBUG] fallbackSpeed in if: %d\n", fallbackSpeed);
    }
  
    if (direction == targetDirection) {
      Serial.printf("[DEBUG] maxSpeed for direction found: %d\n", i_maxSpeed);
      return i_maxSpeed;  // Return the maxSpeed for the matching direction
    }

    index = ms.MatchStart + ms.MatchLength;
  }

  return fallbackSpeed == 999 ? -1 : fallbackSpeed;
}

// Function to get speed limit using HERE API
int getSpeedLimit(double lat, double lng, String& direction) {
  // String url = "https://www.thunderclient.com/welcome";
  String url = constructApiUrl(lat, lng) ;
  Serial.printf("[DEBUG] URL: %s\n", url.c_str());

  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("[ERROR] No wifi connection.\n");
    return -1;
  };

  // HTTPClient http;
  // http.begin(url);
  // int httpCode = http.GET();
  // Serial.printf("[DEBUG] httpReturnCode: %d\n", httpCode);

  // if (httpCode != 200) {
  //   http.end();
  //   Serial.printf("[ERROR] HTTP GET Failed: Could not fetching speed limit.\n");
  //   return -1;
  // }

  // String payload = http.getString();
  // http.end();
  // String payload = jsonResponse_WierdaRoad ;
  String payload = jsonResponse_Work;
  Serial.println(payload);

  int result = getMaxSpeedForDirection(payload, direction);
  Serial.printf("[DEBUG] getMaxSpeedForDirection result = %d\n", result);

  if (result == -1) {
    Serial.printf("[ERROR] JsonParse: Could not extract the speed limit from the json.\n");
    return -1;
  }
  return result;
}

void displayLCD(int speedLimit, String direction) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.printf("Speed %3.0f / %-3d km/h", gps.speed.kmph(), speedLimit);

  lcd.setCursor(0, 1);

  lcd.setCursor(0, 2);

  lcd.setCursor(0, 3);
  lcd.printf("%02d-%02d %4s  %2s %2s%02ds", 
    gps.date.day(), 
    gps.date.month(), 
    (WiFi.status() == WL_CONNECTED) ? "WiFi" : "", 
    direction,                                      
    (1 - (gps.time.minute() % 2) == 1) ? "1m" : "", 
    60 - gps.time.second()
  );                        
}

void updateLCD() {
  lcd.setCursor(6, 0);
  lcd.printf("%3.0f", gps.speed.kmph());

  lcd.setCursor(15, 3);
  lcd.printf("%2s%02ds",                                 
    (1 - (gps.time.minute() % 2) == 1) ? "1m" : "", 
    60 - gps.time.second()
  );
}

void setup() {
  // USB Serial Communication
  Serial.begin(115200);
  Serial.printf("[SETUP] Initializing...\n");

  // GPS Communication
  Serial1.begin(GPSBaud, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.printf("[SETUP] GPS module initialized.\n");

  // Digital Output Pin Setup and Initialization
  pinMode(ONBOARD_LED, OUTPUT);
  pinMode(GPS_HIGH, OUTPUT);
  pinMode(GPS_LOW, OUTPUT);
  pinMode(LCD_HIGH, OUTPUT);
  pinMode(LCD_LOW, OUTPUT);
  digitalWrite(ONBOARD_LED, LOW);
  digitalWrite(GPS_HIGH, HIGH);
  digitalWrite(GPS_LOW, LOW);
  digitalWrite(LCD_HIGH, HIGH);
  digitalWrite(LCD_LOW, LOW);

  // LCD
  lcd.init();
  lcd.backlight();
  Serial.printf("[SETUP] LCD initialized.\n");

  // WiFi Connection
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(5000);
    Serial.printf("[SETUP] Connecting to WiFi...\n");
  }
  Serial.printf("[SETUP] Connected to WiFi.\n");
  Serial.printf("[SETUP] Setup Completed.\n");
}

void loop() {
  // Check for GPS data and parse it
  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }

  // Check if GPS time and date are valid
  if (gps.time.isValid() && gps.time.isUpdated() && 
      gps.date.isValid() && gps.date.isUpdated() &&
      gps.location.isValid() && gps.location.isUpdated()) {
  
    // Send data just before even minutes
    // if (gps.time.minute() % 2 == 1 && gps.time.second() == 50) {
    if (gps.time.second() % 30 == 0) {
      double lat = gps.location.lat();
      double lng = gps.location.lng();
      double heading = gps.course.deg();
      String direction = getCardinalDirection(heading);

      // Get speed limit
      int speedLimit = getSpeedLimit(lat, lng, direction);
      displayLCD(speedLimit, direction);

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

      Serial.println(data);
      delay(100);
    } 
  }
  else if ((millis() - lastMillis) > 500) {
    lastMillis = millis(); 
    if (gps.time.isValid() && gps.date.isValid() && gps.location.isValid()) {
      updateLCD(); 
    } else {
      lcd.setCursor(0, 0);
      lcd.print("GPS not ready yet!") ;
    }
  }
}
