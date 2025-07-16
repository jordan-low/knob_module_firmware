import smbus2
import time

I2CBUS = 7
I2C_ADDR = 0x49
unique_id_size = 10
SAMPLING_RATE = 100
bus = smbus2.SMBus(I2CBUS)

REG_ENCODER_1 = 0x01
REG_ENCODER_2 = 0x02
REG_ENCODER_3 = 0x03
REG_ENCODER_4 = 0x04
REG_ANALOG_0 = 0x05
REG_ANALOG_1 = 0x06
REG_ANALOG_2 = 0x07
REG_ANALOG_3 = 0x08
REG_ANALOG0_POS = 0x09
REG_ANALOG1_POS = 0x0A
REG_ANALOG2_POS = 0x0B
REG_ENCODER_POS = 0x20
REG_ENCODER_BTN = 0x30
REG_VERSION = 0xFE
REG_UNIQUE_ID = 0x10



def set_position(index, value):
    lsb = value & 0xFF
    msb = (value >> 8) & 0xFF
    bus.write_i2c_block_data(I2C_ADDR, 0x00 + index, [lsb, msb])
    time.sleep(0.01)

def read_encoder(reg):
    # First send a "read request" (0x00) followed by the register address
    bus.write_i2c_block_data(I2C_ADDR, 0x00, [reg])
    time.sleep(0.01)  # Short delay

    # Read 3 bytes: LSB, MSB, button
    data = bus.read_i2c_block_data(I2C_ADDR, reg, 3)
    pos = data[0] | (data[1] << 8)
    if pos & 0x8000:  # Convert to signed int16 if needed
        pos -= 0x10000
    button = data[2]
    return pos, button


def read_analog(reg):
    bus.write_byte(I2C_ADDR,reg)
    time.sleep(0.01)
    data = bus.read_i2c_block_data(I2C_ADDR,reg,2)
    value = data[0]<<8 |data[1]
    value = value/1024 *3.3
    return (value)



def read_position(reg):
    bus.write_byte(I2C_ADDR, reg)
    time.sleep(0.01)
    data = bus.read_i2c_block_data(I2C_ADDR, reg, 2)
    value = data[0] | (data[1] << 8)
    if value & 0x8000:  # if signed
        value -= 0x10000
    return value

def set_led(value):
    bus.write_i2c_block_data(I2C_ADDR,REG_LED_STATE,[value])
    time.sleep(0.01)

def set_sampling_rate(value):
    bus.write_i2c_block_data(I2C_ADDR,SAMPLING_RATE,[value])

def get_version():
    bus.write_byte(I2C_ADDR,REG_VERSION,4)
    time.sleep(0.01)
    data = bus.read_i2c_block_data(I2C_ADDR,REG_VERSION,4)
    hardware_id = str(data[0]) +"." + str(data[1])
    software_id = str(data[2])+"." + str(data[3])
    return hardware_id,software_id


try:
    set_sampling_rate(SAMPLING_RATE)
    hardware_version , firmware_version = get_version()
    print(f"hardware version : {hardware_version}, firmware version: {firmware_version}")
    prev_buttons = [0, 0, 0, 0]

    V_ref = 3.3
    #read_analog(REG_ANALOG_3)
    V_switch = V_ref/19

    A_voltage = [0.0, 0.0, 0.0]
    A_pos = [0,0,0]

    for i in range(3):
        voltage = read_analog(REG_ANALOG_0 + i)
        pos = round(read_analog(REG_ANALOG_0 + i)/3.3*19)
        A_voltage[i]= voltage
        A_pos[i] = pos
        print(pos)
    print(f"{A_voltage[0]} {A_voltage[1]} {A_voltage[2]}")
    print(f"{A_pos[0]} {A_pos[1]} {A_pos[2]}")

    while True:
        data_pos = []
        data_btn = []
        data_analog = [0.0, 0.0, 0.0]
        for i in range(1,5):
            pos, button = read_encoder(0x00+i)
            if prev_buttons[i-1] == 1 and button == 0:
                set_position(i, 0)  # Reset encoder i to 0
            prev_buttons[i-1] = button 

            data_pos.append(pos)
            data_btn.append(button)
        
        for i in range(3):
            data_analog[i] = read_analog(0x05+i)
            diff = (data_analog[i]-A_voltage[i])/0.18
            A_pos[i] = A_pos[i] + round(diff)
            if 0.7>(diff%0.18) > 0.3:
                A_pos[i] += 1
            A_voltage[i] = data_analog[i]

        print(f"RS1: {A_pos[0]}, RS2=> {A_pos[1]}, RS3: {A_pos[2]}")
        '''
        print(f"RS1=> {a0/V_switch:.2f}: {a0:.2f}V, RS2=> {a1/V_switch:.2f}: {a1:.2f}V, RS3=> {a2/V_switch:.2f}: {a2:.2f}V")

        pos0 = 

        print(f"A0 Position: {pos0}, A1 Position: {pos1}, A2 Position: {pos2}")
        time.sleep(0.1)
        '''
        
        bus.write_byte(I2C_ADDR, 0x10)  
        time.sleep(0.01)
        id_num = bus.read_i2c_block_data(I2C_ADDR, 0x10, unique_id_size)
        #print(f"Encoder position: {data_pos[0]}, {data_pos[1]}, {data_pos[2]}, {data_pos[3]}/ Button pressed: {bool(data_btn[0])}, {bool(data_btn[1])}, {bool(data_btn[2])}, {bool(data_btn[3])}/ Received Unique ID:", " ".join(f"{b:02X}" for b in id_num))

except KeyboardInterrupt:        
    print("exiting")
