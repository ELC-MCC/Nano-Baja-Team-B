#include <Servo.h>

// Pin Definitions
const int SERVO_PIN = 13;
const int ESC_LEFT_PIN = 12;
const int ESC_RIGHT_PIN = 11;

// Objects
Servo myServo;
Servo escLeft;
Servo escRight;

void setup() {
  // RFD 900x communicates at 57600 or 115200 by default
  Serial.begin(57600); 

  // Attach components
  myServo.attach(SERVO_PIN);
  escLeft.attach(ESC_LEFT_PIN, 1000, 2000);  // Standard PWM range
  escRight.attach(ESC_RIGHT_PIN, 1000, 2000);

  // Safety: Initialize ESCs at "zero" throttle
  escLeft.writeMicroseconds(1000);
  escRight.writeMicroseconds(1000);
  
  Serial.println("System Initialized. Awaiting RF commands...");
  delay(2000); // Allow ESCs to arm
}

void loop() {
  // Check if data is coming in from the RFD 900x
  if (Serial.available() > 0) {
    char command = Serial.read();

    // Simple Command Logic
    // 'W' = Full Throttle, 'S' = Stop, 'A' = Turn Left, 'D' = Turn Right
    switch (command) {
      case 'W': 
        updateDrive(1500, 1500, 90); // Throttle up, servo centered
        Serial.println("Moving Forward");
        break;
      case 'S':
        updateDrive(-1500, -1500, 90); // Reverse
        Serial.println("Reverse");
        break;
      case 'A':
        updateDrive(1200, 1200, 45); // Slow move, turn servo
        Serial.println("Turning Left");
        break;
      case 'D':
        updateDrive(1200, 1200, 135); // Slow move, turn servo
        Serial.println("Turning Right");
        break;
      default:
        updateDrive(0, 0, 90); // Brake
        Serial.println("stopped");
    }
  }
}

// Function to update all actuators at once
void updateDrive(int throttleL, int throttleR, int servoAngle) {
  escLeft.writeMicroseconds(throttleL);
  escRight.writeMicroseconds(throttleR);
  myServo.write(servoAngle);
}
