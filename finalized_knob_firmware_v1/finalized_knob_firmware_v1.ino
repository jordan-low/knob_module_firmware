#include <Arduino.h>
#include <Wire.h>
#include <ArduinoUniqueID.h>

#define I2C_ADDR 0x08

#define HARDWARE_VER_MAJOR 1
#define HARDWARE_VER_MINOR 0
#define SOFTWARE_VER_MAJOR 1
#define SOFTWARE_VER_MINOR 2

#define REG_ENCODER1   0x01
#define REG_ENCODER2   0x02
#define REG_ENCODER3   0x03
#define REG_ENCODER4   0x04
#define REG_ANALOG0  0x05
#define REG_ANALOG1  0x06
#define REG_ANALOG2  0x07
#define REG_ANALOG3  0x08
#define REG_VERSION   0xFE
#define REG_UNIQUE_ID 0x10

//pin configuration
const int addrPins[] = {2, 1, 20};
const int numAddrPins = sizeof(addrPins) / sizeof(addrPins[0]);
const int analogPins[] = {18, 19, 7, 6}; 
const int numAnalogPins = sizeof(analogPins) / sizeof(analogPins[0]);

//internal configuration

int analogValues[numAnalogPins];
volatile uint8_t I2C_addr = 0x00;
volatile uint8_t RegisterAddress = REG_ENCODER1;
uint8_t sampling_Delay =100;
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


void requestEvent() {
  uint8_t ch = RegisterAddress - REG_ENCODER1;
  switch (RegisterAddress) { 
    case 0x01: //encoder 1
    if (ch < 4) {
      int16_t p = encoders[ch].value;
      uint8_t btn = encoders[ch].buttonPressed ? 1 : 0;

      Wire.write(p & 0xFF);        // LSB
      Wire.write((p >> 8) & 0xFF); // MSB
      Wire.write(btn);   
    }
    break;

    case 0x02: //encoder 2
    if (ch < 4) {
      int16_t p = encoders[ch].value;
      uint8_t btn = encoders[ch].buttonPressed ? 1 : 0;

      Wire.write(p & 0xFF);        // LSB
      Wire.write((p >> 8) & 0xFF); // MSB
      Wire.write(btn);   
    }
    break;

    case 0x03: //encoder 3
    if (ch < 4) {
      int16_t p = encoders[ch].value;
      uint8_t btn = encoders[ch].buttonPressed ? 1 : 0;

      Wire.write(p & 0xFF);        // LSB
      Wire.write((p >> 8) & 0xFF); // MSB
      Wire.write(btn);   
    }
    break;

    case 0x04: //encoder 4
    if (ch < 4) {
      int16_t p = encoders[ch].value;
      uint8_t btn = encoders[ch].buttonPressed ? 1 : 0;

      Wire.write(p & 0xFF);        // LSB
      Wire.write((p >> 8) & 0xFF); // MSB
      Wire.write(btn);   
    }
    break;

    case 0x05: //read Analog[0]
    Wire.write(analogValues[0]>>8 & 0xFF);
    Wire.write(analogValues[0] & 0xFF);
    break;

    case 0x06: //read Analog[1]
    Wire.write(analogValues[0]>>8 & 0xFF);
    Wire.write(analogValues[0] & 0xFF);
    break;

    case 0x07: //read Analog[2]
    Wire.write(analogValues[0]>>8 & 0xFF);
    Wire.write(analogValues[0] & 0xFF);
    break;

    case 0x08: //read Analog[3]
    Wire.write(analogValues[0]>>8 & 0xFF);
    Wire.write(analogValues[0] & 0xFF);
    break;

    case 0xFE: //version
    Wire.write(HARDWARE_VER_MAJOR);
    Wire.write(HARDWARE_VER_MINOR);
    Wire.write(SOFTWARE_VER_MAJOR);
    Wire.write(SOFTWARE_VER_MINOR);
    break;

    case 0x10: //Unique ID
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
        // One-byte write
        switch (RegisterAddress) {
          case 0x30:
            if (data1 > 0 && data1 < 200)
              sampling_Delay = data1;
            break;
          case 0x20:
            ledState = data1;
            break;
        }
      } else if (BytesReceived == 3) {
        // Two-byte write for encoder value
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
    pinMode(addrPins[i], INPUT_PULLUP);
  }

  for (int i = 0; i < 4; i++) {
    pinMode(encoders[i].pinA, INPUT_PULLUP);
    pinMode(encoders[i].pinB, INPUT_PULLUP);
    pinMode(encoders[i].pinButton, INPUT_PULLUP);
    encoders[i].lastStateA = digitalRead(encoders[i].pinA);
  }

  if (digitalRead(addrPins[0]) == 1) I2C_addr += 1;
  if (digitalRead(addrPins[1]) == 1) I2C_addr += 4;
  if (digitalRead(addrPins[2]) == 1) I2C_addr += 8;
  I2C_addr += I2C_ADDR;

  Wire.begin(I2C_addr);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
}

void loop() {
  for (int i = 0; i < numAnalogPins; i++) {
    analogValues[i] = analogRead(analogPins[i]);
  }

  for (int i = 0; i < 4; i++) {
    int a = digitalRead(encoders[i].pinA);
    int b = digitalRead(encoders[i].pinB);
    if (a != encoders[i].lastStateA) {
      if (a == b)
        encoders[i].value++;
      else
        encoders[i].value--;
      encoders[i].lastStateA = a;
    }
    encoders[i].buttonPressed = (digitalRead(encoders[i].pinButton) == LOW);
  }

  delay(2);
}
