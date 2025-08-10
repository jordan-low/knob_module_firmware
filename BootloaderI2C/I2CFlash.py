from intelhex import IntelHex
from smbus2 import SMBus
import time
import sys

I2C_BUS = 1               # Adjust to your I2C bus number
BASE_ADDR = 0x49          # Your bootloader base I2C address
PAGE_SIZE = 64
MAX_CHUNK_SIZE = 31      # Max bytes per I2C block write data (excluding command byte)

def get_i2c_addr(bit2, bit1, bit0):
    return BASE_ADDR + (bit2 << 2) + (bit1 << 1) + bit0

def flash_page(bus, addr, page_addr, data):
    # Step 1: Send page address command (0x00) + 2 bytes word address
    page_word_addr = page_addr // 2  # convert byte address to word address if needed
    addr_bytes = [(page_word_addr >> 8) & 0xFF, page_word_addr & 0xFF]
    bus.write_i2c_block_data(addr, 0x00, addr_bytes)
    time.sleep(0.01)

    # Step 2: Send page data in chunks with offset as command byte
    offset = 0
    while offset < len(data):
        chunk = data[offset : offset + MAX_CHUNK_SIZE]
        # offset as command/register byte, chunk as data bytes
        bus.write_i2c_block_data(addr, offset, chunk)
        offset += len(chunk)
        time.sleep(0.01)

    print(f"Flashed page 0x{page_addr:04X}")

def exit_bootloader(bus, addr):
    bus.write_byte(addr, 0xFF)
    print("Sent exit command, rebooting...")

def main(hexfile, bit2=0, bit1=0, bit0=0):
    i2c_addr = get_i2c_addr(bit2, bit1, bit0)
    print(f"Using I2C address 0x{i2c_addr:02X}")

    ih = IntelHex()
    ih.loadhex(hexfile)

    start_addr = ih.minaddr()
    end_addr = ih.maxaddr()

    # Pad size to PAGE_SIZE boundary
    if (end_addr - start_addr + 1) % PAGE_SIZE != 0:
        padded_size = ((end_addr - start_addr + 1 + PAGE_SIZE -1) // PAGE_SIZE) * PAGE_SIZE
    else:
        padded_size = end_addr - start_addr + 1

    print(f"Flashing {padded_size} bytes from 0x{start_addr:04X} to 0x{start_addr + padded_size -1:04X}")

    with SMBus(I2C_BUS) as bus:
        for page_start in range(start_addr, start_addr + padded_size, PAGE_SIZE):
            data = [ih[addr] if addr <= end_addr else 0xFF for addr in range(page_start, page_start + PAGE_SIZE)]
            flash_page(bus, i2c_addr, page_start, data)
            time.sleep(0.05)

        time.sleep(0.1)
        exit_bootloader(bus, i2c_addr)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 flash_tiny817.py firmware.hex [bit2 bit1 bit0]")
        sys.exit(1)

    hexfile = sys.argv[1]
    bits = list(map(int, sys.argv[2:5])) if len(sys.argv) >= 5 else [0,0,0]
    main(hexfile, *bits)
