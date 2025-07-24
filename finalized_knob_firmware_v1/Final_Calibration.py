import smbus2
import time

#I2C addressing
I2CBUS = 7
I2C_ADDR_1 = 0x51 #internal knob
I2C_ADDR_2 = 0x49 #operational board
unique_id_size = 10
SAMPLING_RATE = 100
bus = smbus2.SMBus(I2CBUS)

#Register addressing
REG_ENCODER_1 = 0x01
REG_ENCODER_2 = 0x02
REG_ENCODER_3 = 0x03
REG_ENCODER_4 = 0x04
REG_ANALOG_0 = 0x05
REG_ANALOG_1 = 0x06
REG_ANALOG_2 = 0x07
REG_ANALOG_3 = 0x08
REG_ENCODER_POS = 0x20
REG_ENCODER_BTN = 0x30
REG_VERSION = 0xFE

# Calibration table for operational knobs
rs1_table = [
    (19.00, 19), (18.57, 18), (17.7, 17), (16.53, 16), (15.42, 15), (14.34, 14),
    (13.19, 13), (12.03, 12), (10.77, 11), (9.49, 10), (8.23, 9), (7.02, 8),
    (5.81, 7), (4.64, 6), (3.53, 5), (2.4, 4), (1.28, 3), (0.43, 2), (0, 1)
]
rs2_table = [
    (19, 19), (18.68, 18), (17.95, 17), (16.85, 16), (15.75, 15), (14.67, 14),
    (13.6, 13), (12.44, 12), (11.24, 11), (10.06, 10), (8.8, 9), (7.65, 8),
    (6.43, 7), (5.27, 6), (4.1, 5), (2.95, 4), (1.76, 3), (0.93, 2), (0.52, 1)
]
rs3_table = [
    (19, 19), (18.57, 18), (17.7, 17), (16.64, 16), (15.58, 15), (14.53, 14),
    (13.47, 14), (12.31, 12), (11.16, 11), (10.03, 10), (8.9, 9), (7.67, 8),
    (6.52, 7), (5.37, 6), (4.24, 5), (3.08, 4), (1.89, 3), (1, 2), (0.52, 1)
]

#functions
def lookup_target(raw_value, lut):
    closest = min(lut, key=lambda pair: abs(pair[0] - raw_value))
    return closest[1]

def set_position(I2C_ADDR, index, value):
    lsb = value & 0xFF
    msb = (value >> 8) & 0xFF
    bus.write_i2c_block_data(I2C_ADDR, 0x00 + index, [lsb, msb])
    time.sleep(0.01)

def read_encoder(I2C_ADDR, reg):
    bus.write_i2c_block_data(I2C_ADDR, 0x00, [reg])
    time.sleep(0.01)
    data = bus.read_i2c_block_data(I2C_ADDR, reg, 3)
    pos = data[0] | (data[1] << 8)
    if pos & 0x8000:
        pos -= 0x10000
    button = data[2]
    return pos, button

def read_analog(I2C_ADDR, reg):
    bus.write_byte(I2C_ADDR, reg)
    time.sleep(0.01)
    data = bus.read_i2c_block_data(I2C_ADDR, reg, 2)
    value = (data[0] << 8) | data[1]
    value = value / 1024 * 3.3
    return value

def set_sampling_rate(value):
    bus.write_i2c_block_data(I2C_ADDR, SAMPLING_RATE, [value])

def get_version(I2C_ADDR):
    bus.write_byte(I2C_ADDR, REG_VERSION, 4)
    time.sleep(0.01)
    data = bus.read_i2c_block_data(I2C_ADDR, REG_VERSION, 4)
    hardware_id = f"{data[0]}.{data[1]}"
    software_id = f"{data[2]}.{data[3]}"
    return hardware_id, software_id

#main
try:
    #get version
    set_sampling_rate(SAMPLING_RATE)
    hardware_version, firmware_version = get_version()
    print(f"hardware version : {hardware_version}, firmware version: {firmware_version}")
    prev_buttons = [0, 0, 0, 0]

    #internal knob(encoder)
    while True:
        data_pos = []
        data_btn = []
        for i in range(1, 5):
            pos, button = read_encoder(I2C_ADDR_1, (0x00 + i))
            #reset encoder value when button pressed
            if prev_buttons[i - 1] == 1 and button == 0:
                set_position(i, 0)  # Reset encoder i to 0
            prev_buttons[i - 1] = button
            #read encoder values (position, button status)
            data_pos.append(pos)
            data_btn.append(button)
        #internal knob(ADC)
        a3_1 = read_analog(I2C_ADDR_1, REG_ANALOG_3)

        #operational knob
        a0_2 = read_analog(I2C_ADDR_2, REG_ANALOG_0)
        a1_2 = read_analog(I2C_ADDR_2, REG_ANALOG_1)
        a2_2 = read_analog(I2C_ADDR_2, REG_ANALOG_2)
        a3_2 = read_analog(I2C_ADDR_2, REG_ANALOG_3)  # V_REF

        V_switch = a3_2 / 19     # voltage per switch

        #convert analog value to pos
        scaled_a0 = a0_2 / V_switch
        scaled_a1 = a1_2 / V_switch
        scaled_a2 = a2_2 / V_switch

        target_rs1 = lookup_target(scaled_a0, rs1_table)
        target_rs2 = lookup_target(scaled_a1, rs2_table)
        target_rs3 = lookup_target(scaled_a2, rs3_table)


        bus.write_byte(I2C_ADDR_1, 0x10)
        bus.write_byte(I2C_ADDR_2, 0x10)
        time.sleep(0.01)
        id_num_1 = bus.read_i2c_block_data(I2C_ADDR_1, 0x10, unique_id_size)
        id_num_2 = bus.read_i2c_block_data(I2C_ADDR_2, 0x10, unique_id_size)

        # print for internal knob
        print(f"Encoder position: {data_pos[1]}, ADC: {a3_1:.2f} Received Unique ID:",
           " ".join(f"{b:02X}" for b in id_num_1))
        
         #print for operational knob 
        print(f"RS1=> {target_rs1} RS2=> {target_rs2} RS3=> {target_rs3} (target) Received Unique ID:",
           " ".join(f"{b:02X}" for b in id_num_2))
        
        

except KeyboardInterrupt:
    print("exiting")
