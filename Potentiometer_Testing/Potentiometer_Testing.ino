#include <Wire.h>

#define I2C_ADDRESS 0x48 // 7-bit address

#define POT0_PIN PA3 // ADC IN 0
#define POT1_PIN PA4 // ADC IN 1
#define POT2_PIN PA5 // ADC IN 2

volatile uint16_t potValues[3] = {0};

void setup() {
Wire.begin(I2C_ADDRESS); // Initialize I2C slave
Wire.onRequest(requestEvent); // Register data request callback
analogReference(3.3); // Use VCC as ADC reference (3.3V)
}

void loop() {
// Read ADC values
potValues[0] = analogRead(POT0_PIN);
potValues[1] = analogRead(POT1_PIN);
potValues[2] = analogRead(POT2_PIN);
delay(5); // Optional: slight delay for stability
}

void requestEvent() {
// Send 6 bytes (3x uint16_t values)
for (int i = 0; i < 3; i++) {
Wire.write(highByte(potValues[i]));
Wire.write(lowByte(potValues[i]));
}
}