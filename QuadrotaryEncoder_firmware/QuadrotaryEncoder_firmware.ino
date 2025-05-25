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
volatile uint8_t RegisterAddress = 0x00;

struct Encoder {
  int pinA;
  int pinB;
  int pinButton;
  int lastStateA;
  int value;
  bool buttonPressed;
};

Encoder encoder[4] = {
  {3,4,10,HIGH,0,false},
  {5,6,11,HIGH,0,false},
  {7,8,12,HIGH,0,false},
  {9,10,13,HIGH,0,false},
};
void requestEvent(){
  
}

void receiveEvent(int ByteReceived){
  
}

void setup() {
  // put your setup code here, to run once:
for( int i =0; i<4; i ++){
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
  I2C_addr+= I2C_ADDR;
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
