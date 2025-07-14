
from smbus2 import SMBus
import time

# I2C Configuration
I2C_ADDR = 0x48
I2C_BUS = smbus2.SMBus(I2CBUS)

def read_potentiometer():
    with SMBus(I2C_BUS) as bus:
        data = bus.read_i2c_block_data(I2C_ADDR, 0, 2)
        adc_value = (data[0] << 8) | data[1]
        return adc_value

while True:
    try:
        adc = read_potentiometer()
        voltage = (adc / 1023.0) * 3.3  # Adjust Vref as needed
        print(f"ADC: {adc} | Voltage: {voltage:.2f} V")
        time.sleep(0.5)
    except Exception as e:
        print("Error:", e)
        time.sleep(2)



