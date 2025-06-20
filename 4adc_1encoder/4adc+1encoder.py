import smbus2
import time

# I2C Configuration
I2CBUS = 7
I2C_ADDR = 0x08
unique_id_size = 10
SAMPLING_RATE = 100
bus = smbus2.SMBus(I2CBUS)

# Register definitions
REG_ENCODER1 = 0x01
REG_ANALOG0 = 0x05
REG_ANALOG1 = 0x06
REG_ANALOG2 = 0x07
REG_ANALOG3 = 0x08
REG_VERSION = 0xFE
REG_UNIQUE_ID = 0x10
REG_SAMPLING = 0x30
REG_LED = 0x20

def set_encoder_position(value):
    lsb = value & 0xFF
    msb = (value >> 8) & 0xFF
    bus.write_i2c_block_data(I2C_ADDR, REG_ENCODER1, [lsb, msb])
    time.sleep(0.01)

def read_encoder():
    # send register address
    bus.write_byte(I2C_ADDR, REG_ENCODER1)
    time.sleep(0.01)
    
    # Read 3 bytes: LSB, MSB, button
    data = bus.read_i2c_block_data(I2C_ADDR, REG_ENCODER1, 3)
    pos = data[0] | (data[1] << 8)
    
    # Convert to signed int16 if needed
    if pos & 0x8000:
        pos -= 0x10000
    
    button = bool(data[2])
    return pos, button

def read_analog(channel):
    if channel < 0 or channel > 3:
        raise ValueError("Channel must be 0-3")
    
    reg = REG_ANALOG0 + channel
    bus.write_byte(I2C_ADDR, reg)
    time.sleep(0.01)
    
    data = bus.read_i2c_block_data(I2C_ADDR, reg, 2)
    value = (data[0] << 8) | data[1]
    
    voltage = (value / 1024.0) * 3.3
    return voltage

def read_analogs():
    values = []
    for i in range(4):
        values.append(read_analog(i))
    return values

def set_led(value):
    bus.write_i2c_block_data(I2C_ADDR, REG_LED, [value])
    time.sleep(0.01)

def set_sampling_rate(value):
    if value > 0 and value < 200:
        bus.write_i2c_block_data(I2C_ADDR, REG_SAMPLING, [value])
        time.sleep(0.01)

def get_version():
    bus.write_byte(I2C_ADDR, REG_VERSION)
    time.sleep(0.01)
    
    data = bus.read_i2c_block_data(I2C_ADDR, REG_VERSION, 4)
    hardware_version = f"{data[0]}.{data[1]}"
    software_version = f"{data[2]}.{data[3]}"
    return hardware_version, software_version

def get_unique_id():
    bus.write_byte(I2C_ADDR, REG_UNIQUE_ID)
    time.sleep(0.01)
    
    id_data = bus.read_i2c_block_data(I2C_ADDR, REG_UNIQUE_ID, unique_id_size)
    return " ".join(f"{b:02X}" for b in id_data)

def main():
    try:
        set_sampling_rate(SAMPLING_RATE)
        unique_id = get_unique_id()
        print(f"Device Unique ID: {unique_id}")
        

        prev_button = False
        
        while True:
            encoder_pos, button_pressed = read_encoder()
            
            if prev_button and not button_pressed:
                set_encoder_position(0)
            
            prev_button = button_pressed
            
            analog_values = read_all_analogs()
            
            print(f"Encoder: {encoder_pos:4d} | Button: {'ON ' if button_pressed else 'OFF'} | " +
                  f"ADC: {analog_values[0]:.2f}V {analog_values[1]:.2f}V {analog_values[2]:.2f}V {analog_values[3]:.2f}V")
            
            time.sleep(0.01)  
    except KeyboardInterrupt:
        print("\nExiting...")