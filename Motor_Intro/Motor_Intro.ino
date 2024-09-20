#define MotFwd  19  // Motor Forward pin
#define MotRev  32  // Motor Reverse pin

int encoderPinA = 18; // Encoder Output 'A' must be connected to an interrupt pin of the Arduino.
int encoderPinB = 2;  // Encoder Output 'B' must be connected to an interrupt pin of the Arduino.

volatile long encoderValue = 0;  // Store the current position of the encoder.
volatile int lastEncoded = 0; // Here updated value of encoder store.
long homePosition = 0;           // Variable to store the initial position.
bool isHighMode = false;         // Flag to indicate high mode.
bool isLowMode = false;          // Flag to indicate low mode.
bool tighten = false;
bool loosen = false;

void setup() {
  pinMode(MotFwd, OUTPUT); 
  pinMode(MotRev, OUTPUT); 
  Serial.begin(9600); // Initialize serial communication
  
  pinMode(encoderPinA, INPUT_PULLUP); 
  pinMode(encoderPinB, INPUT_PULLUP);
  
  digitalWrite(encoderPinA, HIGH); // Turn pullup resistor on.
  digitalWrite(encoderPinB, HIGH); // Turn pullup resistor on.
  
  // Attach interrupts for encoder pins.
  attachInterrupt(digitalPinToInterrupt(encoderPinA), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), updateEncoder, CHANGE);
  
  // Store the initial encoder position as home position.
  homePosition = encoderValue;
  Serial.print("Home Position = ");
  Serial.println(homePosition);
}

void loop() {
  // Check if high mode is active.
  if (isHighMode) {
    for (int i = 0; i < 3; i++) {
      Serial.println("Quick turn");
      digitalWrite(MotFwd, LOW); 
      digitalWrite(MotRev, HIGH);
      delay(800); // Quick counter-clockwise turns.
      digitalWrite(MotFwd, LOW); 
      digitalWrite(MotRev, LOW);
      delay(200);
    }
    
    // After 3 quick turns, return to home position.
    Serial.println("Returning to home position");
    returnToHomePosition();
  }
  
  // Check if low mode is active.
  else if (isLowMode) {
    for (int i = 0; i < 3; i++) {
      digitalWrite(MotFwd, HIGH); 
      digitalWrite(MotRev, LOW);
      delay(800); // Quick clockwise turns.
      digitalWrite(MotFwd, LOW); 
      digitalWrite(MotRev, LOW);
      delay(200);
    }

    // After 3 quick turns, return to home position.
    returnToHomePosition();
  }

  else if (tighten) {
    Serial.println("Tightening");
    digitalWrite(MotFwd, LOW); 
    digitalWrite(MotRev, HIGH);
    delay(2200); // Quick clockwise turns.
  }
  
  else if (loosen) {
    Serial.println("Looseining");
    digitalWrite(MotRev, LOW); 
    digitalWrite(MotFwd, HIGH);
    delay(2200); // Quick clockwise turns.
  }
  
  // Normal mode (no movement).
  else {
    digitalWrite(MotFwd, LOW); 
    digitalWrite(MotRev, LOW); // No rotation.
    delay(1000); // Delay in normal mode.
  }
  
  // Read serial input to change mode.
  //h=highMode; l=lowMode; t=tighten; ls=loosen; n=noMovement
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    if (input.equals("h")) {
      isHighMode = true;
      isLowMode = false;
      tighten = false;
      loosen = false;
    } else if (input.equals("l")) {
      isHighMode = false;
      isLowMode = true;
      tighten = false;
      loosen = false;
    } else if (input.equals("t")) {
      isHighMode = false;
      isLowMode = false;
      tighten = true;
      loosen = false;
    } else if (input.equals("l")) {
      isHighMode = false;
      isLowMode = false;
      tighten = false;
      loosen = false;
    } else if (input.equals("ls")) {
      isHighMode = false;
      isLowMode = false;
      tighten = false;
      loosen = true;
      
    } else if (input.equals("n")) {
      isHighMode = false;
      isLowMode = false;
      tighten = false;
    }
  }
}

void updateEncoder() {
  int MSB = digitalRead(encoderPinA); // MSB = most significant bit.
  int LSB = digitalRead(encoderPinB); // LSB = least significant bit.

  int encoded = (MSB << 1) | LSB;      // Convert the 2 pin value to a single number.
  int sum = (lastEncoded << 2) | encoded; // Add it to the previous encoded value.

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue--;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue++;

  lastEncoded = encoded; // Store this value for next time.
}

// Function to return the motor to the home position.
void returnToHomePosition() {
  long currentPos = encoderValue;
  Serial.print("Current Position: ");
  Serial.println(currentPos);
  long error = homePosition - currentPos;
  Serial.print("Error: ");
  Serial.println(error);
  
  while (abs(error) > 1) {  // Continue until error is small enough.
    if (error > 0) {
      digitalWrite(MotFwd, HIGH); 
      digitalWrite(MotRev, LOW); // Move clockwise.
    } else {
      digitalWrite(MotFwd, LOW); 
      digitalWrite(MotRev, HIGH); // Move counter-clockwise.
    }

    delay(10); // Adjust delay to control speed.
    currentPos = encoderValue;  // Update current position.
    error = homePosition - currentPos; // Recalculate error.
  }
  
  digitalWrite(MotFwd, LOW); 
  digitalWrite(MotRev, LOW); // Stop the motor.
}
