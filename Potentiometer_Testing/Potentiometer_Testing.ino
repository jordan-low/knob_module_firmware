#include <Arduino.h>
#include <Wire.h>

#define I2C_Address 0x48
#define OUTPUT_PIN A1  //adc pin connected to potentiometer

volatile uint16_t pinValue = 0;

void setup() {
  Wire.beign(I2C_Address);
  Wire.onRequest(requestEvent);
  analogReference(VCC);
}

void loop() {
  pinValue = analogRead(OUTPIN_PIN);
  delay(2);
}

void requestEvent() {
  Wire.write(highByte(pinValue));
  Wire.write(lowByte(pinValue));
}