import smbus
import time

slave_address = 0x08
unique_id_size = 10
bus = smbus.SMBus(7)  

try:
    while True:
        data = bus.read_i2c_block_data(slave_address, 0x00, unique_id_size)
        print("Received Unique ID:", " ".join(f"{b:02X}" for b in data))
        time.sleep(2)
except KeyboardInterrupt:        
    print("exiting")