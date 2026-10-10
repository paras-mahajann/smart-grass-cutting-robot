# Testing and Calibration Log

Fill this in during tests. Real measurements here also serve as the experimental data for the research paper.

Test setup:
- Date:
- Tester(s):
- Location / surface (grass height, slope, dry or wet):
- Battery voltage at start:
- Firmware version / commit:

## 1. Turn Calibration

| Trial | `TURN_180_MS` | Battery (V) | Surface | Measured angle (deg) | Notes |
|---|---|---|---|---|---|
| 1 | 1350 | | | | |
| 2 | | | | | |
| 3 | | | | | |

Final value used:

## 2. Obstacle Detection

| Trial | Obstacle type | Actual distance (cm) | Reading (cm) | Stopped in time? (Y/N) | Notes |
|---|---|---|---|---|---|
| 1 | | | | | |
| 2 | | | | | |
| 3 | | | | | |
| 4 | | | | | |
| 5 | | | | | |

## 3. Turn Direction Decision

| Trial | Left clearance (cm) | Right clearance (cm) | Expected direction | Actual direction | Correct? |
|---|---|---|---|---|---|
| 1 | | | | | |
| 2 | | | | | |
| 3 | | | | | |

## 4. Bluetooth Range and Response

| Distance (m) | Obstacles between | Commands sent | Commands received | Lag (approx. s) |
|---|---|---|---|---|
| 1 | | | | |
| 5 | | | | |
| 10 | | | | |

## 5. Coverage Test (Autonomous)

| Trial | Area (m x m) | Run time (min) | Area covered (%) | Turns | Stuck / failed? | Notes |
|---|---|---|---|---|---|---|
| 1 | | | | | | |
| 2 | | | | | | |
| 3 | | | | | | |

How coverage was measured (photo, grid count, etc.):

## 6. Cutter Performance

| Cutter level (`K` digit) | PWM | Grass height before (cm) | Grass height after (cm) | Motor current (A) | Notes |
|---|---|---|---|---|---|
| K3 | 85 | | | | |
| K5 | 142 | | | | |
| K9 | 255 | | | | |

## 7. Battery Runtime

| Trial | Start (V) | End (V) | Run time (min) | Mode | Notes |
|---|---|---|---|---|---|
| 1 | | | | | |
| 2 | | | | | |

## 8. Comparison with Manual Mowing (optional, for the paper)

| Method | Area (m^2) | Time (min) | Evenness of cut | Notes |
|---|---|---|---|---|
| Robot, autonomous | | | | |
| Manual mower | | | | |

## Issues Found

| Date | Issue | Cause | Fix | Resolved? |
|---|---|---|---|---|
| | | | | |
