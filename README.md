## ESP32 + CC1101 with LVGL Touch Support
I tested various touch libraries, and the only one that allowed the CC1101 to work correctly was LVGL.
I know this may look messy, but I figured I'd share it so others can improve or use it as they please.

![Image](https://github.com/user-attachments/assets/46c7ffc2-ecaf-41bf-a22b-b3ce93cd9371)


# Setup Instructions
1. Install Arduino IDE
Make sure you have the Arduino IDE installed before proceeding.

2. Install ESP32 Board Support
Open Arduino IDE.
Go to File > Preferences.
In the Additional Boards Manager URLs field, enter:
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
Click OK and restart the Arduino IDE.
3. Use the Correct ESP32 Version
The test phase is using ESP32 v2.0.14.
Versions above 2.0.14 may not work correctly.
If you experience issues, downgrade to 2.0.14 or lower.
💡 Note: As of 2024/08/02, TFT_eSPI does not work on versions higher than 2.0.14 (Issue #3329).

# Items used
- ESP32 Lilygo T-Display S3
- CC1101 Module
- IR Reciever
- IR Transmitter
- LILYGO® T-Display-S3 TF Shield

# Wiring Configuration

- CC1101_SCK  12
- CC1101_MISO 13
- CC1101_MOSI 11
- CC1101_CS   10
- GDO0        44
- GDO2        43

# Infrared (IR) Module
- IR_LED_PIN      1
- IR_RECEIVE_PIN  2

# Additional Notes:
You can modify the wiring to enable SD card support if needed.
Captive Portal: Tested and works fine. Not currently in it at the time.
Bluetooth Issues: If adding Bluetooth functionality, you may encounter compatibility issues.

# Related Resources
🔗 https://github.com/Xinyuan-LilyGO/T-Display-S3

Feel free to contribute, modify, or improve this project! 🚀
