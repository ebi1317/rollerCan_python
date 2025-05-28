"""Listens for serial commands like: SET_SPEED 1500\n

Parses them

Sends I²C commands to the RollerCAN driver"""

import serial
import time

class RollerController:
    def __init__(self, port='/dev/ttyUSB0', baudrate=115200):
        self.serial = serial.Serial(
            port=port,
            baudrate=baudrate,
            timeout=1
        )
        self.wait_time = 0.1
    
    def send_command(self, command: str) -> str:
        """Send command and return response"""
        self.serial.write(f"{command}\n".encode())
        time.sleep(self.wait_time)
        if self.serial.in_waiting:
            return self.serial.readline().decode().strip()
        return ""

    def set_speed(self, speed: int) -> None:
        """Set motor speed in 0.01 rpm units"""
        print(f"Setting speed to {speed/100:.2f} rpm")
        response = self.send_command(f"SET_SPEED {speed}")
        if response:
            print(f"Response: {response}")
        time.sleep(1)  # Wait for speed change

    def set_position(self, position: float) -> None:
        """Set position in degrees"""
        # Convert to 0.01 degree units
        pos_hundredths = int(position * 100)
        
        # First set position mode
        self.send_command("SET_MODE POS")
        
        # Set approach speed (10 rpm default)
        self.send_command("SET_SPEED 1000")
        
        print(f"Moving to position: {position} degrees")
        response = self.send_command(f"SET_POSITION {pos_hundredths}")
        if response:
            print(f"Response: {response}")
        time.sleep(3)  # Wait for movement to complete

    def close(self):
        """Stop motor and close connection"""
        self.set_speed(0)
        self.serial.close()

def main():
    try:
        controller = RollerController()
        
        """# Test speed control
        speeds = [1000]  # in 0.01 rpm
        for speed in speeds:
            controller.set_speed(speed)
        
        # Test position control
        positions = [0]  # in degrees
        for pos in positions:
            controller.set_position(pos)"""
        controller.set_speed(2000)  # Set to 20 rpm
        controller.set_position(90)
            
    finally:
        controller.close()

if __name__ == "__main__":
    main()