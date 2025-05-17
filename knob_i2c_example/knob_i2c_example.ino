#include <Wire.h>
#define I2C_ADDR 0x49
const int analogPins[] = {18, 19, 7, 6};
const int numPins = sizeof(analogPins)/sizeof(analogPins[0]);
int analogValues[numPins];

void requestEvent(){
for (int i=0; i <numPins; i++)
{
  Wire.write((analogValues[i]>>8) & 0xFF);
  Wire.write((analogValues[i])& 0xFF);
}
}
void setup() {
  // put your setup code here, to run once:
  for(int i=0; i<numPins; i++)
  {
    pinMode(analogPins[i],INPUT_PULLUP);
  }
   Wire.begin(I2C_ADDR);
   Wire.onRequest(requestEvent);
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i=0; i<numPins; i++){
    analogValues[i] = analogRead(analogPins[i]);
  }
  delay(100);
}
