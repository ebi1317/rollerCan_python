#pragma once

#include <M5Core2.h>
#include <Wire.h>

// RollerCAN I2C Address and Registers
#define ROLLERCAN_I2C_ADDR 0x64

// Register Map
#define ROLLERCAN_REG_OUTPUT    0x00 
#define ROLLERCAN_REG_MODE      0x01
#define ROLLERCAN_REG_STATUS    0x0C
#define ROLLERCAN_REG_LED       0x30
#define ROLLERCAN_REG_SPEED     0x40
#define ROLLERCAN_REG_SPEED_FB  0x60
#define ROLLERCAN_REG_POSITION  0x80
#define ROLLERCAN_REG_POS_FB    0x90
#define ROLLERCAN_REG_CURRENT   0xB0
#define ROLLERCAN_REG_CUR_FB    0xC0
#define ROLLERCAN_REG_VERSION   0xFE

// Operation Modes
#define MODE_SPEED   1
#define MODE_POS     2
#define MODE_CURRENT 3
#define MODE_DIAL    4
// Button Definitions from button.h
#define BUTTON_FILTER 500
#define BUTTON_FILTER_TIMEROUT BUTTON_FILTER*3

typedef struct {
    uint8_t button_status;
    unsigned long button_delay;
    uint8_t is_pressed;
    uint8_t is_longlongpressed;
    uint8_t was_click;
    uint8_t was_longpress;
    uint8_t was_longlongpress;
} button_t;

class RollerCAN {
public:
    bool begin(TwoWire &wire = Wire);
    
    // Basic control
    bool setMotorOutput(bool enable);
    bool setMode(uint8_t mode);
    
    // Speed control
    bool setSpeed(int32_t speed);  // speed in 0.01 rpm
    int32_t getSpeed();
    bool setSpeedMaxCurrent(int32_t current); // current in 0.01mA
    
    // Position control  
    bool setPosition(int32_t position); // position in 0.01 degrees
    int32_t getPosition();
    bool setPositionMaxCurrent(int32_t current);
    
    // Current control
    bool setCurrent(int32_t current); // current in 0.01mA
    int32_t getCurrent();
    
    // System status
    uint8_t getStatus();
    uint8_t getError();
    bool isStalled();
    bool isOverVoltage();
    
    // LED control
    bool setLEDColor(uint8_t r, uint8_t g, uint8_t b);
    bool setLEDBrightness(uint8_t brightness);
    
    // Configuration
    bool calibrateEncoder();
    bool saveConfig();
   // Button handling
   button_t btnA;
   button_t btnB; 
   button_t btnC;
   
   void updateButtons() {
       if (!button_init_flag) {
           initButton(&btnA);
           initButton(&btnB);
           initButton(&btnC);
           button_init_flag = 1;
       }
       buttonUpdate(&btnA);
       buttonUpdate(&btnB); 
       buttonUpdate(&btnC);
   }
    
private:
    TwoWire *_wire;
    uint8_t _address;
    uint8_t _mode;
    bool _enabled;
    
    bool writeRegister(uint8_t reg, uint8_t *data, size_t len);
    bool readRegister(uint8_t reg, uint8_t *data, size_t len);
    uint8_t button_init_flag = 0;
    
    void initButton(button_t* btn) {
        btn->button_status = 1;
        btn->button_delay = 0;
        btn->is_pressed = 0;
        btn->is_longlongpressed = 0;
        btn->was_click = 0;
        btn->was_longpress = 0;
        btn->was_longlongpress = 0;
    }
    
    void buttonUpdate(button_t* btn) {
        static uint8_t is_long_pressing = 0;
        
        if (!btn->is_pressed && !btn->button_status) {
            btn->is_pressed = 1;
            btn->button_delay = millis();
        }
        
        if (btn->is_pressed && !btn->button_status) {
            if (millis() - btn->button_delay > 5000) {
                if (!is_long_pressing) {
                    btn->is_longlongpressed = 1;
                    is_long_pressing = 1;
                }
            }
        }
        
        if (is_long_pressing) {
            if (btn->button_status) {
                is_long_pressing = 0;
                btn->is_pressed = 0;
            }
        } else {
            if (btn->is_pressed && btn->button_status) {
                if (millis() - btn->button_delay >= 100 && 
                millis() - btn->button_delay <= 500)
                    btn->was_click = 1;
                else if (millis() - btn->button_delay >= 2000 && 
                millis() - btn->button_delay <= 5000)
                    btn->was_longpress = 1;
                else if (millis() - btn->button_delay > 5000)
                    btn->was_longlongpress = 1;
                btn->is_pressed = 0;
            }
        }
    }
};