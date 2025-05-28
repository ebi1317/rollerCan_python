#include "rollercan.h"
#include <M5Core2.h>

RollerCAN roller;
String inputString = "";    // String to hold incoming serial data
bool stringComplete = false; // Whether the string is complete

// Speed control buttons - top row
Button speedBtn1(10, 70, 95, 40, "100");    // x, y, w, h, label
Button speedBtn2(115, 70, 95, 40, "200");
Button speedBtn3(220, 70, 95, 40, "300");

// Position control buttons - arranged in two rows
Button degree1(10, 120, 95, 40, "0");      // First row
Button degree2(115, 120, 95, 40, "90");
Button degree3(220, 120, 95, 40, "180");
Button degree4(62, 170, 95, 40, "270");    // Second row
Button degree5(167, 170, 95, 40, "360");

// Global variables for control
int32_t targetPosition = 0;    // Target position in 0.01 degrees
bool positionMode = false;     // Flag to indicate if in position mode
int32_t approachSpeed = 1000;  // Default approach speed in 0.01 rpm
bool negativeDirection = false;  // Track rotation direction
int32_t positionSpeed = 5;  // Speed for position changes 

int32_t normalizePosition(int32_t rawPosition) {
    // Convert from 0.01 degree units to actual degrees
    int32_t degrees = rawPosition / 100;
    
    // Normalize to 0-360 range
    degrees = degrees % 360;
    if (degrees < 0) {
        degrees += 360;
    }
    
    return degrees;
}
   

int32_t shortestPathDistance(int32_t current, int32_t target) {
    current = normalizePosition(current);
    target = normalizePosition(target);
    
    int32_t clockwise = target - current;
    if (clockwise < 0) clockwise += 360;
    
    int32_t counterClockwise = target - current;
    if (counterClockwise > 0) counterClockwise -= 360;
    
    return (abs(clockwise) <= abs(counterClockwise)) ? clockwise : counterClockwise;
}

void setMotorSpeed(int32_t speed) {
    if (positionMode) {
        roller.setSpeed(speed);
    } else {
        roller.setMode(MODE_SPEED);
        roller.setSpeed(negativeDirection ? -speed : speed);
    }
}

void moveToNormalizedPosition(int32_t targetDegrees) {
    int32_t currentDegrees = normalizePosition(roller.getPosition());
    int32_t distance = shortestPathDistance(currentDegrees, targetDegrees);
    
    // Calculate absolute position based on current position
    int32_t currentPos = roller.getPosition();
    int32_t baseTurns = (currentPos / 36000) * 36000;  // Get complete rotations
    targetPosition = baseTurns + (targetDegrees * 100);
    
    // Adjust target if shortest path requires crossing 0/360 boundary
    if (distance < 0) {
        targetPosition -= 36000;
    }
    
    positionMode = true;
    roller.setMode(MODE_POS);
    roller.setSpeed(approachSpeed);
    roller.setPosition(targetPosition);
}

void processSerialCommand() {
    if (inputString.startsWith("SET_SPEED")) {
        int speed = inputString.substring(10).toInt(); // Skip "SET_SPEED " and convert to int
        approachSpeed = speed;  // Speed in 0.01 rpm units
        setMotorSpeed(approachSpeed);
        Serial.printf("Speed set to %.2f rpm\n", speed/100.0f);
    }
    // Clear the string for next command
    inputString = "";
    stringComplete = false;
}

void setup() {
    Serial.begin(115200);  // Initialize serial communication
    inputString.reserve(200);  // Reserve 200 bytes for the inputString
    
    M5.begin();
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextSize(2);
    
    // Initialize RollerCAN
    if (!roller.begin()) {
        M5.Lcd.println("RollerCAN init failed!");
        while(1) delay(100);
    }
    
    // Set to speed mode and enable motor
    roller.setMode(MODE_SPEED);
    roller.setMotorOutput(true);

    // Configure and draw speed buttons
    M5.Lcd.fillRect(10, 70, 95, 40, BLUE);     // Speed 1000
    M5.Lcd.fillRect(115, 70, 95, 40, BLUE);    // Speed 2000
    M5.Lcd.fillRect(220, 70, 95, 40, BLUE);    // Speed 3000

    // Configure and draw position buttons
    M5.Lcd.fillRect(10, 120, 95, 40, GREEN);   // 0 degrees
    M5.Lcd.fillRect(115, 120, 95, 40, GREEN);  // 90 degrees
    M5.Lcd.fillRect(220, 120, 95, 40, GREEN);  // 180 degrees
    M5.Lcd.fillRect(62, 170, 95, 40, GREEN);   // 270 degrees
    M5.Lcd.fillRect(167, 170, 95, 40, GREEN);  // 360 degrees

    // Draw button labels with centered text
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(WHITE);
    
    // Speed button labels
    M5.Lcd.drawString("10", 30, 80);
    M5.Lcd.drawString("20", 135, 80);
    M5.Lcd.drawString("30", 240, 80);
    
    // Position button labels
    M5.Lcd.drawString("0", 45, 130);
    M5.Lcd.drawString("90", 150, 130);
    M5.Lcd.drawString("180", 245, 130);
    M5.Lcd.drawString("270", 85, 180);
    M5.Lcd.drawString("360", 190, 180);



    // Draw physical button labels at bottom
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.drawString("ANTI", 50, 220);
    M5.Lcd.drawString("STOP", 160, 220);
    M5.Lcd.drawString("CW", 270, 220);

    // Degree button handlers
    degree1.addHandler([](Event& e) {
        moveToNormalizedPosition(0);
    }, E_TOUCH);
    
    degree2.addHandler([](Event& e) {
        moveToNormalizedPosition(90);
    }, E_TOUCH);
    
    degree3.addHandler([](Event& e) {
        moveToNormalizedPosition(180);
    }, E_TOUCH);
    
    degree4.addHandler([](Event& e) {
        moveToNormalizedPosition(270);
    }, E_TOUCH);
    
    degree5.addHandler([](Event& e) {
        moveToNormalizedPosition(360);
    }, E_TOUCH);

    // Speed button handlers
    speedBtn1.addHandler([](Event& e) {
        approachSpeed = 12000;  // 120 rpm
        setMotorSpeed(approachSpeed);
    }, E_TOUCH);
    
    speedBtn2.addHandler([](Event& e) {
        approachSpeed = 20000;  // 200 rpm
        setMotorSpeed(approachSpeed);
    }, E_TOUCH);
    
    speedBtn3.addHandler([](Event& e) {
        approachSpeed = 30000;  // 300 rpm
        setMotorSpeed(approachSpeed);
    }, E_TOUCH);

    // Draw button labels
    M5.Lcd.setTextSize(1);
    M5.Lcd.setTextColor(WHITE);
    M5.Lcd.drawString("ANTI", 50, 220);  // Button A label
    M5.Lcd.drawString("STOP", 160, 220); // Button B label
    M5.Lcd.drawString("CW", 270, 220);   // Button C label
}

void loop() {
    M5.update();
    
    // Handle serial input
    while (Serial.available()) {
        char inChar = (char)Serial.read();
        if (inChar == '\n') {
            stringComplete = true;
        } else {
            inputString += inChar;
        }
    }

    if (stringComplete) {
        processSerialCommand();
    }
    
    // Display current status with normalized position
    M5.Lcd.fillRect(10, 10, 300, 35, BLACK);
    M5.Lcd.setCursor(10, 10);
    M5.Lcd.setTextSize(2);
    M5.Lcd.setTextColor(GREEN);
    M5.Lcd.printf("Speed: %.2f rpm", roller.getSpeed() / 100.0f);
    M5.Lcd.setCursor(10, 30);
    M5.Lcd.printf("Pos: %d deg", normalizePosition(roller.getPosition()));

    // Handle physical buttons
    if (M5.BtnA.wasPressed()) {
        // Counter-clockwise rotation
        positionMode = false;
        negativeDirection = true;
        roller.setMode(MODE_SPEED);
        roller.setSpeed(-approachSpeed);
    } else if (M5.BtnB.wasPressed()) {
        // Stop
        roller.setSpeed(0);
        positionMode = false;
    } else if (M5.BtnC.wasPressed()) {
        // Clockwise rotation
        positionMode = false;
        negativeDirection = false;
        roller.setMode(MODE_SPEED);
        roller.setSpeed(approachSpeed);
    }
    
    
    delay(100);
}