#include <Servo.h>

// --- Pin Definitions ---
const int BASE_PIN = 3;
const int SHOULDER_PIN = 5;
const int GRIPPER_PIN = 6;

// --- Servo Objects ---
Servo baseServo;
Servo shoulderServo;
Servo gripperServo;

// --- Servo Angles (track current positions) ---
int basePos = 90;
int shoulderPos = 90;
int gripperPos = 90;

// --- Limits & presets ---
const int BASE_MIN = 0;
const int BASE_MAX = 180;
const int SHOULDER_MIN = 20;
const int SHOULDER_MAX = 180;
const int GRIP_OPEN = 60;
const int GRIP_CLOSE = 180;
const int MOVE_DELAY = 8; // ms delay per degree for smooth motion

// --- Smooth servo movement ---
void moveServoSmooth(Servo &servo, int &currentPos, int targetPos, int stepDelay = MOVE_DELAY)
{
  targetPos = constrain(targetPos, 0, 180);

  if (currentPos == targetPos) return;

  if (currentPos < targetPos) {
    for (int p = currentPos; p <= targetPos; p++) {
      servo.write(p);
      delay(stepDelay);
    }
  } else {
    for (int p = currentPos; p >= targetPos; p--) {
      servo.write(p);
      delay(stepDelay);
    }
  }

  currentPos = targetPos;
}

// --- Display current servo states ---
void printStatus() {
  Serial.print("Base: "); Serial.print(basePos);
  Serial.print(" | Shoulder: "); Serial.print(shoulderPos);
  Serial.print(" | Gripper: "); Serial.println(gripperPos);
}

// --- Display available commands ---
void printGuide() {
  Serial.println(F("==========================================="));
  Serial.println(F("3-SERVO ROBOT ARM CONTROL (Arduino UNO)"));
  Serial.println(F("==========================================="));
  Serial.println(F("Individual absolute control:"));
  Serial.println(F(" BASE <angle>"));
  Serial.println(F(" SHOULDER <angle>"));
  Serial.println(F(" GRIPPER <angle>"));
  Serial.println(F("Relative base control:"));
  Serial.println(F(" RIGHT <angle>"));
  Serial.println(F(" LEFT <angle>"));
  Serial.println(F("Gripper control:"));
  Serial.println(F(" OPEN"));
  Serial.println(F(" CLOSE"));
  Serial.println(F("Combined motion macros:"));
  Serial.println(F(" RISE_CLOSE"));
  Serial.println(F(" RISE_OPEN"));
  Serial.println(F(" DOWN_CLOSE"));
  Serial.println(F(" DOWN_OPEN"));
  Serial.println(F("Utility:"));
  Serial.println(F(" CENTER"));
  Serial.println(F(" STATUS"));
  Serial.println(F("==========================================="));
}

// --- Command Processor ---
void processCommand(const String &cmdIn, const String &argIn) {
  String cmd = cmdIn;
  cmd.toUpperCase();
  String arg = argIn;
  arg.trim();

  if (cmd == "BASE") {
    moveServoSmooth(baseServo, basePos, constrain(arg.toInt(), BASE_MIN, BASE_MAX));
  } else if (cmd == "RIGHT") {
    moveServoSmooth(baseServo, basePos, constrain(basePos + arg.toInt(), BASE_MIN, BASE_MAX));
  } else if (cmd == "LEFT") {
    moveServoSmooth(baseServo, basePos, constrain(basePos - arg.toInt(), BASE_MIN, BASE_MAX));
  } else if (cmd == "SHOULDER") {
    moveServoSmooth(shoulderServo, shoulderPos, constrain(arg.toInt(), SHOULDER_MIN, SHOULDER_MAX));
  } else if (cmd == "GRIPPER") {
    moveServoSmooth(gripperServo, gripperPos, constrain(arg.toInt(), 0, 180));
  } else if (cmd == "OPEN") {
    moveServoSmooth(gripperServo, gripperPos, GRIP_OPEN);
  } else if (cmd == "CLOSE") {
    moveServoSmooth(gripperServo, gripperPos, GRIP_CLOSE);
  } else if (cmd == "RISE_CLOSE") {
    moveServoSmooth(shoulderServo, shoulderPos, 20);
    moveServoSmooth(gripperServo, gripperPos, GRIP_CLOSE);
  } else if (cmd == "RISE_OPEN") {
    moveServoSmooth(shoulderServo, shoulderPos, 20);
    moveServoSmooth(gripperServo, gripperPos, GRIP_OPEN);
  } else if (cmd == "DOWN_CLOSE") {
    moveServoSmooth(shoulderServo, shoulderPos, 70);
    moveServoSmooth(gripperServo, gripperPos, GRIP_CLOSE);
  } else if (cmd == "DOWN_OPEN") {
    moveServoSmooth(shoulderServo, shoulderPos, 70);
    moveServoSmooth(gripperServo, gripperPos, GRIP_OPEN);
  } else if (cmd == "CENTER") {
    moveServoSmooth(baseServo, basePos, 90);
    moveServoSmooth(shoulderServo, shoulderPos, 40);
    moveServoSmooth(gripperServo, gripperPos, 170);
  } else if (cmd == "STATUS") {
    printStatus();
  } else {
    Serial.print("Unknown command: ");
    Serial.println(cmd);
  }
}

void setup() {
  Serial.begin(9600);

  baseServo.attach(BASE_PIN);
  shoulderServo.attach(SHOULDER_PIN);
  gripperServo.attach(GRIPPER_PIN);

  baseServo.write(basePos);
  shoulderServo.write(shoulderPos);
  gripperServo.write(gripperPos);

  delay(200);
  printGuide();
}

void loop() {
  if (!Serial.available()) return;

  String line = Serial.readStringUntil('\n');
  line.trim();
  if (line.length() == 0) return;

  char buf[200];
  line.toCharArray(buf, sizeof(buf));

  char *saveptr = NULL;
  char *token = strtok_r(buf, " \t\r\n", &saveptr);

  while (token != NULL) {
    String cmd = String(token);
    cmd.toUpperCase();

    if (cmd == "BASE" || cmd == "RIGHT" || cmd == "LEFT" ||
        cmd == "SHOULDER" || cmd == "GRIPPER") {

      char *next = strtok_r(NULL, " \t\r\n", &saveptr);
      if (next == NULL) {
        Serial.print("Missing angle for ");
        Serial.println(cmd);
        break;
      }

      processCommand(cmd, String(next));
      token = strtok_r(NULL, " \t\r\n", &saveptr);
      continue;
    }

    processCommand(cmd, "");
    token = strtok_r(NULL, " \t\r\n", &saveptr);
  }

  Serial.println("Sequence complete.");
  printStatus();
  Serial.println("-------------------------------------------");
}
