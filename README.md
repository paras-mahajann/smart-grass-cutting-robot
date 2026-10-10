# Smart Grass Bot

An autonomous grass-cutting robot built as a final-year Computer Engineering project at R. C. Patel Institute of Technology, Shirpur. It supports manual control over Bluetooth and a reactive autonomous mode with ultrasonic obstacle avoidance that covers the field in a spiral/serpentine pattern.

> Project Guide: Dr. S. R. Sonawane
> Intended client: Ahilya Lawns
> Academic year: 2026-27

## Features

- Manual drive control from a smartphone over Bluetooth (SC05 module on Serial1)
- Autonomous mode: drives straight, detects the field boundary or an obstacle with the front sensor, reverses slightly, then turns 180 degrees toward the side with more open space
- Adjustable drive speed (10 levels) and adjustable cutter speed (10 levels) from the app
- Blade motor control (on/off) in both modes
- Physical MANUAL/AUTO mode switch alongside the Bluetooth commands
- Non-blocking state machine, so Bluetooth commands and sensors stay responsive during autonomous turns
- Companion Android app built in MIT App Inventor

## Hardware

| Component | Qty | Purpose |
|---|---|---|
| Arduino Mega 2560 | 1 | Main controller |
| BTS7960 motor driver | 3 | Left wheel, right wheel, cutter (blade) motor |
| HC-SR04 ultrasonic sensor | 3 | Front, left and right obstacle detection |
| SC05 Bluetooth module | 1 | Wireless control (Serial1, 9600 baud) |
| Free-swiveling front caster wheel | 1 | Passive support, enables smooth pivot turns |
| MANUAL/AUTO toggle switch | 1 | Physical mode selection |
| Status LED | 1 | Lit while in autonomous mode |

> The SC05 RX pin needs a voltage divider if the module uses 3.3V logic.

### Pin Configuration

| Driver | RPWM | LPWM | R_EN | L_EN |
|---|---|---|---|---|
| Left drive wheel | 2 | 3 | 22 | 24 |
| Right drive wheel | 6 | 7 | 30 | 32 |
| Cutter (blade) | 4 | 5 | 26 | 28 |

| Sensor | TRIG | ECHO |
|---|---|---|
| Front | 38 | 40 |
| Left | 36 | 34 |
| Right | 44 | 42 |

| Other | Pin |
|---|---|
| Bluetooth SC05 | Serial1 (TX1 = 18, RX1 = 19) |
| Status LED | 46 |
| Mode switch | 48 (INPUT_PULLUP, short to GND = autonomous) |

Both enable pins (R_EN and L_EN) of each BTS7960 are tied HIGH in software. If one is not connected, the driver will not run in one direction.

## How It Works

**Manual mode:** The app sends single-character commands over Bluetooth and the firmware maps each one to a motor action. The robot keeps doing the last command until the next one arrives.

**Autonomous mode:**
1. The robot drives straight with the blade running.
2. The front sensor is polled every 100 ms. When something is closer than 30 cm, the robot stops.
3. It reads the left and right sensors and picks the side with more open space.
4. It reverses briefly, then pivots 180 degrees toward that side.
5. It drives straight again. Repeating this cycle produces the spiral/serpentine coverage of the field.

Sending `M` or `S`, or flipping the physical switch, breaks out and stops immediately.

## Repository Structure

```
smart-grass-bot/
├── firmware/
│   └── smart_grass_bot.ino
├── app/
│   ├── SmartGrass.aia
│   └── screenshots/
├── docs/
│   ├── client_report.pdf
│   ├── pin_layout.md
│   ├── user_manual.md
│   ├── troubleshooting.md
│   └── testing_log.md
├── images/
├── README.md
├── LICENSE
└── .gitignore
```

## Documentation

- [Pin layout and wiring](docs/pin_layout.md)
- [User manual](docs/user_manual.md): operation, speed levers, turn calibration
- [Troubleshooting](docs/troubleshooting.md): motors, Bluetooth, sensors, app error 507
- [Testing and calibration log](docs/testing_log.md): test data template

## Getting Started

1. Install the Arduino IDE.
2. Open `firmware/smart_grass_bot.ino`.
3. Select **Tools > Board > Arduino Mega or Mega 2560**.
4. Connect the hardware as described in the pin tables above.
5. Upload the sketch.
6. Pair the Bluetooth module with your phone.
7. Open the Serial Monitor at 9600 baud to see debug messages.
8. Control the robot from the app or any Bluetooth serial terminal.

## Bluetooth Commands

Send plain single characters.

| Command | Action |
|---|---|
| `F` | Manual: drive forward (until next command) |
| `B` | Manual: drive backward |
| `L` | Manual: pivot left |
| `R` | Manual: pivot right |
| `S` | Manual: stop |
| `D` | Rotate 180 degrees right, then stop (blocking in manual; non-blocking in autonomous, then resumes) |
| `E` | Rotate 180 degrees left, then stop (same behavior as `D`) |
| `C` | Blade ON |
| `X` | Blade OFF |
| `A` | Switch to autonomous mode |
| `M` | Switch to manual mode (also stops motors and blade) |
| `0` to `9` | Set drive speed lever (0 = slowest, about 80 PWM; 9 = fastest, 255 PWM) |
| `K` then `0` to `9` | Set cutter speed (two bytes; 0 = 0 PWM, 9 = 255 PWM), applies immediately in both modes |

## Tunable Parameters

| Parameter | Default | Meaning |
|---|---|---|
| `OBSTACLE_DIST_CM` | 30 | Distance at which the robot reacts |
| `AUTO_DRIVE_SPEED` | 150 | PWM speed while driving autonomously |
| `AUTO_TURN_SPEED` | 180 | PWM speed while pivoting |
| `DEFAULT_BLADE_SPEED` | 50 | Initial cutter speed until changed from the app |
| `TURN_180_MS` | 1350 | Time for a 180 degree pivot (calibrate on your surface) |
| `REVERSE_MS` | 400 | Back-up time before turning |
| `SENSOR_POLL_MS` | 100 | Front sensor polling interval |

## Known Issues

- The MIT App Inventor companion app shows error 507 on some devices (seen on Moto Edge 60 Pro). Workaround: use a third-party Bluetooth serial controller app.
- The turn time (`TURN_180_MS`) depends on battery level and ground surface, so recalibrate when conditions change.

## Team

| Name | Role |
|---|---|
| Paras Mahajan | Firmware, documentation |
| TODO | TODO |
| TODO | TODO |
| TODO | TODO |

## Future Work

- Path planning for full-area coverage
- Additional sensors for edge and slope detection
- Battery monitoring and automatic return to charging point

## License

Released under the MIT License. See `LICENSE`.
