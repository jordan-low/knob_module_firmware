#include <Wire.h>
#define I2C_ADDR 0x49
const int digitalPins[] = {2,1,20};
const int addrPins = sizeof(digitalPins)/sizeof(digitalPins[0]);
const int analogPins[] = {18, 19, 7, 6};
const int numPins = sizeof(analogPins)/sizeof(analogPins[0]);
int analogValues[numPins];
int addr = 0;

int channelSelect = 0;
void requestEvent(){
/*for (int i=0; i <numPins; i++)
{
  Wire.write((analogValues[i] >> 8) & 0xFF);
  Wire.write((analogValues[i]) & 0xFF);
}
*/
  Wire.write((analogValues[channelSelect] >> 8) & 0xFF);
  Wire.write((analogValues[channelSelect]) & 0xFF);
}
void receiveEvent(int ByteReceived){
  if(ByteReceived>=1)
  {  
    channelSelect = Wire.read();
  }
}

void setup() {
  // put your setup code here, to run once:
  for(int i=0; i<numPins; i++)
  {
    pinMode(analogPins[i],INPUT_PULLUP);
  }
  for(int i =0; i<addrPins; i++)
  {
    pinMode(digitalPins[i],INPUT_PULLUP);
  }
  if(digitalRead(digitalPins[0])==1)
  addr += 1;
  if(digitalRead(digitalPins[1])==1)
  addr += 4;
  if(digitalRead(digitalPins[2])==1)
  addr += 8;
  
  addr+= I2C_ADDR;
   Wire.begin(addr);
   Wire.onRequest(requestEvent);
   Wire.onReceive(receiveEvent);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i=0; i<numPins; i++){
    analogValues[i] = analogRead(analogPins[i]);
  }
  delay(100);
}