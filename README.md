# RC-CAR

A Wi-Fi controller for an old Toyota Tacoma RC car using an ESP32 and a dual H-bridge motor driver.

> The browser cannot drive the original radio receiver directly. This project replaces or electrically bypasses the car's receiver and connects the motor/steering outputs to an ESP32 through a motor driver. Do not connect motors directly to ESP32 GPIO pins.

## Hardware

- ESP32 development board
- Dual H-bridge driver rated for the car's motor voltage/current (TB6612FNG is preferred; an L298N also works with more heat loss)
- Separate battery supply for the motors
- Common ground between ESP32 and motor-driver logic ground
- The car's drive motor and steering motor/servo

The default pin map is in `firmware/RC_CAR.ino`. Change it before flashing. The example assumes:

- Drive motor: one H-bridge channel
- Steering motor: the second H-bridge channel
- The steering mechanism is a two-wire motor. A three-wire servo needs a servo driver instead.

## Upload and use

1. Install the ESP32 Arduino board package and open `firmware/RC_CAR.ino`.
2. Change `MOTOR_BATTERY_VOLTAGE` and the GPIO pins if required.
3. Flash the sketch to the ESP32.
4. Connect the car battery and motor driver according to its datasheet. Keep the wheels off the ground for the first test.
5. Join the Wi-Fi network `RC-Car` using password `rc-car-1234`.
6. Open `http://192.168.4.1` in a browser. The control page can be opened in a separate tab or on another device connected to the same network.
7. Hold the on-screen buttons, arrow keys, or WASD to move. Releasing the control stops the output.

## Safety

- The firmware has a 500 ms communication watchdog and stops both outputs if commands stop arriving.
- Test with the drive wheels lifted first, then use low speed.
- Add a physical battery disconnect or emergency stop.
- Never power the motors from the ESP32 3.3 V pin.
- A real vehicle can move unexpectedly; keep people, pets, and obstacles away.

## Network note

The ESP32 starts its own access point, so no cloud service is required. This is intentionally local-only. If you later put it on a home network, add authentication and do not expose the control endpoint to the internet.
