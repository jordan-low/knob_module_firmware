# knob_module_firmware

- **Internal Knob Board** (`0x51`): 1 quad rotary encoder with push button and 1 analog knob.
- **Operational Knob Board** (`0x49`): 3 analog knobs (RS1, RS2, RS3) with discrete calibration mapping.

The code script communicates with the boards via I2C Bus 7 to read encoder data, analog voltages, and identify devices via version and unique ID registers.

## Setup

### ATtiny817 (finalized_knob_firmware_v1.ino)
- Board: ATtiny3227/3217/1627/1617/1607/827/817/807/427/417
- Chip: ATtiny817
- Clock: 10 MHz internal
- Programmer: "SerialUPDI - SLOW: 57600 baud"

### jetson (Final_Calibration.py)
- host: 192.168.55.1
- Jetson device with I2C Bus 7 enabled
- Internal Knob Board at address `0x51`
- Operational Knob Board at address `0x49`
- Shared 3.3V and GND

## Check for I2C detection using:
sudo i2cdetect -y -r 7

## Run the script using:
python3 Final_Calibration.py

---

## Test Cases

### Internal Knob Board

#### Encoder

**Objective:**  
Verify that the firmware on the ATtiny817 correctly detects rotary encoder events (direction and step count), maintains the correct count, and exposes it reliably via I2C.

**Test Steps:**
- Rotate encoder clockwise by 5 detents  
  → Read encoder count over I2C and verify value is +5  
- Rotate encoder counter-clockwise by 3 detents  
  → Read encoder count and verify it is +2  
- Rotate quickly clockwise by 10 detents  
  → Verify no steps are lost

**Pass/Fail Criteria:**
- Firmware must accurately reflect encoder direction and step count via I2C
- Fast rotations should not result in missed steps

#### Analog Knob (7-position)

**Test Step:**
- Record ADC value for each known position
- Start at position 0 → rotate to position 3 → record voltage
- Repeat 5 times to verify consistency
- Repeat for position 5

---

### Operational Knob Board (19 postions)

**Objective:**  
Verify that changes in potentiometer position are accurately read

**Test Steps:**
1. **Setup:**
   - Read `Vin` and `Vref`
   - Compute: `position = Vin / (Vref / 19)`
   - Record values for each detent
   - Establish thresholds to assign discrete position numbers

2. **Consistency:**
   - Choose 3-4 random positions A to test
   - Start at position 0 and rotate to A
   - Record and check voltage consistency
   - Repeat 5 times

3. **Reverse Check:**
   - Rotate to random position
   - Compute position from voltage
   - Check if calculated position matches expected

**Pass/Fail Criteria:**
- ADC values received over I2C should reflect actual potentiometer position
- Acceptable deviation: +-5%
- Failure indicated by inconsistent or stuck values

## Sample Output

hardware version : 1.0, firmware version: 1.2

Encoder position: -3, ADC: 1.79 Received Unique ID: AA BB CC DD EE FF 00 11 22 33

RS1=> 12 RS2=> 7 RS3=> 3 (target) Received Unique ID: 44 55 66 77 88 99 AA BB CC DD