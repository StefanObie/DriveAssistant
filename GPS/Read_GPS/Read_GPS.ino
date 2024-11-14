#include <TinyGPS++.h>

// Initialize TinyGPS++ object
TinyGPSPlus gps;

// Set GPS module baud rate
const uint32_t GPSBaud = 9600;

void setup() {
  // Begin Serial communication for debugging (USB connection)
  Serial.begin(115200);
  delay(1000);
  Serial.println("GPS module is initializing...");

  // Begin Serial1 for GPS communication
  Serial1.begin(GPSBaud, SERIAL_8N1, 16, 17);  // RX=16, TX=17
}

void loop() {
  // Check if data is available from GPS
  while (Serial1.available() > 0) {
    gps.encode(Serial1.read()); // Parse the GPS data

    if (gps.location.isUpdated()) {
      // Display location information on Serial Monitor
      Serial.print("Latitude: ");
      Serial.print(gps.location.lat(), 6);
      Serial.print(", Longitude: ");
      Serial.print(gps.location.lng(), 6);
      Serial.print(", Altitude: ");
      Serial.print(gps.altitude.meters());
      Serial.println(" meters");

      // Display date and time information
      Serial.print("Date: ");
      Serial.print(gps.date.month());
      Serial.print("/");
      Serial.print(gps.date.day());
      Serial.print("/");
      Serial.print(gps.date.year());

      Serial.print(" Time: ");
      Serial.print(gps.time.hour());
      Serial.print(":");
      Serial.print(gps.time.minute());
      Serial.print(":");
      Serial.print(gps.time.second());
      Serial.println();
    }
  }
}


// #define GPS_RX_PIN 16
// #define GPS_TX_PIN 17
// #define GPS_BAUD 9600

// HardwareSerial GPS(2);

// void setup() {
//   Serial.begin(115200);
//   Serial.println("Starting GPS data reading...");
  
//   GPS.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
//   Serial.println("GPS Serial connection initialized.");
// }

// void loop() {
//   // Confirm GPS connection by printing a message if no data is received
//   if (!GPS.available()) {
//     Serial.println("No data from GPS module.");
//     delay(1000);
//     return;
//   }
  
//   while (GPS.available()) {
//     char c = GPS.read();
//     Serial.print(c);
//   }
// }

