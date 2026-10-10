# User Manual

How to operate the Smart Grass Bot safely and get consistent results.

## Safety First

- Keep hands, feet and loose clothing away from the blade at all times.
- Send `X` (blade off) and `M` (manual, stops everything) before touching the robot.
- Disconnect the battery before any wiring or blade work.
- Test on open ground first, with the blade off (`X`), until you trust the behavior.
- Keep people and pets away from the cutting area.

## Powering On

1. Place the robot on open ground with at least 1 m of clear space ahead.
2. Set the mode switch to **manual** (open, not connected to GND).
3. Connect the battery. The Serial Monitor (9600 baud, if connected by USB) shows `Smart Grass Bot ready.`
4. The status LED is off in manual mode and on in autonomous mode.

## Connecting from the Phone

1. Pair the SC05 Bluetooth module in your phone's Bluetooth settings.
2. Open the MIT App Inventor app, or any Bluetooth serial terminal app.
3. Connect to the module and send single characters.
4. If the app shows error 507, use a third-party Bluetooth serial terminal instead (see Troubleshooting).

## Manual Mode

| Send | Result |
|---|---|
| `F` / `B` | Drive forward / backward until the next command |
| `L` / `R` | Pivot left / right in place |
| `S` | Stop |
| `D` / `E` | Rotate 180 degrees right / left, then stop |
| `C` / `X` | Blade on / off |
| `0` to `9` | Drive speed (0 is slowest, about 80 PWM; 9 is fastest, 255 PWM) |
| `K` then `0` to `9` | Cutter speed (0 is off, 9 is full) |

Notes:
- The robot keeps doing the last drive command until you send another one, so send `S` when you want it to stop.
- `D` and `E` block for about 1.35 seconds while rotating.
- Entering manual mode with `M` stops the motors and turns the blade off.

## Autonomous Mode

Start it with `A`, or by flipping the mode switch to GND.

What the robot does:
1. Drives straight with the blade running.
2. Stops when the front sensor sees something closer than 30 cm.
3. Compares the left and right sensors and picks the side with more open space.
4. Reverses briefly, then turns 180 degrees toward that side.
5. Drives straight again and repeats.

To stop: send `M` or `S`, or flip the switch back to manual.

You can still send `D` or `E` during autonomous mode to force a 180 degree turn. The robot then resumes the cycle.

You can also change the cutter speed with `K` + digit while it runs.

## Setting the Cutter Speed

The cutter starts at a PWM of 50 until you change it. Send `K` followed by a digit:

| Command | Approx. PWM |
|---|---|
| `K0` | 0 |
| `K3` | 85 |
| `K5` | 142 |
| `K9` | 255 |

The change applies immediately, in both modes.

## Calibrating the 180 Degree Turn

The turn time (`TURN_180_MS`, default 1350 ms) changes with battery level, wheel grip and ground surface.

1. In manual mode, send `D` with the robot on the ground where it will work.
2. Observe the rotation. Mark the starting heading with tape.
3. If it turns less than 180 degrees, increase `TURN_180_MS` by 50 to 100 ms. If it turns more, decrease it.
4. Re-upload the firmware and repeat until it ends facing the opposite way.
5. Record the final value in `docs/testing_log.md`.

## Tuning Obstacle Detection

| Parameter | Effect of increasing it |
|---|---|
| `OBSTACLE_DIST_CM` | Robot reacts earlier, further from the boundary |
| `AUTO_DRIVE_SPEED` | Faster coverage, less time to react |
| `REVERSE_MS` | Backs up further before turning |

Tall grass or sloping ground can confuse ultrasonic sensors. Test before relying on the settings.

## Shutting Down

1. Send `X` and `M`.
2. Disconnect the battery.
3. Clean the blade area only after the battery is disconnected.
