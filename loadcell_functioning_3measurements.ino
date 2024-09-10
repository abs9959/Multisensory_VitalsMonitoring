#include "HX711.h"

// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 4;  // Use a GPIO pin suitable for your ESP32 board
const int LOADCELL_SCK_PIN = 18;  // Use a GPIO pin suitable for your ESP32 board

HX711 scale;

float calibrationFactor = 0; // Variable to store the calibration factor
const float knownMass = 0.285; // Mass in kilograms for calibration
const float gravity = 9.81;  // Acceleration due to gravity (m/s^2)

void setup() {
  Serial.begin(9600);
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);

  // Initial calibration
  //recalibrate();
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();
    
    if (command == 'm') {  // 'm' for measure
      Serial.println("Taking 3 measurements...");
      for (int i = 0; i < 3; i++) {
        float force = scale.get_units(10); // Get the force in Newtons
        Serial.print("Measurement ");
        Serial.print(i + 1);
        Serial.print(": ");
        Serial.print(force);
        Serial.println(" N"); // Display force in Newtons
        delay(1000);  // Delay between measurements
      }
    } 
    else if (command == 'r') {  // 'r' for recalibrate
      recalibrate();
    }
  }
  delay(1000);  // Loop delay to prevent excessive CPU usage
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
