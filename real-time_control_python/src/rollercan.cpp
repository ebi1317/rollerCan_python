#include "rollercan.h"

bool RollerCAN::begin(TwoWire &wire) {
    _wire = &wire;
    _wire->begin();
    
    // Test communication by reading firmware version
    uint8_t version;
    if (!readRegister(ROLLERCAN_REG_VERSION, &version, 1)) {
        return false;
    }
    
    // Initialize with safe defaults
    setMotorOutput(false);
    setMode(MODE_SPEED);
    return true;
}

bool RollerCAN::setMotorOutput(bool enable) {
    uint8_t data = enable ? 1 : 0;
    return writeRegister(ROLLERCAN_REG_OUTPUT, &data, 1);
}

bool RollerCAN::setMode(uint8_t mode) {
    if (mode < MODE_SPEED || mode > MODE_DIAL) {
        return false;
    }
    return writeRegister(ROLLERCAN_REG_MODE, &mode, 1);
}

bool RollerCAN::setSpeed(int32_t speed) {
    uint8_t data[4];
    data[0] = speed & 0xFF;
    data[1] = (speed >> 8) & 0xFF;
    data[2] = (speed >> 16) & 0xFF;
    data[3] = (speed >> 24) & 0xFF;
    return writeRegister(ROLLERCAN_REG_SPEED, data, 4);
}

int32_t RollerCAN::getSpeed() {
    uint8_t data[4];
    if (!readRegister(ROLLERCAN_REG_SPEED_FB, data, 4)) {
        return 0;
    }
    return (int32_t)(data[0] | (data[1] << 8) | 
                     (data[2] << 16) | (data[3] << 24));
}

bool RollerCAN::setPosition(int32_t position) {
    uint8_t data[4];
    data[0] = position & 0xFF;
    data[1] = (position >> 8) & 0xFF;
    data[2] = (position >> 16) & 0xFF;
    data[3] = (position >> 24) & 0xFF;
    return writeRegister(ROLLERCAN_REG_POSITION, data, 4);
}

int32_t RollerCAN::getPosition() {
    uint8_t data[4];
    if (!readRegister(ROLLERCAN_REG_POS_FB, data, 4)) {
        return 0;
    }
    return (int32_t)(data[0] | (data[1] << 8) | 
                     (data[2] << 16) | (data[3] << 24));
}

bool RollerCAN::setCurrent(int32_t current) {
    if (current > 120000 || current < -120000) {
        current = (current > 0) ? 120000 : -120000;
    }
    
    uint8_t data[4];
    data[0] = current & 0xFF;
    data[1] = (current >> 8) & 0xFF;
    data[2] = (current >> 16) & 0xFF;
    data[3] = (current >> 24) & 0xFF;
    return writeRegister(ROLLERCAN_REG_CURRENT, data, 4);
}

int32_t RollerCAN::getCurrent() {
    uint8_t data[4];
    if (!readRegister(ROLLERCAN_REG_CUR_FB, data, 4)) {
        return 0;
    }
    return (int32_t)(data[0] | (data[1] << 8) | 
                     (data[2] << 16) | (data[3] << 24));
}

uint8_t RollerCAN::getStatus() {
    uint8_t status;
    if (!readRegister(ROLLERCAN_REG_STATUS, &status, 1)) {
        return 0;
    }
    return status;
}

bool RollerCAN::setLEDColor(uint8_t r, uint8_t g, uint8_t b) {
    uint8_t data[3] = {r, g, b};
    return writeRegister(ROLLERCAN_REG_LED, data, 3);
}

bool RollerCAN::writeRegister(uint8_t reg, uint8_t* data, size_t len) {
    _wire->beginTransmission(ROLLERCAN_I2C_ADDR);
    _wire->write(reg);
    _wire->write(data, len);
    return (_wire->endTransmission() == 0);
}

bool RollerCAN::readRegister(uint8_t reg, uint8_t* data, size_t len) {
    _wire->beginTransmission(ROLLERCAN_I2C_ADDR);
    _wire->write(reg);
    if (_wire->endTransmission(false) != 0) {
        return false;
    }
    
    size_t received = _wire->requestFrom(ROLLERCAN_I2C_ADDR, len);
    if (received != len) {
        return false;
    }
    
    for (size_t i = 0; i < len; i++) {
        data[i] = _wire->read();
    }
    return true;
}