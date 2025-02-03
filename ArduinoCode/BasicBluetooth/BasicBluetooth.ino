#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

void setup() {
  Serial.begin(115200);
  SerialBT.begin("ESP32_Echo");
  Serial.println("The device started, now you can pair it with Bluetooth!");
}

void loop() {
  if (SerialBT.available()) {
    // Buffer to store incoming data
    char incomingMessage[256]; // Adjust the size as necessary
    int index = 0;

    // Read incoming message into the buffer
    while (SerialBT.available() && index < sizeof(incomingMessage) - 1) {
      incomingMessage[index++] = SerialBT.read();
    }
    incomingMessage[index] = '\0'; // Null-terminate the string

    // Print received message to Serial Monitor
    Serial.print("Received: ");
    Serial.println(incomingMessage);

    // Echo the message back
    SerialBT.print("Echo: ");
    SerialBT.println(incomingMessage);
  }

  delay(20);
}
