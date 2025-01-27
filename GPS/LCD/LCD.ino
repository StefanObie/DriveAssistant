#include <TinyGPS++.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "config.h"

// GPS
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
String lcd_message = "";

// config.h
const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const String apiKey = HERE_API_KEY;

// Global Variables
unsigned long lastMillisShortTimer = 0;
unsigned long lastMillisLongTimer = 0;
short speedLimit = 0;
String direction = "" ;
String district = "" ;
String street = "" ;
// bool fallbackSpeedUsed = false;

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

  Serial.printf("[DEBUG] URL: %s%s\n", baseUrl.c_str(), params.c_str());
  return baseUrl + params;
}

String postRequest(String url) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.printf("[ERROR] No wifi connection.\n");
    lcd_message = "No WiFi Connection";
    return "";
  };

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();
  Serial.printf("[DEBUG] httpReturnCode: %d\n", httpCode);

  if (httpCode != 200) {
    http.end();
    Serial.printf("[ERROR] HTTP GET Failed: Could not fetching speed limit.\n");
    lcd_message = "Post request failed";
    return "";
  }

  String response_payload = http.getString();
  http.end();

  Serial.println(response_payload);
  return response_payload;
}

short getMaxSpeedForDirection(const String& jsonResponse) {
  String targetDirection = direction;
  // Parse the JSON response
  StaticJsonDocument<1536> doc;
  DeserializationError error = deserializeJson(doc, jsonResponse);

  if (error) {
    Serial.printf("[ERROR] JSON deserialization failed: %s\n", error.c_str());
    return -1;
  }

  // Navigate through the JSON structure
  JsonArray items = doc["items"];
  if (items.isNull()) {
    Serial.printf("[ERROR] No \"items\" found in JSON response.\n");
    return -1;
  }

  for (JsonObject item : items) {
    //Street Title processing
    JsonArray address = item["address"];
    street = item["address"]["street"].as<String>();
    district = item["address"]["district"].as<String>();
    Serial.printf("[DEBUG] Street %s, District %s\n", street, district);

    //SpeedLimit Processing
    JsonArray jsonSpeedLimits = item["navigationAttributes"]["speedLimits"];
    if (jsonSpeedLimits.isNull()) {
      Serial.printf("[ERROR] No speed limits found.\n");
      return -1;
    }

    // Iterate through speed limits
    short fallbackSpeed = 999;
    // fallbackSpeedUsed = false;
    //TODO: Check for direction next to the target direction too. Look into array of char pointers.
    for (JsonObject jsonSpeedLimit : jsonSpeedLimits) {
      String jsonDirection = jsonSpeedLimit["direction"].as<String>();
      short maxSpeed = jsonSpeedLimit["maxSpeed"].as<short>();
      Serial.printf("[DEBUG] Direction: %s, maxSpeed: %d\n", jsonDirection, maxSpeed);

      if (fallbackSpeed > maxSpeed)
        fallbackSpeed = maxSpeed; // Fallback speed is the min of all speed limits found

      if (jsonDirection == targetDirection)
        return maxSpeed;  // Return the maxSpeed for the matching direction
    }

    Serial.printf("[DEBUG] No matching jsonDirection found, using fallback speed: %d km/h.\n", fallbackSpeed);
    // fallbackSpeedUsed = true;
    return fallbackSpeed;
  }

  Serial.printf("[ERROR] End of getMaxSpeedForDirection reached with no speed limit.\n");
  return -1;
}

void displayLCD() {
  //Speed
  lcd.setCursor(0, 0);
  lcd.printf("Speed %3.0f / %-3d km/h",
    gps.speed.kmph(), 
    speedLimit
    // (fallbackSpeedUsed ? "°" : "")
  );

  //Street Title
  lcd.setCursor(0, 1);
  lcd.printf("%-20s", street);

  //User feedback
  lcd.setCursor(0, 2);
  lcd.printf("%-20s", 
    (lcd_message == "" ? district.c_str() : lcd_message.c_str()) //Show error message, if any
  );

  //Date & Time, Wifi Status and Direction
  lcd.setCursor(0, 3);
  lcd.printf("%02d-%02d %4s  %2s %2s%02ds", 
    gps.date.day(), 
    gps.date.month(), 
    (WiFi.status() == WL_CONNECTED) ? "WiFi" : "", 
    direction,                                      
    (1 - (gps.time.minute() % 2) == 1) ? "1m" : "", 
    60 - (gps.time.second())
  );                        
}

void setup() {
  // USB Serial Communication
  Serial.begin(115200);
  Serial.printf("[SETUP] Initializing.\n");

  // GPS Communication
  Serial1.begin(GPSBaud, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.printf("[SETUP] GPS module initialized.\n");

  // Digital Output Pin Setup and Initialization
  pinMode(GPS_HIGH, OUTPUT);
  pinMode(GPS_LOW, OUTPUT);
  pinMode(LCD_HIGH, OUTPUT);
  pinMode(LCD_LOW, OUTPUT);
  digitalWrite(GPS_HIGH, HIGH);
  digitalWrite(GPS_LOW, LOW);
  digitalWrite(LCD_HIGH, HIGH);
  digitalWrite(LCD_LOW, LOW);
  Serial.printf("[SETUP] Pins initialized.\n");

  // LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  Serial.printf("[SETUP] LCD initialized.\n");

  // WiFi Connection
  WiFi.begin(ssid, password);
  Serial.printf("[SETUP] Connect the WiFi.\n");

  Serial.printf("[SETUP] Setup Completed.\n");
  lcd.clear();
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
    if (gps.time.minute() % 2 == 1 && gps.time.second() == 50) {
    // if (gps.time.second() % 30 == 0) {
      double lat = gps.location.lat();
      double lng = gps.location.lng();
      direction = getCardinalDirection(gps.course.deg());

      // Get speed limit
      String url = constructApiUrl(lat, lng);
      String response_payload = postRequest(url);
      speedLimit = getMaxSpeedForDirection(response_payload);

      Serial.printf("%d, %02d/%02d/%02d, %02d:%02d:%02d, (%.6f, %.6f), %s, %.0f, %d\n",
        gps.course.age(),
        gps.date.year(),
        gps.date.month(),
        gps.date.day(),
        (gps.time.hour() +2) % 24,
        gps.time.minute(),
        gps.time.second(),
        lat,
        lng,
        direction,
        gps.speed.kmph(),
        speedLimit
      );

      delay(100);

      // Ensure the error messages are displayed for Long Timer seconds
      lastMillisLongTimer = millis(); 
    } 
  }
  
  if ((millis() - lastMillisShortTimer) > 500) { //0.5s
    lastMillisShortTimer = millis(); 
    displayLCD(); 
  } 
  
  if ((millis() - lastMillisLongTimer) > 20000) { //20s
    lastMillisLongTimer = millis();
    lcd_message = "" ;
    lcd.clear();
    Serial.printf("[DEBUG] LCD cleared.\n") ;

    if (!gps.time.isValid() && !gps.date.isValid() && !gps.location.isValid()) {
      lcd_message = "No GPS Signal Yet!" ;
    }

    if (WiFi.status() != WL_CONNECTED) {
      WiFi.disconnect() ;
      WiFi.begin(ssid, password);
    }
  }
}
