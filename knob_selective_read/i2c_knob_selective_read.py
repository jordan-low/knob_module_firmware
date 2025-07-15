import smbus2
import time

I2C_ADDR = 0x49
bus = smbus2.SMBus(8)

def read_analog_channel(channel):
    bus.write_byte(I2C_ADDR,channel) #send one byte command
    time.sleep(0.05)    #allow time for uC process
    
    data = bus.read_i2c_block_data(I2C_ADDR,0,2) #read from the slave device
    value = (data[0]<<8 | data[1])
    return value


try:
    while True:
        analogdata =[]
        for i in range(4):
            adcvalue = read_analog_channel(i)
            adcvalue = adcvalue/1024 *3.3
            analogdata.append(adcvalue)
        print(f"analog readings: {analogdata[0]:.2f} {analogdata[1]:.2f} {analogdata[2]:.2f} {analogdata[3]:.2f}")
except KeyboardInterrupt:
    print ("Exiting")
    bus.close()
            