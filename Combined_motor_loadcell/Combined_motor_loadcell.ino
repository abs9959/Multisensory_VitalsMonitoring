//Libraries
//ADC for reading load cells
#include "HX711.h"

// CIRCUIT WIRING
// Motor control pins
#define MotFwd  19  // Motor Forward pin
#define MotRev  32  // Motor Reverse pin

//Encoder Pins
int encoderPinA = 18; // Encoder Output 'A' must be connected to an interrupt pin of the Arduino.
int encoderPinB = 2;  // Encoder Output 'B' must be connected to an interrupt pin of the Arduino.

volatile long encoderValue = 0;  // Store the current position of the encoder.
volatile int lastEncoded = 0; // Here updated value of encoder store.
long homePosition = 0;           // Variable to store the initial position.

bool isHighMode = false;         // Flag to indicate high mode.
bool isLowMode = false;          // Flag to indicate low mode.
bool tighten = false;
bool loosen = false;

// Load cell pins
const int LOADCELL_DOUT_PIN = 4;  //  Use a GPIO pin suitable for your ESP32 board
const int LOADCELL_SCK_PIN = 22;  // Use a GPIO pin suitable for your ESP32 board

unsigned long previousMillis = 0;  // Store the last time force was measured
const long interval = 500;         // Interval at which to measure force (500 milliseconds)


HX711 scale;

//Constants
float calibrationFactor = 0;      // Variable to store the calibration factor
const float knownMass = 0.379;    // Mass in kilograms for calibration (weight = .285kg)
const float gravity = 9.81;       // Acceleration due to gravity (m/s^2)
float targetForce = 0.0; 

void setup() {
  // Setup for motor control
  pinMode(MotFwd, OUTPUT); 
  pinMode(MotRev, OUTPUT); 
  Serial.begin(9600); // Initialize serial communication
  
  pinMode(encoderPinA, INPUT_PULLUP); 
  pinMode(encoderPinB, INPUT_PULLUP);
  
  digitalWrite(encoderPinA, HIGH); // Turn pullup resistor on.
  digitalWrite(encoderPinB, HIGH); // Turn pullup resistor on.
  
  attachInterrupt(digitalPinToInterrupt(encoderPinA), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), updateEncoder, CHANGE);
  
  homePosition = encoderValue;  // Store the initial encoder position as home position.
  Serial.print("Home Position = ");
  Serial.println(homePosition);

  // Setup for load cell
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  // recalibrate(); 
}


void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    // Handle motor control commands
    if (input.equals("h")) {
      isHighMode = true;
      isLowMode = false;
      tighten = false;
      loosen = false;
      motorControl();
    } else if (input.equals("l")) {
      isLowMode = true;
      isHighMode = false;
      tighten = false;
      loosen = false;
      motorControl();
    } else if (input.equals("t")) {
      tighten = true;
      isHighMode = false;
      isLowMode = false;
      loosen = false;
      motorControl();
    } else if (input.equals("ls")) {
      loosen = true;
      isHighMode = false;
      isLowMode = false;
      tighten = false;
      motorControl();

    //no motor function performed
    } else {
      isHighMode = false;
      isLowMode = false;
      tighten = false;
      loosen = false;

      if (input.equals('n'){
        motorControl();    
      
      // Measure force
      } else if (input.equals("m")) {  
          measure();
        
        // Recalibrate the load cell
        } else if (input.equals("r")) { 
            recalibrate();
      
        // Adjust motor to target force
        } else if (input.equals("f")) {
            Serial.print("Target force: ");
            while (Serial.available() == 0);  // Wait for input
            String targetInput = Serial.readStringUntil('\n');  // Read user input for target force
            float targetForce = targetInput.toFloat();  // Convert the input to a float
            if (targetForce != 0 || targetInput.equals("0")) {  // Valid check for 0 as well
              adjustMotorToForce(targetForce);
            } else {
            Serial.println("Invalid force input. Please provide a numeric value.");
            }
        }
    }
  }
}


// Functions
void measure(){
  for (int i = 0; i < 3; i++) {
    float force = scale.get_units(10);  // Get the force in Newtons
    Serial.print("Measurement ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(force);
    Serial.println(" N");
    delay(1000);  // Delay between measurements (1s)
  }
}

// Motor Control Function
void motorControl() {
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
    returnToHomePosition();
  } else if (isLowMode) {
    for (int i = 0; i < 3; i++) {
      digitalWrite(MotFwd, HIGH); 
      digitalWrite(MotRev, LOW);
      delay(800); // Quick clockwise turns.
      digitalWrite(MotFwd, LOW); 
      digitalWrite(MotRev, LOW);
      delay(200);
    }
    returnToHomePosition();
  } else if (tighten) {
    Serial.println("Tightening");
    digitalWrite(MotFwd, LOW); 
    digitalWrite(MotRev, HIGH);
    delay(2200);

  } else if (loosen) {
    Serial.println("Loosening");
    digitalWrite(MotRev, LOW); 
    digitalWrite(MotFwd, HIGH);
    delay(2200); 
  } else {
    Serial.println("Stopped");
    digitalWrite(MotFwd, LOW); 
    digitalWrite(MotRev, LOW); // No rotation
    delay(1000); // Delay in normal mode
  }

  measure();
}

void updateEncoder() {
  int MSB = digitalRead(encoderPinA); 
  int LSB = digitalRead(encoderPinB); 

  int encoded = (MSB << 1) | LSB;      
  int sum = (lastEncoded << 2) | encoded; 

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) encoderValue--;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) encoderValue++;

  lastEncoded = encoded;
}

void returnToHomePosition() {
  long currentPos = encoderValue;
  Serial.print("Current Position: ");
  Serial.println(currentPos);
  long error = homePosition - currentPos;
  Serial.print("Error: ");
  Serial.println(error);
  
  while (abs(error) > 1) {  
    if (error > 0) {
      digitalWrite(MotFwd, HIGH); 
      digitalWrite(MotRev, LOW); 
    } else {
      digitalWrite(MotFwd, LOW); 
      digitalWrite(MotRev, HIGH); 
    }
    delay(10); 
    currentPos = encoderValue;  
    error = homePosition - currentPos;
  }
  digitalWrite(MotFwd, LOW); 
  digitalWrite(MotRev, LOW); 
}

void recalibrate() {
  if (scale.is_ready()) {
    scale.set_scale();  // Reset the scale factor
    Serial.println("Tare... remove any weights from the scale.");
    delay(8000);
    scale.tare();
    Serial.println("Tare done...");
    Serial.print("Place a known weight on the scale...");
    delay(8000);  // Time to place the weight

    long reading = scale.get_units(10);  // Average of 10 readings
    Serial.print("Raw reading with known weight: ");
    Serial.println(reading);

    // Calculate the calibration factor using the known weight
    float knownForce = knownMass * gravity;  // Force in Newtons
    calibrationFactor = (float)reading / knownForce;
    Serial.print("Calibration Factor: ");
    Serial.println(calibrationFactor);

    // Set the scale to use the calculated calibration factor
    scale.set_scale(calibrationFactor);
    Serial.println("Recalibration complete.");
  } 
  else {
    Serial.println("HX711 not found. Recalibration failed.");
  }
}


void adjustMotorToForce(float targetForce) {
  bool notReached = true;
  float threshold = 0.1;  // N of noise, determined based on resolution

  while (notReached) {
    float currentForce = scale.get_units(10);  // Read force in Newtons, average of 10 readings
    Serial.print("Current Force: ");
    Serial.println(currentForce);

    if (abs(targetForce - currentForce) <= threshold) {
      notReached = false;
      break;  // Target force has been reached within the threshold
    } 
    else {
      if (currentForce < targetForce) {
        tighten = true;
        motorControl(); 
        tighten = false;
      }
      else {
        loosen = true;
        motorControl(); 
        loosen = false;
      }

      delay(100);  // Add a small delay to stabilize readings
    }
  }

  // Final force reading after loop ends 
  float finalForce = scale.get_units(10); 
  Serial.print("Final Force: ");
  Serial.println(finalForce);
}
