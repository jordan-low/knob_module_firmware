import smbus2
import time

# I2C setup
I2CBUS = 7
I2C_ADDR_1 = 0x51  # internal knob
unique_id_size = 10
SAMPLING_RATE = 100
bus = smbus2.SMBus(I2CBUS)

# Register addresses
REG_ENCODER_2 = 0x02
REG_ANALOG_3 = 0x08
REG_VERSION = 0xFE
REG_LED_COLOR = 0x20

current_color = 0  # 0 = Red, 1 = Green, 2 = Blue

def cycle_led_color():
    global current_color
    current_color = (current_color + 1) % 3
    bus.write_i2c_block_data(I2C_ADDR_1, REG_LED_COLOR, [current_color])
    print(f"LED color cycled to: {['Red', 'Green', 'Blue'][current_color]}")


def lookup_target(raw_value, lut):
    closest = min(lut, key=lambda pair: abs(pair[0] - raw_value))
    return closest[1]

def set_position(index, value):
    lsb = value & 0xFF
    msb = (value >> 8) & 0xFF
    bus.write_i2c_block_data(I2C_ADDR_1, 0x00 + index, [lsb, msb])
    time.sleep(0.01)

def read_encoder(reg):
    bus.write_i2c_block_data(I2C_ADDR_1, 0x00, [reg])
    time.sleep(0.01)
    data = bus.read_i2c_block_data(I2C_ADDR_1, reg, 3)
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
    bus.write_i2c_block_data(I2C_ADDR_1, SAMPLING_RATE, [value])

def get_version():
    bus.write_byte(I2C_ADDR_1, REG_VERSION)
    time.sleep(0.01)
    data = bus.read_i2c_block_data(I2C_ADDR_1, REG_VERSION, 4)
    return f"{data[0]}.{data[1]}", f"{data[2]}.{data[3]}"


try:
    set_sampling_rate(SAMPLING_RATE)
    hardware_version, firmware_version = get_version()
    print(f"hardware version : {hardware_version}, firmware version: {firmware_version}")

    prev_buttons = 0
    prev_positions = 0
    prev_adc = 0.0

    while True:
        changed = False
        data_pos = []
        data_btn = []

        # Internal knob encoders
        for i in range(4):
            reg = 0x01 + i
            pos, button = read_encoder(reg)

            # Check encoder change
            if pos != prev_positions[i]:
                changed = True
                prev_positions[i] = pos

            # Check button press release
            if prev_buttons[i] == 1 and button == 0:
                set_position(i + 1, 0)

            # Check button change
            if button != prev_buttons[i]:
                changed = True
                prev_buttons[i] = button

            data_pos.append(pos)
            data_btn.append(button)

        # Internal ADC
        adc = read_analog(I2C_ADDR_1, REG_ANALOG_3)
        if abs(adc - prev_adc) > 0.05:  # noise threshold
            changed = True
            prev_adc = adc

        if changed:
            cycle_led_color()

        # Print status
        print(f"Internal Encoder pos: {data_pos[1]}, ADC: {adc:.2f}, LED color: {['Red', 'Green', 'Blue'][current_color]}")

        time.sleep(0.05)

except KeyboardInterrupt:
    print("Exiting...")
