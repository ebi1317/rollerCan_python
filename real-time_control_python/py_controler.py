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
    
    def get_speed(self) -> int:
        """Get current motor speed"""
        response = self.send_command("GET_SPEED")
        if response:
            try:
                return int(response)
            except ValueError:
                print(f"Invalid speed response: {response}")
        return 0
    def get_position(self) -> float:
        """Get current motor position in degrees"""
        response = self.send_command("GET_POSITION")
        if response:
            try:
                return int(response) / 100.0  # Convert from hundredths of a degree
            except ValueError:
                print(f"Invalid position response: {response}")
        return 0.0
    def set_cont_movment(self, speed: int, duration: int = None) -> None:
        """Set continuous movement for a specified duration"""
        self.set_speed(speed)
        self.get_speed()
        time.sleep(duration)
        self.set_speed(0)

    def normalize_position(self, position: int) -> int:
        """Normalize position to 0-360 degrees range"""
        return position % 36000  # 36000 = 360 degrees * 100 (internal units)

    def shortest_path_distance(self, current: int, target: int) -> int:
        """Calculate shortest path between two angles"""
        diff = target - current
        if abs(diff) > 18000:  # If difference > 180 degrees
            if diff > 0:
                diff -= 36000
            else:
                diff += 36000
        return diff

    def set_position(self, position: float, speed: int) -> None:
        """Set position in degrees using shortest path
        Args:
            position (float): Target position in degrees
            speed (int): Speed in 0.01 rpm units
        """
        # Convert degrees to internal units (hundredths of degrees)
        target_degrees = int(position * 100)
        current_position = self.get_position() * 100  # Convert to internal units
        
        # Normalize positions to 0-360 range
        current_normalized = self.normalize_position(int(current_position))
        target_normalized = self.normalize_position(target_degrees)
        
        # Calculate shortest path
        distance = self.shortest_path_distance(current_normalized, target_normalized)
        
        # Calculate absolute position based on current position
        base_turns = (int(current_position) // 36000) * 36000  # Get complete rotations
        target_position = base_turns + target_normalized
        
        # Adjust target if shortest path requires crossing 0/360 boundary
        if distance < 0:
            target_position -= 36000
        
        # Set position mode and move
        response = self.send_command("SET_MODE POS")
        if not response:
            print("Warning: No response from SET_MODE command")
        
        response = self.send_command(f"SET_SPEED {speed}")
        if not response:
            print("Warning: No response from SET_SPEED command")
        
        print(f"Moving to position: {position} degrees (internal units: {target_position})")
        response = self.send_command(f"SET_POSITION {target_position}")
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
        
        """ use this if you have list of speeds or positions
        # Test speed control
        speeds = [10,20,40,60,80,100]  # in rpm
        for speed in speeds:
            controller.set_cont_movment(speed)
        
        # Test position control
        positions = [0,25,50,90]  # in degrees
        for pos in positions:
            controller.set_position(pos)"""
        

        # Test basic communication
        print("Testing communication...")
        print("Current speed:", controller.send_command("GET_SPEED"))
        print("Current position:", controller.send_command("GET_POSITION"))
        
        # Test speed control
        #print("\nTesting speed control...")
        #controller.set_cont_movment(2000, 2)  # Move at 20 rpm for 2 seconds
        
        # Test position control
        print("\nTesting position control...")
        controller.set_position(0, 2000)  # Move to 45 degrees at 20 rpm

    finally:
        controller.close()

if __name__ == "__main__":
    main()
