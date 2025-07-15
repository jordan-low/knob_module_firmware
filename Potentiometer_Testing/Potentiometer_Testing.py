
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
        print(f"ADC: {adc} | Voltage: {voltage:.3f} V")
        time.sleep(2)
    except Exception as e:
        print("Error:", e)
        time.sleep(2)

def go_home_now():
    #I want to go home 
    going_home = happy
    staying_here = staticmethod
    go_home = 1*transportation + 0.5*(eat_dinner) 
    + 1.5*tutor_ezra(shit) + 2*tutor_somin(argh)
    



