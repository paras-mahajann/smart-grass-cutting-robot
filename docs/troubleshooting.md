# Troubleshooting

## Motors

| Symptom | Likely cause | Fix |
|---|---|---|
| A motor only works in one direction | One BTS7960 enable pin (R_EN or L_EN) is not wired | Check both enable pins of that driver reach the Mega (22/24, 30/32, 26/28) |
| A motor does not move at all | No battery power to the driver, or loose B+ / B- | Check the power terminals and that the grounds are shared |
| Robot veers to one side | Motor speeds or wheel friction differ | Check wheel grip, then compensate in `setMotors()` if needed |
| `D` / `E` drives forward instead of spinning | Older firmware with same-sign wheel speeds | Use the corrected firmware where `D` and `E` use opposite signs |
| Pivot turns wrong direction | A motor is wired reversed | Swap that motor's two leads, or swap the sign in the code |
| Motors stutter | Weak battery or loose connection | Test with a charged battery and recheck wiring |

## Bluetooth

| Symptom | Likely cause | Fix |
|---|---|---|
| Module will not pair | Wrong PIN or already paired elsewhere | Try `1234` or `0000`; unpair from other devices |
| Paired but no response | TX / RX not crossed, or wrong baud rate | Module TX to Mega RX1 (19); module RX to Mega TX1 (18). Confirm 9600 baud |
| Garbled commands | Baud mismatch | Match the module's baud rate to `Serial1.begin()` |
| Random behavior | RX pin overvoltage or noise | Use a voltage divider on the module RX if it is 3.3V logic |
| Cutter speed changes unexpectedly | A stray `K` waiting for its digit | Send a digit to complete it, or send `M` |

### MIT App Inventor error 507

This was seen on a Moto Edge 60 Pro. It appears to be a platform-level problem, not a bug in the blocks.

Options:
1. Use a third-party Bluetooth serial terminal app. It sends the same single characters.
2. Ask in the MIT App Inventor community forum, with the device model and the app's .aia file.
3. Test the app on a different phone to confirm it is device-specific.

## Sensors

| Symptom | Likely cause | Fix |
|---|---|---|
| Robot never turns | Front sensor reads -1 (no echo) | Check front TRIG 38 and ECHO 40, and 5V power to the sensor |
| Turns too late or too early | `OBSTACLE_DIST_CM` too small or large | Adjust the parameter |
| Always picks the same side | A side sensor is disconnected or reads -1 (treated as open) | Check left (36/34) and right (44/42) sensors |
| False detections | Tall grass or uneven ground in front of the sensor | Angle the sensor and test over real grass |

Use the Serial Monitor at 9600 baud. The firmware prints the left and right clearance and the chosen turn direction.

## Mode and Switch

| Symptom | Likely cause | Fix |
|---|---|---|
| Starts autonomous on power-up | Mode switch is closed to GND | Open the switch before powering on |
| Status LED never lights | LED polarity or resistor | Check pin 46, polarity and a series resistor |
| Mode flips on its own | Floating or noisy switch wire | Use short wires and check the ground connection |

## Upload Problems

| Symptom | Fix |
|---|---|
| Upload fails | Select Arduino Mega or Mega 2560 and the right COM port |
| Upload fails with Bluetooth connected | Disconnect pins 18 and 19 during upload if needed |
| Compile errors | Update the Arduino IDE and recopy the full sketch |
