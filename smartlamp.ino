#include "BluetoothSerial.h"
#include <Arduino.h>
#define IR_USE_ESP32_CHANNEL_ALLOCATOR
#include <IRremote.hpp>

BluetoothSerial SerialBT;

// RGB LED pins and PWM channels
const int redPin = 27;
const int greenPin = 26;
const int bluePin = 14;
const int redChannel = 6;
const int greenChannel = 7;
const int blueChannel = 8;

// Inputs
const int joyXPin = 32;
const int joyYPin = 33;
const int joyButton = 13;
const int potPin = 34;

// IR translator
const int IR_RECEIVE_PIN = 15;
const int IR_SEND_PIN = 4;

enum Mode { AUTO_RAINBOW, MANUAL_JOYSTICK, PURE_WHITE, LAMP_OFF, BLUETOOTH_MODE };
Mode currentMode = AUTO_RAINBOW;

unsigned long lastRainbowUpdate = 0;
const int rainbowDelay = 15;
int autoHue = 0;
int lastManualHue = 0;

bool lastJoyBtnState = HIGH;
unsigned long lastJoyDebounce = 0;
const unsigned long debounceDelay = 50;

unsigned long lastAnalogReadTime = 0;
const unsigned long analogSampleInterval = 40;
float currentBrightnessMultiplier = 1.0;

unsigned long lastRemoteClickTime = 0;
const unsigned long remoteDebounceDelay = 50;

const uint16_t SAMSUNG_ADDRESS = 0x0707;
const uint8_t SAMSUNG_POWER_CMD = 0x02;
const uint8_t SAMSUNG_HOME_CMD = 0x79;
const uint8_t SAMSUNG_SELECT_CMD = 0x68;
const uint8_t SAMSUNG_RETURN_CMD = 0x58;
const uint16_t SAMSUNG_LEFT_CMD = 0x65;
const uint16_t SAMSUNG_RIGHT_CMD = 0x62;
const uint8_t SAMSUNG_UP_CMD = 0x60;
const uint8_t SAMSUNG_DOWN_CMD = 0x61;
const uint8_t SAMSUNG_VOLUP_CMD = 0x07;
const uint8_t SAMSUNG_VOLDN_CMD = 0x0B;

void setRainbowColor(int hue, float dimFactor);

void sendSamsungCommand(uint16_t command) {
  IrReceiver.stop();
  IrSender.sendSamsung48(SAMSUNG_ADDRESS, command, 3);
  delay(100);
  IrReceiver.start();
}

void setup() {
  Serial.begin(9600);

  pinMode(joyXPin, INPUT);
  pinMode(joyYPin, INPUT);
  pinMode(joyButton, INPUT_PULLUP);
  pinMode(potPin, INPUT);

  ledcSetup(redChannel, 5000, 8);
  ledcSetup(greenChannel, 5000, 8);
  ledcSetup(blueChannel, 5000, 8);
  ledcAttachPin(redPin, redChannel);
  ledcAttachPin(greenPin, greenChannel);
  ledcAttachPin(bluePin, blueChannel);

  SerialBT.begin("ESP32-Lamp-Remote");
  IrReceiver.begin(IR_RECEIVE_PIN, DISABLE_LED_FEEDBACK);
  IrSender.begin(IR_SEND_PIN, DISABLE_LED_FEEDBACK);
}

void loop() {
  unsigned long currentTime = millis();

  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.protocol == decode_type_t::NEC) {
      uint16_t elegooCommand = IrReceiver.decodedIRData.command;
      Serial.println(elegooCommand, HEX);

      if (currentTime - lastRemoteClickTime >= remoteDebounceDelay) {
        lastRemoteClickTime = currentTime;

        if (elegooCommand == 0x45 || elegooCommand == 0x00) {
          sendSamsungCommand(SAMSUNG_POWER_CMD);
        } else if (elegooCommand == 0x40) {
          sendSamsungCommand(SAMSUNG_SELECT_CMD);
        } else if (elegooCommand == 0x47) {
          sendSamsungCommand(SAMSUNG_RETURN_CMD);
        } else if (elegooCommand == 0x44) {
          sendSamsungCommand(SAMSUNG_LEFT_CMD);
        } else if (elegooCommand == 0x43) {
          sendSamsungCommand(SAMSUNG_RIGHT_CMD);
        } else if (elegooCommand == 0x46) {
          sendSamsungCommand(SAMSUNG_UP_CMD);
        } else if (elegooCommand == 0x15) {
          sendSamsungCommand(SAMSUNG_DOWN_CMD);
        } else if (elegooCommand == 0x19) {
          sendSamsungCommand(SAMSUNG_HOME_CMD);
        } else if (elegooCommand == 0x42) {
          if (currentMode == BLUETOOTH_MODE || currentMode == PURE_WHITE || currentMode == LAMP_OFF) {
            currentMode = AUTO_RAINBOW;
          } else {
            sendSamsungCommand(SAMSUNG_VOLUP_CMD);
          }
        } else if (elegooCommand == 0x4A) {
          if (currentMode == BLUETOOTH_MODE || currentMode == PURE_WHITE || currentMode == LAMP_OFF) {
            currentMode = AUTO_RAINBOW;
          } else {
            sendSamsungCommand(SAMSUNG_VOLDN_CMD);
          }
          sendSamsungCommand(SAMSUNG_VOLUP_CMD);
        } else if (elegooCommand == 0x0C) {
          currentMode = BLUETOOTH_MODE;
          setRainbowColor(0, currentBrightnessMultiplier);
        } else if (elegooCommand == 0x18) {
          currentMode = BLUETOOTH_MODE;
          setRainbowColor(13, currentBrightnessMultiplier);
        } else if (elegooCommand == 0x5E) {
          currentMode = BLUETOOTH_MODE;
          setRainbowColor(42, currentBrightnessMultiplier);
        } else if (elegooCommand == 0x08) {
          currentMode = BLUETOOTH_MODE;
          setRainbowColor(120, currentBrightnessMultiplier);
        } else if (elegooCommand == 0x1C) {
          currentMode = BLUETOOTH_MODE;
          setRainbowColor(180, currentBrightnessMultiplier);
        } else if (elegooCommand == 0x5A) {
          currentMode = BLUETOOTH_MODE;
          setRainbowColor(240, currentBrightnessMultiplier);
        } else if (elegooCommand == 0x16) {
          currentMode = BLUETOOTH_MODE;
          setRainbowColor(275, currentBrightnessMultiplier);
        } else if (elegooCommand == 0x52) {
          currentMode = AUTO_RAINBOW;
          sendSamsungCommand(SAMSUNG_VOLDN_CMD);
        }
      }
    }
    IrReceiver.resume();
  }

  if (currentTime - lastAnalogReadTime >= analogSampleInterval) {
    lastAnalogReadTime = currentTime;
    int potValue = analogRead(potPin);
    currentBrightnessMultiplier = potValue / 4095.0;

    if (currentMode == MANUAL_JOYSTICK) {
      int xRaw = analogRead(joyXPin);
      int yRaw = analogRead(joyYPin);
      int dx = xRaw - 2048;
      int dy = yRaw - 2048;

      if (abs(dx) > 600 || abs(dy) > 600) {
        float angleRad = atan2(dx, -dy);
        float angleDeg = angleRad * 180.0 / PI;
        if (angleDeg < 0) {
          angleDeg += 360.0;
        }
        lastManualHue = (int)angleDeg;

        if (lastManualHue >= 357 || lastManualHue <= 3) {
          lastManualHue = 0;
        } else if (lastManualHue >= 117 && lastManualHue <= 123) {
          lastManualHue = 120;
        } else if (lastManualHue >= 237 && lastManualHue <= 243) {
          lastManualHue = 240;
        }
      }
      setRainbowColor(lastManualHue, currentBrightnessMultiplier);
    }
  }

  if (SerialBT.available()) {
    char incomingChar = SerialBT.read();
    if (incomingChar == 'R' || incomingChar == 'r') {
      currentMode = BLUETOOTH_MODE; setRainbowColor(0, currentBrightnessMultiplier);
    } else if (incomingChar == 'O' || incomingChar == 'o') {
      currentMode = BLUETOOTH_MODE; setRainbowColor(13, currentBrightnessMultiplier);
    } else if (incomingChar == 'Y' || incomingChar == 'y') {
      currentMode = BLUETOOTH_MODE; setRainbowColor(42, currentBrightnessMultiplier);
    } else if (incomingChar == 'G' || incomingChar == 'g') {
      currentMode = BLUETOOTH_MODE; setRainbowColor(120, currentBrightnessMultiplier);
    } else if (incomingChar == 'C' || incomingChar == 'c') {
      currentMode = BLUETOOTH_MODE; setRainbowColor(180, currentBrightnessMultiplier);
    } else if (incomingChar == 'B' || incomingChar == 'b') {
      currentMode = BLUETOOTH_MODE; setRainbowColor(240, currentBrightnessMultiplier);
    } else if (incomingChar == 'P' || incomingChar == 'p') {
      currentMode = BLUETOOTH_MODE; setRainbowColor(275, currentBrightnessMultiplier);
    } else if (incomingChar == 'M' || incomingChar == 'm') {
      currentMode = BLUETOOTH_MODE; setRainbowColor(300, currentBrightnessMultiplier);
    } else if (incomingChar == 'W' || incomingChar == 'w') {
      currentMode = (currentMode == PURE_WHITE) ? LAMP_OFF : PURE_WHITE;
    } else if (incomingChar == 'A' || incomingChar == 'a') {
      currentMode = AUTO_RAINBOW;
    }
  }

  int joyReading = digitalRead(joyButton);
  if (joyReading != lastJoyBtnState) {
    lastJoyDebounce = currentTime;
  }
  if ((currentTime - lastJoyDebounce) > debounceDelay && joyReading == LOW) {
    if (currentMode == AUTO_RAINBOW) {
      currentMode = MANUAL_JOYSTICK;
    } else if (currentMode == MANUAL_JOYSTICK) {
      currentMode = PURE_WHITE;
    } else if (currentMode == PURE_WHITE) {
      currentMode = LAMP_OFF;
    } else {
      currentMode = AUTO_RAINBOW;
    }
    delay(200);
  }
  lastJoyBtnState = joyReading;

  if (currentMode == AUTO_RAINBOW) {
    if (currentTime - lastRainbowUpdate >= rainbowDelay) {
      lastRainbowUpdate = currentTime;
      setRainbowColor(autoHue, currentBrightnessMultiplier);
      autoHue = (autoHue + 1) % 360;
    }
  } else if (currentMode == PURE_WHITE) {
    int safeWhite = constrain((int)(currentBrightnessMultiplier * 180), 0, 180);
    ledcWrite(redChannel, safeWhite);
    ledcWrite(greenChannel, safeWhite);
    ledcWrite(blueChannel, safeWhite);
  } else if (currentMode == LAMP_OFF) {
    ledcWrite(redChannel, 0);
    ledcWrite(greenChannel, 0);
    ledcWrite(blueChannel, 0);
  }
}

void setRainbowColor(int hue, float dimFactor) {
  float r, g, b;
  float h = hue / 60.0;
  int i = floor(h);
  float f = h - i;
  float q = 1.0 - f;

  switch (i) {
    case 0: r = 1.0; g = f; b = 0.0; break;
    case 1: r = q; g = 1.0; b = 0.0; break;
    case 2: r = 0.0; g = 1.0; b = f; break;
    case 3: r = 0.0; g = q; b = 1.0; break;
    case 4: r = f; g = 0.0; b = 1.0; break;
    default: r = 1.0; g = 0.0; b = q; break;
  }

  const float safetyScale = 0.75;
  int rVal = constrain((int)(r * 255 * dimFactor * safetyScale), 0, 255);
  int gVal = constrain((int)(g * 255 * dimFactor * safetyScale), 0, 255);
  int bVal = constrain((int)(b * 255 * dimFactor * safetyScale), 0, 255);
  ledcWrite(redChannel, rVal);
  ledcWrite(greenChannel, gVal);
  ledcWrite(blueChannel, bVal);
}
