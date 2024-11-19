#include <TinyGPS++.h>
#include <BluetoothSerial.h>

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

void setup() {
  // Begin Serial communication for debugging
  Serial.begin(115200);
  Serial.println("Initializing...");

  // Begin Bluetooth communication
  if (!btSerial.begin("ESP32")) { // Set Bluetooth device name
    Serial.println("Bluetooth initialization failed!");
    while (1); // Stay here in case of Bluetooth failure
  }
  Serial.println("Bluetooth initialized. Waiting for connections...");

  // Begin GPS communication
  Serial1.begin(GPSBaud, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("GPS module initialized.");

  //Digital Output pin setup and init
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
}

void loop() {
  // Check for GPS data and parse it
  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }

  // Check if GPS time and date are valid
  if (gps.time.isValid() && gps.date.isValid()) {
    // Get the current minute and second from GPS
    int currentMinute = gps.time.minute();
    int currentSecond = gps.time.second();

    // Send data on even minutes and only once per cycle (close to 0 seconds)
    if (currentMinute % 2 == 0 && currentSecond == 0) {
      // Prepare the data string as a comma-separated list
      String data = String(gps.date.year()) + "/" + 
                    String(gps.date.month()) + "/" + 
                    String(gps.date.day()) + "," +
                    String(gps.time.hour()) + ":" + 
                    String(gps.time.minute()) + ":" + 
                    String(gps.time.second()) + "," +
                    String(gps.location.lat(), 6) + "," +
                    String(gps.location.lng(), 6) + "," +
                    String(gps.course.deg(), 2) + "," +
                    String(gps.speed.kmph());

      // Print data to Bluetooth and Serial monitor
      btSerial.println(data);
      Serial.println(data);

      // Wait for a few seconds to avoid duplicate transmissions within the same minute
      delay(2000);
    }
  } 
  // else {
  //   btSerial.println("GPS data not valid yet...");
  //   Serial.println("GPS data not valid yet...");
  // }
}
