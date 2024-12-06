// Define the pins for the 7-segment decoder
#define MS_DIGIT0 12
#define MS_DIGIT1 13
#define MS_DIGIT2 14
#define MS_DIGIT3 27

#define LS_DIGIT_G 33
#define LS_DIGIT_CF 32

int number;

void setup() {
  Serial.begin(115200);
  // Initialize the pins as output
  pinMode(MS_DIGIT0, OUTPUT);
  pinMode(MS_DIGIT1, OUTPUT);
  pinMode(MS_DIGIT2, OUTPUT);
  pinMode(MS_DIGIT3, OUTPUT);
  pinMode(LS_DIGIT_G, OUTPUT);
  pinMode(LS_DIGIT_CF, OUTPUT);

  number = 80; // Replace with the desired number to test
}

void loop() {
  // Serial.println(String(number));
  displaySpeedlimit(number);

  number += 10 ;
  if (number == 130) {
    number = 60;
  }

  delay(3000); // Add a delay for testing purposes
}

// Function to get the most significant digit of a number
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
