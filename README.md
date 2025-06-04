 RollerCAN Python Control

This repository demonstrates how to control a RollerCAN motor driver using Python and the M5Stack Core2 display.

🛠️ Requirements

* [VS Code](https://code.visualstudio.com/)
* [PlatformIO extension](https://platformio.org/)
* M5Stack Core2
* Python 3.x

⚙️ Setup Instructions

1. Install PlatformIO

Install the PlatformIO extension in VS Code.

2. Create a New Project

Create a new PlatformIO project for the M5Stack Core2.

3. Add the `M5Core2` Library

* Go to the PlatformIO sidebar -> **Libraries**
* Search for `M5Core2`
* Click Add to Project and choose your project

4. Upload Firmware to M5Core2

* Open `src/main.cpp`
* Build and upload the firmware to the M5Core2

5. Control via Python

Use `py_controller.py` on your PC to send commands and control the motor via USB serial.
