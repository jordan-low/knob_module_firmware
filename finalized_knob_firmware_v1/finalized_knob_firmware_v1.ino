#include <Wire.h>
#define I2C_ADDR 0x49

#define HARDWARE_VER_MAJOR 1
#define HARDWARE_VER_MINOR 0
#define SOFTWARE_VER_MAJOR 1
#define SOFTWARE_VER_MINOR 2

//pin configuration
const int digitalPins[] = {2,1,20};
const int addrPins = sizeof(digitalPins)/sizeof(digitalPins[0]);
const int analogPins[] = {18, 19, 7, 6};
const int numPins = sizeof(analogPins)/sizeof(analogPins[0]);

//internal configuration

int analogValues[numPins];
volatile uint8_t I2C_addr = 0x00;
volatile uint8_t RegisterAddress = 0x00;
uint8_t sampling_Delay =100;
uint8_t ledState = 0;

void requestEvent(){
  switch (RegisterAddress){
    case 0x00: //read Analog[0]
    Wire.write(analogValues[0]>>8 & 0xFF);
    Wire.write(analogValues[0] & 0xFF);
    break;
    case 0x01:
    Wire.write(analogValues[1]>>8 & 0xFF);
    Wire.write(analogValues[1] & 0xFF);
    break;
    case 0x02:
    Wire.write(analogValues[2]>>8 & 0xFF);
    Wire.write(analogValues[2] & 0xFF);
    break;
    case 0x03:
    Wire.write(analogValues[3]>>8 & 0xFF);
    Wire.write(analogValues[3] & 0xFF);
    break;
    case 0xFE:
    Wire.write(HARDWARE_VER_MAJOR);
    Wire.write(HARDWARE_VER_MINOR);
    Wire.write(SOFTWARE_VER_MAJOR);
    Wire.write(SOFTWARE_VER_MINOR);
    break;
  }
}

void receiveEvent(int BytesReceived){
  if(BytesReceived>=1)
  {
    RegisterAddress = Wire.read();
    if(BytesReceived>=2){
      uint8_t data = Wire.read();
      switch (RegisterAddress){
        case 0x30:
        if((data>0)&&(data<1000))
        sampling_Delay = data;
        break;

        case 0x20:
        ledState = data;
      }
    }
  }
}
void setup() {
   // put your setup code here, to run once:
  for(int i=0; i<numPins; i++)
  {
    pinMode(analogPins[i],INPUT);
  }
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
  for (int i=0; i<numPins; i++){
    analogValues[i] = analogRead(analogPins[i]);
  }
  delay(sampling_Delay);
}
