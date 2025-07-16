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
REG_ENCODER_POS = 0x20
REG_ENCODER_BTN = 0x30
REG_VERSION = 0xFE

# Lookup tables for raw to target value mapping
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

def lookup_target(raw_value, lut):
    closest = min(lut, key=lambda pair: abs(pair[0] - raw_value))
    return closest[1]

def set_position(index, value):
    lsb = value & 0xFF
    msb = (value >> 8) & 0xFF
    bus.write_i2c_block_data(I2C_ADDR, 0x00 + index, [lsb, msb])
    time.sleep(0.01)

def read_encoder(reg):
    bus.write_i2c_block_data(I2C_ADDR, 0x00, [reg])
    time.sleep(0.01)
    data = bus.read_i2c_block_data(I2C_ADDR, reg, 3)
    pos = data[0] | (data[1] << 8)
    if pos & 0x8000:
        pos -= 0x10000
    button = data[2]
    return pos, button

def read_analog(reg):
    bus.write_byte(I2C_ADDR, reg)
    time.sleep(0.01)
    data = bus.read_i2c_block_data(I2C_ADDR, reg, 2)
    value = (data[0] << 8) | data[1]
    value = value / 1024 * 3.3
    return value

def set_sampling_rate(value):
    bus.write_i2c_block_data(I2C_ADDR, SAMPLING_RATE, [value])

def get_version():
    bus.write_byte(I2C_ADDR, REG_VERSION, 4)
    time.sleep(0.01)
    data = bus.read_i2c_block_data(I2C_ADDR, REG_VERSION, 4)
    hardware_id = f"{data[0]}.{data[1]}"
    software_id = f"{data[2]}.{data[3]}"
    return hardware_id, software_id

try:
    set_sampling_rate(SAMPLING_RATE)
    hardware_version, firmware_version = get_version()
    print(f"hardware version : {hardware_version}, firmware version: {firmware_version}")
    prev_buttons = [0, 0, 0, 0]

    while True:
        data_pos = []
        data_btn = []
        for i in range(1, 5):
            pos, button = read_encoder(0x00 + i)
            if prev_buttons[i - 1] == 1 and button == 0:
                set_position(i, 0)  # Reset encoder i to 0
            prev_buttons[i - 1] = button

            data_pos.append(pos)
            data_btn.append(button)

        a0 = read_analog(REG_ANALOG_0)
        a1 = read_analog(REG_ANALOG_1)
        a2 = read_analog(REG_ANALOG_2)
        a3 = read_analog(REG_ANALOG_3)  # V_REF

        V_switch = a3 / 19

        scaled_a0 = a0 / V_switch
        scaled_a1 = a1 / V_switch
        scaled_a2 = a2 / V_switch

        target_rs1 = lookup_target(scaled_a0, rs1_table)
        target_rs2 = lookup_target(scaled_a1, rs2_table)
        target_rs3 = lookup_target(scaled_a2, rs3_table)

        print(f"RS1=> {scaled_a0:.2f} (raw), {target_rs1} (target)")
        print(f"RS2=> {scaled_a1:.2f} (raw), {target_rs2} (target)")
        print(f"RS3=> {scaled_a2:.2f} (raw), {target_rs3} (target)")

        bus.write_byte(I2C_ADDR, 0x10)
        time.sleep(0.01)
        id_num = bus.read_i2c_block_data(I2C_ADDR, 0x10, unique_id_size)
        # Uncomment below to print encoder positions, buttons and unique ID
        # print(f"Encoder position: {data_pos}, Button pressed: {data_btn}, Received Unique ID:",
        #       " ".join(f"{b:02X}" for b in id_num))

except KeyboardInterrupt:
    print("exiting")
