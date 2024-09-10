#define MotFwd  19  // Motor Forward pin
#define MotRev  32  // Motor Reverse pin

int encoderPin1 = 18; // Encoder Output 'A' must be connected to an interrupt pin of the Arduino.
int encoderPin2 = 2;  // Encoder Output 'B' must be connected to an interrupt pin of the Arduino.

volatile int lastEncoded = 0;  // Updated value of encoder store.
volatile long encoderValue = 0; // Raw encoder value
bool isHighMode = false;        // Flag to indicate high mode
bool isLowMode = false;         // Flag to indicate low mode
bool tighten = false;

void setup() {
  pinMode(MotFwd, OUTPUT); 
  pinMode(MotRev, OUTPUT); 
  Serial.begin(9600); // Initialize serial communication
  
  pinMode(encoderPin1, INPUT_PULLUP); 
  pinMode(encoderPin2, INPUT_PULLUP);
  
  digitalWrite(encoderPin1, HIGH); // Turn pullup resistor on
  digitalWrite(encoderPin2, HIGH); // Turn pullup resistor on
  
  // Call updateEncoder() when any high/low change is seen
  // on interrupt 0 (pin 2), or interrupt 1 (pin 3) 
  attachInterrupt(digitalPinToInterrupt(encoderPin1), updateEncoder, CHANGE); 
  attachInterrupt(digitalPinToInterrupt(encoderPin2), updateEncoder, CHANGE);
}

void loop() {
  // Check if high mode is active
  if (isHighMode) {
    Serial.println("high");
    for (int i = 0; i < 3; i++) {
      digitalWrite(MotFwd, LOW); 
      digitalWrite(MotRev, HIGH);
      delay(800); // Quick counter-clockwise turns
      digitalWrite(MotFwd, LOW); 
      digitalWrite(MotRev, LOW);
      delay(200);
    }
    //delay(500); // Delay before the longer clockwise turn
    digitalWrite(MotFwd, HIGH); 
    digitalWrite(MotRev, LOW);
    delay(2200); // Longer clockwise turn
  }
  
  // Check if low mode is active
  else if (isLowMode) {
    Serial.println("low");
    for (int i = 0; i < 3; i++) {
      digitalWrite(MotFwd, HIGH); 
      digitalWrite(MotRev, LOW);
      delay(800); // Quick clockwise turns
      digitalWrite(MotFwd, LOW); 
      digitalWrite(MotRev, LOW);
      delay(200);
    }
    //delay(500); // Delay before the longer counter-clockwise turn
    digitalWrite(MotFwd, LOW); 
    digitalWrite(MotRev, HIGH);
    delay(2600); // Longer counter-clockwise turn
  }

    else if (tighten) {
    Serial.println("tighten");
      digitalWrite(MotFwd, LOW); 
      digitalWrite(MotRev, HIGH);
      delay(2200); // Quick clockwise turns
  }
  
  // If neither high nor low mode is active, remain in normal mode
  else {
    digitalWrite(MotFwd, LOW); 
    digitalWrite(MotRev, LOW); // No rotation
    delay(1000); // Delay in normal mode
  }
  
  // Read serial input to change mode
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    if (input.equals("high")) {
      isHighMode = true;
      isLowMode = false;
      tighten = false;

    } else if (input.equals("low")) {
      isHighMode = false;
      isLowMode = true;
      tighten = false;
    } 
    else if (input.equals("tighten")) {
      isHighMode = false;
      isLowMode = false;
      tighten = true;
    }
    else if (input.equals("normal")) {
      isHighMode = false;
      isLowMode = false;
      tighten = false;

      
    }
  }
}

void updateEncoder() {
  int MSB = digitalRead(encoderPin1); // MSB = most significant bit
  int LSB = digitalRead(encoderPin2); // LSB = least significant bit

  int encoded = (MSB << 1) | LSB;       // Convert the 2 pin value to a single number
  int sum = (lastEncoded << 2) | encoded; // Add it to the previous encoded value

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue--;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue++;

  lastEncoded = encoded; // Store this value for next time
}
