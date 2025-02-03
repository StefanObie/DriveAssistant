#include <ArduinoJson.h>

// Global Variables
#define ONBOARD_LED 2
unsigned long lastMillis = 0;

// const String jsonResponse_Work = R"({"items":[{"title":"Sonae House, Sandton, 2191, South Africa","navigationAttributes":{"speedLimits":[{"maxSpeed":20,"direction":"W","speedUnit":"kph","source":"derived"},{"maxSpeed":20,"direction":"E","speedUnit":"kph","source":"derived"}]}}]})";
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

int getMaxSpeedForDirection(const String& jsonResponse, String targetDirection) {
  // Parse the JSON response
  StaticJsonDocument<512> doc;
  DeserializationError error = deserializeJson(doc, jsonResponse);

  if (error) {
    Serial.printf("[ERROR] JSON deserialization failed: %s\n", error.c_str());
    return -1;
  }

  int fallbackSpeed = 999;
  for (JsonObject speedLimits : doc["items"][0]["navigationAttributes"]["speedLimits"].as<JsonArray>()) {
    int maxSpeed = speedLimits["maxSpeed"];
    String direction = speedLimits["direction"];
    Serial.printf("maxSpeed %d; direction %s\n", maxSpeed, direction);

    if (fallbackSpeed > maxSpeed) {
      fallbackSpeed = maxSpeed; // Fallback speed is the min of all speed limits found
      Serial.printf("[DEBUG] fallbackSpeed in if: %d\n", fallbackSpeed);
    }
    

    if (direction == targetDirection) {
      Serial.printf("[DEBUG] maxSpeed: %d\n", maxSpeed);
      return maxSpeed;  // Return the maxSpeed for the matching direction
    }

    Serial.printf("[DEBUG] No matching direction found, using fallback speed: %d km/h.\n", fallbackSpeed);
    return fallbackSpeed;
  }
}

// Function to get speed limit using HERE API
int getSpeedLimit(double lat, double lng, String direction) {
  String payload = jsonResponse_Work;

  int result = getMaxSpeedForDirection(payload, direction);
  Serial.printf("[DEBUG] getMaxSpeedForDirection result = %d\n", result);

  if (result == -1) {
    Serial.printf("[ERROR] JsonParse: Could not extract the speed limit from the json.\n");
    return -1;
  }
  return result;
}

void setup() {
  Serial.begin(115200);
  Serial.printf("[SETUP] Setup Completed.\n");
}

void loop() {
  String direction = "N";
  int result = getSpeedLimit(0,0,direction);
  Serial.printf("Loop result: %d", result);
  delay(15000);
}
