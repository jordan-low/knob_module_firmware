#include <Arduino.h>
#include <Wire.h>
#include <ArduinoUniqueID.h>

#define I2C_ADDR 0x49

#define HARDWARE_VER_MAJOR 1
#define HARDWARE_VER_MINOR 0
#define SOFTWARE_VER_MAJOR 1
#define SOFTWARE_VER_MINOR 2

#define REG_ENCODER1   0x01
#define REG_ENCODER2   0x02
#define REG_ENCODER3   0x03
#define REG_ENCODER4   0x04
#define REG_ANALOG0    0x05
#define REG_ANALOG1    0x06
#define REG_ANALOG2    0x07
#define REG_ANALOG3    0x08
#define REG_ANALOG4    0X09
#define REG_VERSION    0xFE
#define REG_UNIQUE_ID  0x10

const int addrPins[] = {2, 1, 20}; 
const int numAddrPins = sizeof(addrPins) / sizeof(addrPins[0]);
const int analogPins[] = {18, 19, 7, 6};
const int numAnalogPins = sizeof(analogPins) / sizeof(analogPins[0]);

int analogValues[numAnalogPins];
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
      Wire.write((analogValues[0] >> 8) & 0xFF);
      Wire.write(analogValues[0] & 0xFF);
      break;
    case REG_ANALOG1:
      Wire.write((analogValues[1] >> 8) & 0xFF);
      Wire.write(analogValues[1] & 0xFF);
      break;
    case REG_ANALOG2:
      Wire.write((analogValues[2] >> 8) & 0xFF);
      Wire.write(analogValues[2] & 0xFF);
      break;
    case REG_ANALOG3:
      Wire.write((analogValues[3] >> 8) & 0xFF);
      Wire.write(analogValues[3] & 0xFF);
      break;
      
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
  for (int i = 2; i < numAnalogPins; i++) {
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

  if (digitalRead(addrPins[0]) == 1) I2C_addr += 1;
  if (digitalRead(addrPins[1]) == 1) I2C_addr += 4;
  if (digitalRead(addrPins[2]) == 1) I2C_addr += 8;
  I2C_addr += I2C_ADDR;

  Wire.begin(I2C_addr);
  Wire.onRequest(requestEvent);
  Wire.onReceive(receiveEvent);

}



// Then modify your loop() function like this:
void loop() {
  //float v_ref = get_voltage(analogRead(analogPins[3]));
  for (int i = 0; i < 4; i++) {
    analogValues[i] = analogRead(analogPins[i]);
  }

  // Keep the encoder update part
  for (int i = 0; i < 4; i++) {
    int a = digitalRead(encoders[i].pinA);
    int b = digitalRead(encoders[i].pinB);
    if (a == HIGH && encoders[i].lastStateA == LOW) { // rising edge only
      if (b == LOW)
        encoders[i].value++;
      else
        encoders[i].value--;
    }
    encoders[i].lastStateA = a;
    encoders[i].buttonPressed = (digitalRead(encoders[i].pinButton) == LOW);
  }

  
  delayMicroseconds(1800);
}
