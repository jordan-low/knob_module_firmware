#include <Wire.h>
#define I2C_ADDR 0x49
#define HARDWARE_VER_MAJOR 1
#define HARDWARE_VER_MINOR 0
#define SOFTWARE_VER_MAJOR 1
#define SOFTWARE_VER_MINOR 2

//ADDR pin config
const int digitalPins[] = {2,1,20};
const int addrPins = sizeof(digitalPins)/sizeof(digitalPins[0]);

volatile uint8_t I2C_addr = 0x00;

#define REG_ENCODER1 0x01
#define REG_ENCODER2 0x02
#define REG_ENCODER3 0x03
#define REG_ENCODER4 0x04
#define REG_VERSION 0xFE

volatile uint8_t RegisterAddress = REG_VERSION;

struct Encoder {
  int pinA;
  int pinB;
  int pinButton;
  int lastStateA;
  int value;
  bool buttonPressed;
};

Encoder encoder[4] = {
  {13,16,12,HIGH,0,false},
  {15,19,14,HIGH,0,false},
  {7,6,17,HIGH,0,false},
  {4,5,9,HIGH,0,false},
};

int16_t pos=0;
uint8_t button=0;

volatile uint8_t lastReceived = 0;


void requestEvent() {
  uint8_t ch = RegisterAddress - REG_ENCODER1;
  switch (RegisterAddress) {
    case REG_ENCODER1:
    if (ch < 4) {
      int16_t p = encoder[ch].value;
      uint8_t btn = encoder[ch].buttonPressed ? 1 : 0;

      Wire.write(p & 0xFF);        // LSB
      Wire.write((p >> 8) & 0xFF); // MSB
      Wire.write(btn);   
    }
    break;
    case REG_ENCODER2:
    if (ch < 4) {
      int16_t p = encoder[ch].value;
      uint8_t btn = encoder[ch].buttonPressed ? 1 : 0;

      Wire.write(p & 0xFF);        // LSB
      Wire.write((p >> 8) & 0xFF); // MSB
      Wire.write(btn);   
    }
    break;
    case REG_ENCODER3:
    if (ch < 4) {
      int16_t p = encoder[ch].value;
      uint8_t btn = encoder[ch].buttonPressed ? 1 : 0;

      Wire.write(p & 0xFF);        // LSB
      Wire.write((p >> 8) & 0xFF); // MSB
      Wire.write(btn);   
    }
    break;
    case REG_ENCODER4:
    if (ch < 4) {
      int16_t p = encoder[ch].value;
      uint8_t btn = encoder[ch].buttonPressed ? 1 : 0;

      Wire.write(p & 0xFF);        // LSB
      Wire.write((p >> 8) & 0xFF); // MSB
      Wire.write(btn);   
    }
    break;

    case REG_VERSION:
      Wire.write(HARDWARE_VER_MAJOR);
      Wire.write(HARDWARE_VER_MINOR);
      Wire.write(SOFTWARE_VER_MAJOR);
      Wire.write(SOFTWARE_VER_MINOR);
      break;
  }
}


void receiveEvent(int ByteReceived) {
  
  if (ByteReceived >= 1) {
   RegisterAddress = Wire.read(); 
    if (ByteReceived >= 2){
      uint8_t low = Wire.read();
      uint8_t high = Wire.read();
      int16_t newPos = (int16_t)((high << 8) | low);
      uint8_t ch = RegisterAddress - 0x01;  // Convert register to index (0–3)
      if (ch < 4) {
        encoder[ch].value = newPos; // Set encoder value manually from master      // Register address for encoder channel
      }
    }
  }
}


void setup() {
  // put your setup code here, to run once:
  for( int i = 0; i<4; i ++){
    pinMode(encoder[i].pinA,INPUT_PULLUP);
    pinMode(encoder[i].pinB,INPUT_PULLUP);
    pinMode(encoder[i].pinButton,INPUT_PULLUP);
    encoder[i].lastStateA = digitalRead(encoder[i].pinA);
  }
  
  //ADDR pin setup
 for(int i =0; i<addrPins; i++)
  {
    pinMode(digitalPins[i],INPUT_PULLUP);
  }
  if(digitalRead(digitalPins[0])==1)
  I2C_addr += 1;
  if(digitalRead(digitalPins[1])==1)
  I2C_addr += 4;
  if(digitalRead(digitalPins[2])==1)
  I2C_addr += 8;
  I2C_addr += I2C_ADDR;
   Wire.begin(I2C_addr);
   Wire.onRequest(requestEvent);
   Wire.onReceive(receiveEvent);
}


void loop() {
  // put your main code here, to run repeatedly:
  for (int i =0; i <4 ; i++){
    int a = digitalRead(encoder[i].pinA);
    int b = digitalRead(encoder[i].pinB);
    if (a !=encoder[i].lastStateA){
      if(a==b) 
      encoder[i].value++;
      else
      encoder[i].value--;
      encoder[i].lastStateA = a;
    }
    encoder[i].buttonPressed = (digitalRead(encoder[i].pinButton)==LOW);
  }
  delay(2);
}