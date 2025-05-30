#include <Arduino.h>
#include <Wire.h>
#include <ArduinoUniqueID.h>

#define I2C_SLAVE_ADDRESS 0x08

void onRequest()
{
  // Send the UniqueID bytes one by one
  for (size_t i = 0; i < UniqueIDsize; i++) {
    Wire.write(UniqueID[i]);
  }
}

void setup()
{
  Wire.begin(I2C_SLAVE_ADDRESS);       // Start I2C in slave mode
  Wire.onRequest(onRequest);           // Register request handler
}

void loop()
{
  // Nothing to do here, wait for I2C request
}