# Pin Layout and Wiring

Board: **Arduino Mega 2560**. All values match the `#define`s in `firmware/smart_grass_bot.ino`.

## Motor Drivers (3x BTS7960)

Each driver uses four control pins. Both enable pins (R_EN and L_EN) are set HIGH in `setup()`. If either one is not wired, the driver will not run in one direction.

| Driver | Function | RPWM | LPWM | R_EN | L_EN |
|---|---|---|---|---|---|
| Left drive | Left wheel | 2 | 3 | 22 | 24 |
| Right drive | Right wheel | 6 | 7 | 30 | 32 |
| Cutter | Blade motor | 4 | 5 | 26 | 28 |

- RPWM drives the motor forward, LPWM drives it in reverse.
- The blade motor only spins one way (RPWM), so its LPWM is always 0.
- PWM pins used: 2, 3, 4, 5, 6, 7 (all PWM-capable on the Mega).

## Ultrasonic Sensors (3x HC-SR04)

| Sensor | Position | TRIG | ECHO |
|---|---|---|---|
| Front | Detects boundary or obstacle ahead | 38 | 40 |
| Left | Measures free space on the left | 36 | 34 |
| Right | Measures free space on the right | 44 | 42 |

- HC-SR04 runs at 5V, so no level shifting is needed with the Mega.
- Readings time out at 30 ms. No echo is treated as open space.

## Bluetooth Module (SC05)

| Module pin | Arduino Mega pin |
|---|---|
| TX | RX1 (pin 19) |
| RX | TX1 (pin 18), through a voltage divider if the module uses 3.3V logic |
| VCC / GND | Per module datasheet (check the voltage before powering) |

- Port: `Serial1` at 9600 baud. Check your module's default baud rate.
- Connect module TX to Arduino RX and module RX to Arduino TX (crossed).

## Other Inputs and Outputs

| Item | Pin | Notes |
|---|---|---|
| Status LED | 46 | HIGH while in autonomous mode, LOW in manual |
| Mode switch | 48 | `INPUT_PULLUP`. Connect to GND = autonomous, open = manual |

## Power Notes

- Share a common ground between the Arduino, the three BTS7960 drivers and the Bluetooth module.
- Power the motors from the battery through the BTS7960 B+ and B- terminals, not from the Arduino.
- Do not power the motors from the Mega's 5V pin.

## Full Pin Map

| Pin | Use |
|---|---|
| 2 | Left drive RPWM |
| 3 | Left drive LPWM |
| 4 | Cutter RPWM |
| 5 | Cutter LPWM |
| 6 | Right drive RPWM |
| 7 | Right drive LPWM |
| 18 | TX1 (to Bluetooth RX) |
| 19 | RX1 (from Bluetooth TX) |
| 22 | Left drive R_EN |
| 24 | Left drive L_EN |
| 26 | Cutter R_EN |
| 28 | Cutter L_EN |
| 30 | Right drive R_EN |
| 32 | Right drive L_EN |
| 34 | Left sensor ECHO |
| 36 | Left sensor TRIG |
| 38 | Front sensor TRIG |
| 40 | Front sensor ECHO |
| 42 | Right sensor ECHO |
| 44 | Right sensor TRIG |
| 46 | Status LED |
| 48 | Mode switch |
