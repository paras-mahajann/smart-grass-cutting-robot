/*
  ============================================================
  SMART GRASS CUTTING ROBOT — Arduino Mega
  Manual (Bluetooth) + Autonomous (Spiral) Mode
  ============================================================

  HARDWARE ASSUMED:
    - Arduino Mega 2560
    - 3 x BTS7960 motor drivers: LEFT drive wheel, RIGHT drive wheel,
      and CUTTER (blade) motor - each with its own RPWM/LPWM/R_EN/L_EN
    - 1 x free-swiveling front caster wheel (no control needed)
    - 1 x SC05 Bluetooth module -> wired to Serial1 (pins 18 TX1 / 19 RX1)
    - 3 x HC-SR04 ultrasonic sensors: FRONT, LEFT, RIGHT
    - Physical MANUAL/AUTO mode switch (in addition to Bluetooth commands)
    - Status LED

  WIRING:
    Driver       RPWM  LPWM  R_EN  L_EN
    Left drive    2     3     22    24
    Right drive   6     7     30    32
    Cutter        4     5     26    28

    Sensor    TRIG  ECHO
    Front      34    36
    Left       38    40
    Right      42    44

    Bluetooth SC05: Serial1 (18/19) - through a voltage divider on the
                     module's RX pin if it's 3.3V logic
    Status LED:      46
    Mode switch:     48 (INPUT_PULLUP - short to GND = AUTONOMOUS)

  BLUETOOTH COMMANDS (send as plain single characters):
    F  - Manual: drive forward (continuous, until next command)
    B  - Manual: drive backward
    L  - Manual: pivot left
    R  - Manual: pivot right
    S  - Manual: stop
    D  - Rotate 180 deg RIGHT, then stop (manual: blocking ~1.1s;
         autonomous: non-blocking, resumes spiral afterward)
    E  - Rotate 180 deg LEFT, then stop (manual: blocking ~1.1s;
         autonomous: non-blocking, resumes spiral afterward)
    C  - Blade ON
    X  - Blade OFF
    A  - Switch to AUTONOMOUS (spiral) mode
    M  - Switch to MANUAL mode (also stops motors immediately)
    0-9 - Set manual drive speed (0=slowest ~80, 9=fastest 255)

  The physical mode switch (pin 48) works alongside the Bluetooth A/M
  commands - whichever changes state last wins, so you can flip a
  physical switch on the bot itself OR use the app.

  AUTONOMOUS SPIRAL LOGIC (purely reactive - no fixed timed legs):
    - Robot drives straight continuously, blade running.
    - FRONT sensor is polled while driving. As soon as it detects the
      field boundary/an obstacle within OBSTACLE_DIST_CM, the robot
      stops and reverses slightly.
    - It then reads the LEFT and RIGHT side sensors and rotates a full
      180 deg TOWARD whichever side has more open space (larger
      reading) - so it always turns into free ground rather than
      straight back into a corner.
    - It then drives straight again until the next obstacle. Repeating
      this drive -> detect -> 180-turn-toward-free-space cycle is what
      produces the spiral/serpentine coverage of the field over time -
      there's no separate "spiral shape" being calculated, it emerges
      from always turning toward the most open side.
    - Blade motor runs for the entire autonomous session.
    - Sending 'M' or 'S' over Bluetooth, or flipping the physical switch,
      immediately breaks out and stops (checked every loop - non-blocking).
    - 'D'/'E' can also force an immediate 180 turn (right/left) mid-drive,
      non-blocking, then the loop resumes driving straight afterward.
  ============================================================
*/

// ---------------- PIN DEFINITIONS ----------------

// Left drive wheel - BTS7960
#define LEFT_RPWM   2
#define LEFT_LPWM   3
#define LEFT_R_EN   22
#define LEFT_L_EN   24

// Right drive wheel - BTS7960
#define RIGHT_RPWM  6
#define RIGHT_LPWM  7
#define RIGHT_R_EN  30
#define RIGHT_L_EN  32

// Cutter (blade) motor - BTS7960
#define BLADE_RPWM  4
#define BLADE_LPWM  5
#define BLADE_R_EN  26
#define BLADE_L_EN  28

// Ultrasonic sensors (3x HC-SR04)
#define FRONT_TRIG_PIN  38
#define FRONT_ECHO_PIN  40
#define LEFT_TRIG_PIN   36
#define LEFT_ECHO_PIN   34
#define RIGHT_TRIG_PIN  44
#define RIGHT_ECHO_PIN  42

// Status LED
#define STATUS_LED  46

// Physical mode switch (LOW = autonomous, HIGH = manual)
#define MODE_SWITCH_PIN 48

// ---------------- TUNABLE PARAMETERS ----------------

const int OBSTACLE_DIST_CM   = 30;     // stop/avoid if closer than this
const int AUTO_DRIVE_SPEED   = 150;    // 0-255 PWM speed while spiralling
const int AUTO_TURN_SPEED    = 180;    // 0-255 PWM speed while pivoting
const int BLADE_SPEED        = 50;    // 0-255 PWM speed for the cutter motor

const unsigned long TURN_180_MS    = 1350;  // calibrate: time for a 180 deg pivot
const unsigned long REVERSE_MS     = 400;   // back-up time before turning

const unsigned long SENSOR_POLL_MS = 100;   // how often to read ultrasonic

// ---------------- MODE ----------------
enum RobotMode { MANUAL_MODE, AUTONOMOUS_MODE };
RobotMode currentMode = MANUAL_MODE;
int lastSwitchState = HIGH; // tracks physical switch for edge detection

// Manual state
int manualSpeed = 200; // default manual speed (0-255)

// Autonomous state machine (purely reactive - no timed spiral legs)
enum SpiralState {
  SPIRAL_DRIVE,
  SPIRAL_RECOVER_STOP,
  SPIRAL_RECOVER_REVERSE,
  SPIRAL_RECOVER_TURN180,
  SPIRAL_MANUAL_ROTATE
};
SpiralState spiralState = SPIRAL_DRIVE;

unsigned long stateStartTime   = 0;
unsigned long lastSensorPoll   = 0;
bool recoveryTurnRight = true; // which way to pivot toward free space

// Non-blocking manual 180-rotate request (can be triggered mid-autonomous
// via Bluetooth 'D'/'E' without freezing the spiral state machine)
bool pendingManualRotate = false;
bool manualRotateDirectionRight = true;

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(9600);     // USB debug console
  Serial1.begin(9600);    // SC05 Bluetooth (default baud - check your module)

  pinMode(LEFT_RPWM, OUTPUT);
  pinMode(LEFT_LPWM, OUTPUT);
  pinMode(LEFT_R_EN, OUTPUT);
  pinMode(LEFT_L_EN, OUTPUT);
  pinMode(RIGHT_RPWM, OUTPUT);
  pinMode(RIGHT_LPWM, OUTPUT);
  pinMode(RIGHT_R_EN, OUTPUT);
  pinMode(RIGHT_L_EN, OUTPUT);
  pinMode(BLADE_RPWM, OUTPUT);
  pinMode(BLADE_LPWM, OUTPUT);
  pinMode(BLADE_R_EN, OUTPUT);
  pinMode(BLADE_L_EN, OUTPUT);

  // Enable all three BTS7960 drivers (tie enables HIGH permanently).
  // NOTE: each driver needs BOTH its EN pins pulled HIGH or it will
  // silently refuse to drive in one direction - double check wiring.
  digitalWrite(LEFT_R_EN, HIGH);
  digitalWrite(LEFT_L_EN, HIGH);
  digitalWrite(RIGHT_R_EN, HIGH);
  digitalWrite(RIGHT_L_EN, HIGH);
  digitalWrite(BLADE_R_EN, HIGH);
  digitalWrite(BLADE_L_EN, HIGH);

  pinMode(FRONT_TRIG_PIN, OUTPUT);
  pinMode(FRONT_ECHO_PIN, INPUT);
  pinMode(LEFT_TRIG_PIN, OUTPUT);
  pinMode(LEFT_ECHO_PIN, INPUT);
  pinMode(RIGHT_TRIG_PIN, OUTPUT);
  pinMode(RIGHT_ECHO_PIN, INPUT);

  pinMode(STATUS_LED, OUTPUT);
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);

  stopMotors();
  setBlade(false);

  // Set initial mode from the physical switch
  lastSwitchState = digitalRead(MODE_SWITCH_PIN);
  currentMode = (lastSwitchState == LOW) ? AUTONOMOUS_MODE : MANUAL_MODE;
  if (currentMode == AUTONOMOUS_MODE) resetSpiral();

  Serial.println(F("Smart Grass Bot ready."));
}

// ---------------- MAIN LOOP ----------------
void loop() {
  handleModeSwitch();  // check physical switch (non-blocking)
  handleBluetooth();   // always check for incoming commands (non-blocking)

  if (currentMode == AUTONOMOUS_MODE) {
    digitalWrite(STATUS_LED, HIGH);
    setBlade(true);
    runSpiralStateMachine();
  } else {
    digitalWrite(STATUS_LED, LOW);
    // Manual mode: motors already set directly by handleBluetooth(),
    // nothing else to do here - fully event driven.
  }
}

// ============================================================
// MODE SWITCH (physical toggle on pin 48)
// ============================================================
void handleModeSwitch() {
  int state = digitalRead(MODE_SWITCH_PIN);
  if (state != lastSwitchState) {
    lastSwitchState = state;
    delay(30); // simple debounce
    if (state == LOW) {
      currentMode = AUTONOMOUS_MODE;
      resetSpiral();
      Serial.println(F("Switch -> AUTONOMOUS mode"));
    } else {
      currentMode = MANUAL_MODE;
      stopMotors();
      setBlade(false);
      Serial.println(F("Switch -> MANUAL mode"));
    }
  }
}

// ============================================================
// BLUETOOTH / COMMAND HANDLING
// ============================================================
void handleBluetooth() {
  if (Serial1.available() > 0) {
    char cmd = Serial1.read();
    Serial.print(F("BT cmd: "));
    Serial.println(cmd);

    switch (cmd) {
      case 'A': // switch to autonomous
        currentMode = AUTONOMOUS_MODE;
        resetSpiral();
        Serial.println(F("-> AUTONOMOUS mode"));
        break;

      case 'M': // switch to manual
        currentMode = MANUAL_MODE;
        stopMotors();
        setBlade(false);
        Serial.println(F("-> MANUAL mode"));
        break;

      case 'F':
        if (currentMode == MANUAL_MODE) setMotors(manualSpeed, manualSpeed);
        break;
      case 'B':
        if (currentMode == MANUAL_MODE) setMotors(-manualSpeed, -manualSpeed);
        break;
      case 'L':
        if (currentMode == MANUAL_MODE) setMotors(-manualSpeed, manualSpeed);
        break;
      case 'R':
        if (currentMode == MANUAL_MODE) setMotors(manualSpeed, -manualSpeed);
        break;
      case 'S':
        if (currentMode == MANUAL_MODE) stopMotors();
        break;

      case 'D': // rotate 180 right, then stop
        if (currentMode == MANUAL_MODE) {
          rotate180Right(); // blocking - fine, nothing else needs to run
        } else if (currentMode == AUTONOMOUS_MODE) {
          pendingManualRotate = true;      // handled non-blocking in spiral loop
          manualRotateDirectionRight = true;
          Serial.println(F("180 RIGHT rotate queued for autonomous mode"));
        }
        break;
      case 'E': // rotate 180 left, then stop
        if (currentMode == MANUAL_MODE) {
          rotate180Left(); // blocking - fine, nothing else needs to run
        } else if (currentMode == AUTONOMOUS_MODE) {
          pendingManualRotate = true;      // handled non-blocking in spiral loop
          manualRotateDirectionRight = false;
          Serial.println(F("180 LEFT rotate queued for autonomous mode"));
        }
        break;

      case 'C':
        setBlade(true);
        Serial.println(F("Blade ON"));
        break;
      case 'X':
        setBlade(false);
        Serial.println(F("Blade OFF"));
        break;

      default:
        if (cmd >= '0' && cmd <= '9') {
          manualSpeed = map(cmd - '0', 0, 9, 80, 255);
          Serial.print(F("Manual speed set to "));
          Serial.println(manualSpeed);
        }
        break;
    }
  }
}

// ============================================================
// MOTOR CONTROL (BTS7960)
// Positive speed = forward, negative = reverse, range -255..255
// ============================================================
void setLeftMotor(int speed) {
  speed = constrain(speed, -255, 255);
  if (speed >= 0) {
    analogWrite(LEFT_RPWM, speed);
    analogWrite(LEFT_LPWM, 0);
  } else {
    analogWrite(LEFT_RPWM, 0);
    analogWrite(LEFT_LPWM, -speed);
  }
}

void setRightMotor(int speed) {
  speed = constrain(speed, -255, 255);
  if (speed >= 0) {
    analogWrite(RIGHT_RPWM, speed);
    analogWrite(RIGHT_LPWM, 0);
  } else {
    analogWrite(RIGHT_RPWM, 0);
    analogWrite(RIGHT_LPWM, -speed);
  }
}

void setMotors(int leftSpeed, int rightSpeed) {
  setLeftMotor(leftSpeed);
  setRightMotor(rightSpeed);
}

void stopMotors() {
  setLeftMotor(0);
  setRightMotor(0);
}

// Blade only ever needs to spin one direction
void setBlade(bool on) {
  if (on) {
    analogWrite(BLADE_RPWM, BLADE_SPEED);
    analogWrite(BLADE_LPWM, 0);
  } else {
    analogWrite(BLADE_RPWM, 0);
    analogWrite(BLADE_LPWM, 0);
  }
}

// ============================================================
// STANDALONE 180-DEGREE ROTATE FUNCTIONS
// Blocking calls: pivot in place for TURN_180_MS, then stop.
// Useful for manual mode ("spin around and stop") or as a one-shot
// utility separate from the autonomous state machine.
// ============================================================
void rotate180Right() {
  Serial.println(F("Rotating 180 deg RIGHT..."));
  setMotors(+AUTO_TURN_SPEED, AUTO_TURN_SPEED);
  delay(TURN_180_MS);
  stopMotors();
  Serial.println(F("Rotation complete - stopped."));
}

void rotate180Left() {
  Serial.println(F("Rotating 180 deg LEFT..."));
  setMotors(AUTO_TURN_SPEED, +AUTO_TURN_SPEED);
  delay(TURN_180_MS);
  stopMotors();
  Serial.println(F("Rotation complete - stopped."));
}

// ============================================================
// ULTRASONIC SENSORS
// ============================================================
long getDistanceCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 30000); // 30ms timeout
  if (duration == 0) return -1; // no echo / out of range
  return duration * 0.0343 / 2;
}

long getFrontDistance() { return getDistanceCM(FRONT_TRIG_PIN, FRONT_ECHO_PIN); }
long getLeftDistance()  { return getDistanceCM(LEFT_TRIG_PIN, LEFT_ECHO_PIN); }
long getRightDistance() { return getDistanceCM(RIGHT_TRIG_PIN, RIGHT_ECHO_PIN); }

// ============================================================
// AUTONOMOUS SPIRAL STATE MACHINE (non-blocking)
// ============================================================
void resetSpiral() {
  spiralState     = SPIRAL_DRIVE;
  stateStartTime  = millis();
  Serial.println(F("Resuming straight drive."));
}

void runSpiralStateMachine() {
  unsigned long now = millis();

  // A manual 180 rotate can be requested at any time via Bluetooth ('D'/'E')
  // even mid-spiral - preempt whatever state we're in and handle it here,
  // non-blocking, so sensors/Bluetooth stay responsive during the turn.
  if (pendingManualRotate) {
    pendingManualRotate = false;
    stopMotors();
    spiralState = SPIRAL_MANUAL_ROTATE;
    stateStartTime = now;
  }

  // Poll FRONT ultrasonic periodically while driving forward (not while turning)
  bool obstacleDetected = false;
  if (spiralState == SPIRAL_DRIVE && now - lastSensorPoll >= SENSOR_POLL_MS) {
    lastSensorPoll = now;
    long dist = getFrontDistance();
    if (dist > 0 && dist < OBSTACLE_DIST_CM) {
      obstacleDetected = true;
    }
  }

  switch (spiralState) {

    case SPIRAL_DRIVE:
      if (obstacleDetected) {
        Serial.println(F("Obstacle/boundary detected ahead - starting turn."));
        stopMotors();
        spiralState = SPIRAL_RECOVER_STOP;
        stateStartTime = now;
        break;
      }
      // No fixed leg length - just keep driving straight until something
      // is detected ahead. The 180-turn-toward-free-space below is what
      // produces the spiral/serpentine coverage over time.
      setMotors(AUTO_DRIVE_SPEED, AUTO_DRIVE_SPEED);
      break;

    case SPIRAL_RECOVER_STOP:
      stopMotors();
      if (now - stateStartTime >= 150) {
        // Check side sensors to decide which way has more open space -
        // pivot toward the clearer side instead of a fixed direction.
        long leftDist  = getLeftDistance();
        long rightDist = getRightDistance();
        // Treat "no echo" (-1) as wide open space.
        if (leftDist < 0) leftDist = 999;
        if (rightDist < 0) rightDist = 999;
        recoveryTurnRight = (rightDist >= leftDist);

        Serial.print(F("Left clearance: "));
        Serial.print(leftDist);
        Serial.print(F(" cm, Right clearance: "));
        Serial.print(rightDist);
        Serial.print(F(" cm -> turning "));
        Serial.println(recoveryTurnRight ? F("RIGHT") : F("LEFT"));

        spiralState = SPIRAL_RECOVER_REVERSE;
        stateStartTime = now;
      }
      break;

    case SPIRAL_RECOVER_REVERSE:
      setMotors(-AUTO_DRIVE_SPEED, -AUTO_DRIVE_SPEED);
      if (now - stateStartTime >= REVERSE_MS) {
        stopMotors();
        spiralState = SPIRAL_RECOVER_TURN180;
        stateStartTime = now;
      }
      break;

    case SPIRAL_RECOVER_TURN180:
      // Smooth 180 pivot enabled by the free front caster wheel -
      // direction chosen in SPIRAL_RECOVER_STOP based on side clearance
      if (recoveryTurnRight) {
        setMotors(AUTO_TURN_SPEED, -AUTO_TURN_SPEED);
      } else {
        setMotors(-AUTO_TURN_SPEED, AUTO_TURN_SPEED);
      }
      if (now - stateStartTime >= TURN_180_MS) {
        stopMotors();
        Serial.println(F("Turn complete - driving straight again."));
        spiralState = SPIRAL_DRIVE;
        stateStartTime = now;
      }
      break;

    case SPIRAL_MANUAL_ROTATE:
      // Non-blocking version of rotate180Right()/rotate180Left() - lets the
      // spiral loop keep running (millis()-based) instead of freezing on delay()
      if (manualRotateDirectionRight) {
        setMotors(AUTO_TURN_SPEED, -AUTO_TURN_SPEED);
      } else {
        setMotors(-AUTO_TURN_SPEED, AUTO_TURN_SPEED);
      }
      if (now - stateStartTime >= TURN_180_MS) {
        stopMotors();
        Serial.println(F("Manual 180 rotate complete - driving straight again."));
        spiralState = SPIRAL_DRIVE;
        stateStartTime = now;
      }
      break;
  }
}

