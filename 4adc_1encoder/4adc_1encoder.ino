#include <Arduino.h>
#include <Wire.h>
#include <ArduinoUniqueID.h>

#define I2C_ADDR 0x47

#define HARDWARE_VER_MAJOR 1
#define HARDWARE_VER_MINOR 0
#define SOFTWARE_VER_MAJOR 1
#define SOFTWARE_VER_MINOR 2

// Register definitions
#define REG_ENCODER1   0x01
#define REG_ANALOG0    0x05
#define REG_ANALOG1    0x06
#define REG_ANALOG2    0x07
#define REG_ANALOG3    0x08
#define REG_VERSION    0xFE
#define REG_UNIQUE_ID  0x10
#define REG_SAMPLING   0x30
#define REG_LED        0x20

// Pin configuration
const int addrPins[] = {2, 1, 20};
const int numAddrPins = sizeof(addrPins) / sizeof(addrPins[0]);
const int analogPins[] = {18, 19, 7, 6}; 
const int numAnalogPins = sizeof(analogPins) / sizeof(analogPins[0]);

// Internal configuration
int analogValues[numAnalogPins];
volatile uint8_t I2C_addr = 0x00;
volatile uint8_t RegisterAddress = 0x01;
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

Encoder encoder = {13, 16, 12, HIGH, 0, false};

void requestEvent() {
  uint8_t ch = RegisterAddress - REG_ENCODER1;
  switch (RegisterAddress) { 
    case REG_ENCODER1: // Encoder 1
      {
        int16_t p = encoder.value;
        uint8_t btn = encoder.buttonPressed ? 1 : 0;

        Wire.write(p & 0xFF);        // LSB
        Wire.write((p >> 8) & 0xFF); // MSB
        Wire.write(btn);             // Button state
      }
      break;

    case REG_ANALOG0: // Analog[0]
      Wire.write((analogValues[0] >> 8) & 0xFF); // MSB
      Wire.write(analogValues[0] & 0xFF);        // LSB
      break;

    case REG_ANALOG1: // Analog[1]
      Wire.write((analogValues[1] >> 8) & 0xFF); // MSB
      Wire.write(analogValues[1] & 0xFF);        // LSB
      break;

    case REG_ANALOG2: // Analog[2]
      Wire.write((analogValues[2] >> 8) & 0xFF); // MSB
      Wire.write(analogValues[2] & 0xFF);        // LSB
      break;

    case REG_ANALOG3: // Analog[3]
      Wire.write((analogValues[3] >> 8) & 0xFF); // MSB
      Wire.write(analogValues[3] & 0xFF);        // LSB
      break;

    case REG_VERSION: // Version information
      Wire.write(HARDWARE_VER_MAJOR);
      Wire.write(HARDWARE_VER_MINOR);
      Wire.write(SOFTWARE_VER_MAJOR);
      Wire.write(SOFTWARE_VER_MINOR);
      break;

    case REG_UNIQUE_ID: // Unique ID
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
          case REG_SAMPLING: 
            if (data1 > 0 && data1 < 200) {
              sampling_Delay = data1;
            }
            break;
            
          case REG_LED: // Set LED state
            ledState = data1;
            break;
        }
      } 
      else if (BytesReceived == 3) {
        // Two byte for encoder
        uint8_t data2 = Wire.read();
        int16_t newPos = (int16_t)((data2 << 8) | data1);
        
        if (RegisterAddress == REG_ENCODER1) {
          encoder.value = newPos;
        }
      }
    }
  }
}

void setup() {
  // Initialize analog pins
  for (int i = 0; i < numAnalogPins; i++) {
    pinMode(analogPins[i], INPUT);
  }

  // Initialize address pins
  for (int i = 0; i < numAddrPins; i++) {
    pinMode(addrPins[i], INPUT_PULLUP);
  }

  // Initialize encoder pins
  pinMode(encoder.pinA, INPUT_PULLUP);
  pinMode(encoder.pinB, INPUT_PULLUP);
  pinMode(encoder.pinButton, INPUT_PULLUP);
  encoder.lastStateA = digitalRead(encoder.pinA);

  // Calculate I2C address
  if (digitalRead(addrPins[0]) == 1) I2C_addr += 1;
  if (digitalRead(addrPins[1]) == 1) I2C_addr += 4;
  if (digitalRead(addrPins[2]) == 1) I2C_addr += 8;
  I2C_addr += I2C_ADDR;

  // Initialize I2C
  Wire.begin(I2C_addr);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);
}

void loop() {
  // Read analog values
  for (int i = 0; i < numAnalogPins; i++) {
    analogValues[i] = analogRead(analogPins[i]);
  }

  // Read encoder
  int a = digitalRead(encoder.pinA);
  int b = digitalRead(encoder.pinB);
  
  // Check for encoder rotation
  if (a != encoder.lastStateA) {
    if (a == b) {
      encoder.value++;
    } else {
      encoder.value--;
    }
    encoder.lastStateA = a;
  }
  
  // Read encoder button
  encoder.buttonPressed = (digitalRead(encoder.pinButton) == LOW);

  // Use configurable delay
  delay(sampling_Delay);
}