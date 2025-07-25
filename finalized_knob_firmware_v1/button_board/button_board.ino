#include <Arduino.h>
#include <Wire.h>
#include <ArduinoUniqueID.h>
#include <Adafruit_NeoPixel.h>


#define I2C_ADDR 0x49

#define HARDWARE_VER_MAJOR 1
#define HARDWARE_VER_MINOR 0
#define SOFTWARE_VER_MAJOR 1
#define SOFTWARE_VER_MINOR 2

#define REG_ENCODER2   0x02
#define REG_ANALOG4    0X09
#define REG_VERSION    0xFE
#define REG_UNIQUE_ID  0x10


#define LED_PIN 18
#define NUM_LEDS 1
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

enum Color { RED, GREEN, BLUE };
Color currentColor = RED;

void changeColor() {
  if (currentColor == RED) {
    strip.setPixelColor(0, strip.Color(0, 255, 0)); // Green
    currentColor = GREEN;
  } else if (currentColor == GREEN) {
    strip.setPixelColor(0, strip.Color(0, 0, 255)); // Blue
    currentColor = BLUE;
  } else {
    strip.setPixelColor(0, strip.Color(255, 0, 0)); // Red
    currentColor = RED;
  }
  strip.show();
}


const int addrPins[] = {2, 1, 20};
const int numAddrPins = sizeof(addrPins) / sizeof(addrPins[0]);

const int analogPins[] = {18, 19, 7, 6};
const int numAnalogPins = sizeof(analogPins) / sizeof(analogPins[0]);

int analogValues[numAnalogPins];
int prevAnalogValues[numAnalogPins] = {0};

volatile uint8_t I2C_addr = 0x00;
volatile uint8_t RegisterAddress = REG_ENCODER1;
uint8_t sampling_Delay = 100;
uint8_t ledState = 0;

struct Encoder {
  int pinA;
  int pinB;
  int pinButton;
  int lastStateA;
  int value;
  bool buttonPressed;
};

Encoder encoders[4] = {
  {13, 16, 12, HIGH, 0, false},
  {15, 19, 14, HIGH, 0, false},
  {7, 6, 17, HIGH, 0, false},
  {4, 5, 9, HIGH, 0, false},
};

int prevEncoderValues[4] = {0};
bool prevButtonStates[4] = {false};


void requestEvent() {
  uint8_t ch = RegisterAddress - REG_ENCODER1;
  switch (RegisterAddress) {
    case REG_ENCODER1:
    case REG_ENCODER2:
    case REG_ENCODER3:
    case REG_ENCODER4:
      if (ch < 4) {
        int16_t p = encoders[ch].value;
        uint8_t btn = encoders[ch].buttonPressed ? 1 : 0;
        Wire.write(p & 0xFF);
        Wire.write((p >> 8) & 0xFF);
        Wire.write(btn);
      }
      break;
    case REG_ANALOG0:
    case REG_ANALOG1:
    case REG_ANALOG2:
    case REG_ANALOG3: {
      int index = RegisterAddress - REG_ANALOG0;
      Wire.write((analogValues[index] >> 8) & 0xFF);
      Wire.write(analogValues[index] & 0xFF);
      break;
    }
    case REG_VERSION:
      Wire.write(HARDWARE_VER_MAJOR);
      Wire.write(HARDWARE_VER_MINOR);
      Wire.write(SOFTWARE_VER_MAJOR);
      Wire.write(SOFTWARE_VER_MINOR);
      break;
    case REG_UNIQUE_ID:
      for (size_t i = 0; i < UniqueIDsize; i++) {
        Wire.write(UniqueID[i]);
      }
      break;
  }
}

void receiveEvent(int BytesReceived) {
  if (BytesReceived >= 1) {
    RegisterAddress = Wire.read();
    if (BytesReceived >= 2) {
      uint8_t data1 = Wire.read();
      if (BytesReceived == 2) {
        switch (RegisterAddress) {
          case 0x30:
            if (data1 > 0 && data1 < 200) sampling_Delay = data1;
            break;
          case 0x20:
            ledState = data1;
            break;
        }
      } else if (BytesReceived == 3) {
        uint8_t data2 = Wire.read();
        int16_t newPos = (int16_t)((data2 << 8) | data1);
        uint8_t ch = RegisterAddress - REG_ENCODER1;
        if (ch < 4) {
          encoders[ch].value = newPos;
        }
      }
    }
  }
}


void setup() {
  for (int i = 0; i < numAnalogPins; i++) {
    pinMode(analogPins[i], INPUT);
  }

  for (int i = 0; i < numAddrPins; i++) {
    pinMode(addrPins[i], INPUT);
  }

  for (int i = 0; i < 4; i++) {
    pinMode(encoders[i].pinA, INPUT_PULLUP);
    pinMode(encoders[i].pinB, INPUT_PULLUP);
    pinMode(encoders[i].pinButton, INPUT_PULLUP);
    encoders[i].lastStateA = digitalRead(encoders[i].pinA);
  }

  // Assign I2C address
  if (digitalRead(addrPins[0]) == 1) I2C_addr += 1;
  if (digitalRead(addrPins[1]) == 1) I2C_addr += 4;
  if (digitalRead(addrPins[2]) == 1) I2C_addr += 8;
  I2C_addr += I2C_ADDR;

  Wire.begin(I2C_addr);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

  // LED strip init
  strip.begin();
  strip.show();
  strip.setPixelColor(0, strip.Color(255, 0, 0)); // Start with RED
  strip.show();
}


void loop() {
  bool changed = false;

  // Read and compare analog values
  for (int i = 0; i < 4; i++) {
    int newValue = analogRead(analogPins[i]);
    analogValues[i] = newValue;
    if (abs(newValue - prevAnalogValues[i]) > 5) {
      prevAnalogValues[i] = newValue;
      changed = true;
    }
  }

  // Handle encoders
  for (int i = 0; i < 4; i++) {
    int a = digitalRead(encoders[i].pinA);
    int b = digitalRead(encoders[i].pinB);

    if (a == HIGH && encoders[i].lastStateA == LOW) {
      if (b == LOW)
        encoders[i].value++;
      else
        encoders[i].value--;
    }
    encoders[i].lastStateA = a;

    // Detect encoder value change
    if (encoders[i].value != prevEncoderValues[i]) {
      prevEncoderValues[i] = encoders[i].value;
      changed = true;
    }

    // Detect button press change
    bool pressed = (digitalRead(encoders[i].pinButton) == LOW);
    encoders[i].buttonPressed = pressed;
    if (pressed != prevButtonStates[i]) {
      prevButtonStates[i] = pressed;
      changed = true;
    }
  }

  // If anything changed change LED color
  if (changed) {
    changeColor();
  }

  delayMicroseconds(1800);
}
