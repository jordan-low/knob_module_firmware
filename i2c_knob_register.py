import smbus2
import time

I2CBUS = 8
I2C_ADDR = 0x49

bus = smbus2.SMBus(I2CBUS)

REG_ANALOG_0 = 0x00
REG_ANALOG_1 = 0x01
REG_ANALOG_2 = 0x02
REG_ANALOG_3 = 0x03
REG_LED_STATE = 0x20
REG_SAMPLING_RATE = 0x30
REG_VERSION = 0xFE

def read_analog(reg):
    bus.write_byte(I2C_ADDR,reg)
    time.sleep(0.01)
    data = bus.read_i2c_block_data(I2C_ADDR,reg,2)
    value = data[0]<<8 |data[1]
    value = value/1024 *3.3
    return (value)

def set_led(value):
    bus.write_i2c_block_data(I2C_ADDR,REG_LED_STATE,[value])
    time.sleep(0.01)

def set_sampling_rate(value):
    bus.write_i2c_block_data(I2C_ADDR,REG_SAMPLING_RATE,[value])

def get_version():
    bus.write_byte(I2C_ADDR,REG_VERSION,4)
    time.sleep(0.01)
    data = bus.read_i2c_block_data(I2C_ADDR,REG_VERSION,4)
    hardware_id = str(data[0]) +"." + str(data[1])
    software_id = str(data[2])+"." + str(data[3])
    return hardware_id,software_id

try:
    set_sampling_rate(300)
    hardware_version , firmware_version = get_version()
    print(f"hardware version : {hardware_version}, firmware version: {firmware_version}")
    set_led(1);
    while True:
        a0 = read_analog(REG_ANALOG_0)
        a1 = read_analog(REG_ANALOG_1)
        a2 = read_analog(REG_ANALOG_2)
        a3 = read_analog(REG_ANALOG_3)
        print(f"analog value: {a0:.2f} {a1:.2f} {a2:.2f} {a3:.2f}")
            

except KeyboardInterrupt:        
    print("exiting")