#include <WiFi.h>
#include <Wire.h>
#include "Adafruit_DRV2605.h"
#include <esp_wpa2.h> // Include the WPA2 library

const char* ssid = "wpa.mcgill.ca";
const char* username = "abigail.buller@mail.mcgill.ca";
const char* password = "rowMacr0w9959!";

// Create an instance of the DRV2605 class
Adafruit_DRV2605 drv;

// Define the PWM output pins
const int pwmPinHR = 5; // Choose any PWM-capable pin on your ESP32 (Changed from 5 to 4)

int heartRate = 60; // Default heart rate
int pattern = 2;
int pulseDurations[10]; // Array to store pulse durations

WiFiServer serverHR(12345);  // The port number must match the one in the Python script


void customPulse(int duration) {
  // Set the PWM duty cycle to control the intensity of the pulse
  analogWrite(pwmPinHR, 200); // Set the duty cycle to 50% (0-255 range)
  
  // Start the DRV2605 effect
  drv.go();
  
  // Wait for the specified duration
  delay(duration);
  
  // Stop the DRV2605 effect
  drv.stop();
  
  // Turn off the PWM output
  analogWrite(pwmPinHR, 0); // Set the duty cycle to 0 (turn off the PWM output)
}

//void calculatePulseDurations(int heartRate, int pattern, int* pulseDurations) {
//  // Calculate the pulse durations based on heart rate and pattern
//  for (int i = 0; i < 10; i++) {
//    pulseDurations[i] = (1000 / heartRate) * (pattern + 1);
//  }
//}

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

void handleHRClient(void * parameter) {
  WiFiServer* server = (WiFiServer*)parameter;
  
  while (true) {
    WiFiClient client = server->available();
    int newHeartRate;

    if (client) {
      Serial.println("HR Client connected");
      String currentLine = "";
      while (client.connected()) {
        if (client.available()) {
          char c = client.read();
          if (c == '\n') {
             //Process the line when we receive a newline character
            if (currentLine.startsWith("HR")) {
              int sent_pattern, sent_heartrate, duration;
              sscanf(currentLine.c_str(), "HR,%d,%d,%d", &sent_pattern, &sent_heartrate, &duration);
              Serial.printf("Received HR: %d, %d, %d\n", sent_pattern, sent_heartrate, duration);

              newHeartRate = sent_heartrate;
      
              if (newHeartRate != 0) { // Check if a valid heart rate value is received
                heartRate = newHeartRate; // Update current heart rate
              }
              
              // Determine the pattern based on heart rate range
              if (heartRate <= 39) {
                pattern = 0; // Very low
              } else if (heartRate >= 40 && heartRate <= 59) {
                pattern = 1; // Low
              } else if (heartRate >= 60 && heartRate <= 100) {
                pattern = 2; // Normal
              } else if (heartRate >= 101 && heartRate <= 120) {
                pattern = 3; // High
              } else {
                pattern = 4; // Very high
              }

              // Calculate pulse durations based on heart rate and pattern
              calculatePulseDurations(heartRate, pattern, pulseDurations);

              unsigned long startTime = millis();
              unsigned long runDuration = duration*1000;
                            
              // Send the appropriate vibration pattern based on pattern category
                while (millis() - startTime < runDuration) {
                switch (pattern) {
                case 0:
                  sendVeryLowPattern(pulseDurations);
                  Serial.printf("HR VERY LOW\n");
                  break;
                case 1:
                  sendLowPattern(pulseDurations);
                  Serial.printf("HR LOW\n");
                  break;
                case 2:
                  sendNormalPattern(pulseDurations);
                  Serial.printf("HR NORMAL\n");
                  break;
                case 3:
                  sendHighPattern(pulseDurations);
                  Serial.printf("HR HIGH\n");
                  break;
                case 4:
                  sendVeryHighPattern(pulseDurations);
                  Serial.printf("HR VERY HIGH\n");
                  break;
                default:
                  // Handle invalid pattern
                  break;
              }
              delay(10);
             }
              
            }
            currentLine = "";
          } else if (c != '\r') {
            currentLine += c;
          }
        }
      }
      client.stop();
      Serial.println("HR Client disconnected");
    }
    delay(10); // Allow the task to yield
  }
}


void setup() {
  Serial.begin(115200);

  WiFi.disconnect(true);  // Disconnect from any network
  WiFi.mode(WIFI_STA);    // Set WiFi mode to station
  esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)username, strlen(username));
  esp_wifi_sta_wpa2_ent_set_username((uint8_t *)username, strlen(username));
  esp_wifi_sta_wpa2_ent_set_password((uint8_t *)password, strlen(password));
  esp_wifi_sta_wpa2_ent_enable(); // Enable WPA2 Enterprise

  //for wpa2 network
  WiFi.begin(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  WiFi.status();
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Start the WiFi servers
  serverHR.begin();
  Serial.println("WiFi servers started");


  //for motor control
  // Initialize the I2C communication
  Wire.begin();

  // Initialize the DRV2605 motor driver
  if (!drv.begin()) {
    Serial.println("Could not find DRV2605");
    while (1);
  }

  // Set the PWM pin as an output
  pinMode(pwmPinHR, OUTPUT);

  // Set the mode of the DRV2605 to internal trigger
  drv.setMode(DRV2605_MODE_INTTRIG);

  // Initialize the PWM pins
  pinMode(pwmPinHR, OUTPUT);


   xTaskCreatePinnedToCore(
    handleHRClient,    // Function to be called
    "HRClientTask",    // Name of the task
    10000,             // Stack size (bytes)
    &serverHR,         // Parameter to pass
    1,                 // Task priority
    NULL,              // Task handle
    0);                // Core to pin the task to

  
}

void loop() {

}
