import smbus2
import time

I2C_ADDR = 0x49
NUM_CHANNELS = 4
BYTES_PER_CHANNEL = 2

bus = smbus2.SMBus(8)

def read_analogvalues():
    #Request data from ATTiny
    num_bytes = NUM_CHANNELS*BYTES_PER_CHANNEL
    data = bus.read_i2c_block_data(I2C_ADDR,0,num_bytes) # DEVICE ADDRESS, RESIGTER ADDRESS, BLOCK SIZE
    
    #convert data back to analog values
    analog_values = [] #create an array
    for i in range(0, len(data), 2):
        value = (data[i]<<8 | data[i+1])
        value = (value/1024)*3.3
        analog_values.append(value) #add to array
    return analog_values                      

try:
    while True:
        analogs = read_analogvalues()
        print(f"analog readings: {analogs[0]:.2f} {analogs[1]:.2f} {analogs[2]:.2f} {analogs[3]:.2f}")
        time.sleep(1)
except KeyboardInterrupt:
    print("Exiting")
except OSError:
    print(OSError)
    