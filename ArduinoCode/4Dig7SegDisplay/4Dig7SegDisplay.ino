// Include the TM1637 library
#include <TM1637Display.h>

// Define the connections to the display
#define CLK 22  // Clock pin (D2)
#define DIO 21  // Data pin (D3)

// Initialize the TM1637 display
TM1637Display display(CLK, DIO);

void setup() {
  // Set the brightness of the display (0-7)
  display.setBrightness(5);  // Adjust brightness as needed
}

void loop() {
  static int counter = 0; // Counter variable

  // Display the counter value
  display.showNumberDec(counter, true); // true = leading zeros off

  // Increment the counter
  counter++;

  // Reset the counter after reaching 9999
  if (counter > 9999) {
    counter = 0;
  }

  // Wait for 1 second
  delay(1000);
}
