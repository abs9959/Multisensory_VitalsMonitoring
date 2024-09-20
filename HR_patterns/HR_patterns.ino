#include <Wire.h>
#include "Adafruit_DRV2605.h"

// Create an instance of the DRV2605 class
Adafruit_DRV2605 drv;

// Define the PWM output pin
const int pwmPin = 5; // Choose any PWM-capable pin on your ESP32

int heartRate = 60; // Default heart rate

void customPulse(int duration) {
  // Set the PWM duty cycle to control the intensity of the pulse
  analogWrite(pwmPin, 200); // Set the duty cycle to 50% (0-255 range)
  
  // Start the DRV2605 effect
  drv.go();
  
  // Wait for the specified duration
  delay(duration);
  
  // Stop the DRV2605 effect
  drv.stop();
  
  // Turn off the PWM output
  analogWrite(pwmPin, 0); // Set the duty cycle to 0 (turn off the PWM output)
}

void sendVeryLowPattern(int pulseDurations[]) {
  // Send the vibration pattern
  for (int i = 0; i < 5; i++) {
    customPulse(pulseDurations[i]);
  }
  // Two beats of rest
  delay(pulseDurations[5]);
}

void sendLowPattern(int pulseDurations[]) {
  // Send the vibration pattern
  for (int i = 0; i < 3; i++) {
    customPulse(pulseDurations[i]);
  }
  // One beat of rest
  delay(pulseDurations[3]);
}

void sendNormalPattern(int pulseDurations[]) {
  // Send the vibration pattern
  for (int i = 0; i < 4; i++) {
    customPulse(pulseDurations[i]);
    delay(pulseDurations[4]);
  }
}

void sendHighPattern(int pulseDurations[]) {
  // Send the vibration pattern
  for (int i = 0; i < 3; i++) {
    customPulse(pulseDurations[i]);
  }
  // One beat of rest
  delay(pulseDurations[3]);
  delay(pulseDurations[4]);
}

void sendVeryHighPattern(int pulseDurations[]) {
  // Send the vibration pattern
  for (int i = 0; i <3; i++) {
    customPulse(pulseDurations[i]);
  }
  // One beat of rest
    delay(pulseDurations[6]);
  customPulse(pulseDurations[3]);
  customPulse(pulseDurations[4]);
  delay(pulseDurations[7]);
  delay(pulseDurations[5]);
}

// Function to calculate pulse durations based on heart rate and pattern
void calculatePulseDurations(int heartRate, int pattern, int pulseDurations[]) {
  int beatDuration = 60000 / heartRate; // Calculate duration of one heart beat in milliseconds
  
  switch (pattern) {
    case 0: // Very low
      // Three pulses with duration of 1/3 of the beat length on the first beat
      pulseDurations[0] = beatDuration / 3;
      pulseDurations[1] = beatDuration / 3;
      pulseDurations[2] = beatDuration / 3;
      // Two short pulses on the second beat
      pulseDurations[3] = beatDuration / 8;
      pulseDurations[4] = beatDuration / 8;
      // Two beats of rest
      pulseDurations[5] = 2 * beatDuration;
      break;
    case 1: // Low
      // Three vibrations of length 1/3 of the beat length starting on the first beat
      pulseDurations[0] = beatDuration / 3;
      pulseDurations[1] = beatDuration / 3;
      pulseDurations[2] = beatDuration / 3;
      // Two beats of rest
      pulseDurations[3] = 2 * beatDuration;
      break;
    case 2: // Normal
      // One vibration pulse at each heart beat that lasts 1/4 of the beat value
      pulseDurations[0] = beatDuration /4;
      pulseDurations[1] = beatDuration /4;
      pulseDurations[2] = beatDuration /4;
      pulseDurations[3] = beatDuration /4;
      // No rest
      pulseDurations[4] = (beatDuration /4) *3;
      break;
    case 3: // High
      // Three vibrations of length 1/2 of the beat length starting on the first beat
      pulseDurations[0] = beatDuration / 2;
      pulseDurations[1] = beatDuration / 2;
      pulseDurations[2] = beatDuration / 2;
      // Two beats of rest
      pulseDurations[3] = beatDuration /2 ;
      pulseDurations[4] = 1 * beatDuration;
      break;
    case 4: // Very high (same pattern as before)
      // First pulse starts on the first beat
      pulseDurations[0] = beatDuration / 2;
      // Second pulse starts halfway between first and second beat
      pulseDurations[1] = beatDuration / 2;
      // Third pulse starts on second beat and lasts half of the beat length
      pulseDurations[2] = beatDuration / 2;
      // Two quick pulses on the third beat
      pulseDurations[3] = beatDuration / 8;
      pulseDurations[4] = beatDuration / 8;
      // Two beats of rest
      pulseDurations[5] = 2 * beatDuration;
      pulseDurations[6] = beatDuration/2;
      pulseDurations[7] = beatDuration/8 * 6;
      break;
  }
}

void setup() {
  // Initialize the I2C communication
  Wire.begin();
  
  // Initialize the serial port for debugging
  Serial.begin(9600);
  
  // Initialize the DRV2605
  if (!drv.begin()) {
    Serial.println("Failed to find DRV2605!");
    while (1);
  }
  
  // Set the PWM pin as an output
  pinMode(pwmPin, OUTPUT);
  
  // Set the mode of the DRV2605 to internal trigger
  drv.setMode(DRV2605_MODE_INTTRIG);
}

void loop() {
  if (Serial.available() > 0) {
    // Read the heart rate value from the serial monitor
    int newHeartRate = Serial.parseInt();
    
    if (newHeartRate != 0) { // Check if a valid heart rate value is received
      heartRate = newHeartRate; // Update current heart rate
    }
    
    // Clear the serial buffer
    while (Serial.available() > 0) {
      Serial.read();
    }
  }
  
  // Determine the pattern based on heart rate range
  int pattern;
  if (heartRate <= 39) {
    pattern = 0; // Very low
    Serial.println("VERY LOW");
  } else if (heartRate >= 40 && heartRate <= 59) {
    pattern = 1; // Low
    Serial.println("LOW");
  } else if (heartRate >= 60 && heartRate <= 100) {
    pattern = 2; // Normal
    Serial.println("NORM");
  } else if (heartRate >= 101 && heartRate <= 120) {
    pattern = 3; // High
    Serial.println("HIGH");
  } else {
    pattern = 4; // Very high
    Serial.println("VERY HIGH");
  }
  
  // Calculate pulse durations based on heart rate and pattern
  int pulseDurations[10]; // Array to store pulse durations
  calculatePulseDurations(heartRate, pattern, pulseDurations);
  
// Send the appropriate vibration pattern based on pattern category
  switch(pattern) {
    case 0:
      sendVeryLowPattern(pulseDurations);
      break;
    case 1:
      sendLowPattern(pulseDurations);
      break;
    case 2:
      sendNormalPattern(pulseDurations);
      break;
    case 3:
      sendHighPattern(pulseDurations);
      break;
    case 4:
      sendVeryHighPattern(pulseDurations);
      break;
    default:
      // Handle invalid pattern
      break;
  }
}
