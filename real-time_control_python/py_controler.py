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
    def set_speed(self, rpm: float) -> None:
        speed = int(rpm * 100)  # Convert to 0.01 rpm units
        print(f"Setting speed to {rpm:.2f} rpm")
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
       
    def set_cont_movment(self, speed: int, duration: int = None) -> None:
        # Set speed mode first
        self.send_command("SET_MODE SPEED")
        """Set continuous movement for a specified duration"""
        self.set_speed(speed)
        self.get_speed()
        time.sleep(duration)
        self.set_speed(0)

    def normalize_position(self, position: float) -> float:
        """Normalize position to 0-360 degree range."""
        normalized = position % 360
        if normalized < 0:
            normalized += 360
        return normalized

    def read_position(self) -> float:
        """Return current motor position in degrees."""
        response = self.send_command("GET_POSITION")
        if response:
            try:
                pos = int(response) / 100.0  # convert centi-degrees to degrees
                print(f"Motor position: {pos:.2f}°")
                normalized_pos = self.normalize_position(pos)
                print(f"Normalized position: {normalized_pos:.2f}°")
                return normalized_pos
            except ValueError:
                print("Invalid response:", response)
                return 0.0
        print("No response from M5Core2")
        return 0.0

    def shortest_path(self, current: float, target: float) -> float:
        """Calculate shortest rotation path between two angles."""
        diff = target - current
        if abs(diff) > 180:
            # If difference is more than 180, go the other way around
            if diff > 0:
                diff -= 360
            else:
                diff += 360
        return diff

    def set_position(self, position: float, speed: float = 20.0) -> None:
        """Move to position (0-360°) using shortest path."""
        try:
            current_pos = self.read_position()
            target_pos = self.normalize_position(position)
            
            # Calculate shortest path
            angle_diff = self.shortest_path(current_pos, target_pos)
            
            # Convert difference to centi-degrees
            #target_steps = int(angle_diff * 100)  # Convert to centi-degrees
            
            print(f"Moving from {current_pos:.2f}° to {target_pos:.2f}° (diff: {angle_diff:.2f}°)")
            
            self.send_command("SET_MODE POSITION")
            self.send_command(f"SET_SPEED {int(speed * 100)}")
            self.send_command(f"SET_POSITION {angle_diff}")  # Send relative steps
        except Exception as e:
            print(f"Error setting position: {e}")
   

    def close(self):
        """Stop motor and close connection"""
        try:
            self.set_speed(0)
        except Exception:
            pass
        if getattr(self, "serial", None):
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
        #print("Testing communication...")
       # print("Current speed:", controller.send_command("GET_SPEED"))
        #print("Current position:", controller.get_position())
        
        # Test speed control
        #print("\nTesting speed control...")
        #controller.set_cont_movment(20000, 2)  # Move at 20 rpm for 2 seconds
        
        # Test position control
        #print("\nTesting position control...")
        #controller.set_position(45, 2000)  # Move to 45 degrees at 20 rpm)
        print("current position:", controller.read_position())
        
        print("Motor controller initialized")
        
        print("\nAvailable commands:")
        print("  sp <rpm> <duration>  - Set speed in RPM with optional duration in seconds")
        print("  p <degrees>          - Move to absolute position in degrees")
        print("  s <stop>             - stop the motor")
        print("  e                    - Exit the program")
        
        while True:
            try:
                command = input("\nEnter command: ")
                parts = command.strip().split()
                
                if not parts:
                    continue
                    
                if parts[0] == "sp":
                    if len(parts) < 2:
                        print("Error: Speed value required")
                        continue
                    speed = float(parts[1])  # Use float for RPM input
                    duration = int(float(parts[2])) if len(parts) > 2 else None  # Convert to int
                    controller.set_cont_movment(speed, duration)
                    
                elif parts[0] == "p":
                    print("curent position:", controller.read_position())
                    if len(parts) < 2:
                        print("Error: Position value required")
                        continue
                    position = float(parts[1])
                    controller.set_position(position)
                
                elif parts[0] == "s":
                    print("Stopping motor...")
                    controller.set_speed(0)
                    print("curent position:", controller.read_position())
                    
                elif parts[0] == "e":
                    print("Exiting...")
                    break
                    
                else:
                    print("Invalid command")
                    
            except ValueError as e:
                print(f"Error: Invalid number format - {e}")
            except Exception as e:
                print(f"Error: {e}")

            
                
    except serial.SerialException as e:
        print(f"Failed to connect to motor controller: {e}")
    

            

    finally:
        if controller:
            controller.close()

if __name__ == "__main__":
    main()
