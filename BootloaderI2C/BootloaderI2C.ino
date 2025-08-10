#include <avr/io.h>
#include <avr/interrupt.h>

#define BASE_ADDR 0x49
#define PAGE_SIZE 64
#define CMD_WRITE_PART 0x01
#define CMD_EXIT 0xFF
#define BOOTLOADER_TIMEOUT_MS 5000
#ifndef TCB_CLKSEL_DIV64_gc
#define TCB_CLKSEL_DIV64_gc 0x5
#endif

// RAM flag to stay in bootloader after reset
volatile uint8_t stay_in_bootloader __attribute__((section(".noinit")));

volatile uint8_t rx_len = 0;
volatile uint8_t rx_buf[34];  // Max I2C block ~32 + command + 2 addr bytes
volatile uint8_t write_buffer[PAGE_SIZE];
volatile uint16_t page_address = 0xFFFF;  // Invalid at start
volatile uint8_t write_offset = 0;

volatile uint32_t millis_count = 0;
uint32_t last_activity_ms = 0;

// Timer initialization for 1ms ticks using TCB0
void timer_init() {
  // Adjust these values for your clock frequency
  TCB0.CCMP = 52; // ~1ms interval @3.33MHz CLK_PER / 64 prescaler
  TCB0.INTCTRL = TCB_CAPT_bm; // Enable compare interrupt
  TCB0.CTRLA = TCB_CLKSEL_DIV64_gc | TCB_ENABLE_bm;
}

// TCB0 ISR increments millis_count
ISR(TCB0_INT_vect) {
  millis_count++;
  TCB0.INTFLAGS = TCB_CAPT_bm;
}

uint32_t millis() {
  uint32_t m;
  cli();
  m = millis_count;
  sei();
  return m;
}

// Wait until NVM ready
static void nvm_wait_ready(void) {
  while (NVMCTRL.STATUS & NVMCTRL_FBUSY_bm);
}

#ifndef NVMCTRL_CMD_ERASE_WRITE_PAGE_gc
#define NVMCTRL_CMD_ERASE_WRITE_PAGE_gc 0x05
#endif
#ifndef NVMCTRL_CMDEX_KEY_gc
#define NVMCTRL_CMDEX_KEY_gc 0xA5
#endif

// Erase and write one flash page atomically
void flash_erase_write_page(uint32_t addr, const uint8_t *data) {
  nvm_wait_ready();

  uint8_t *flash_ptr = (uint8_t *)addr;
  for (uint8_t i = 0; i < PAGE_SIZE; i++) {
    flash_ptr[i] = data[i];
  }

  uint16_t word_addr = addr >> 1;
  NVMCTRL.ADDR = (uint8_t)(word_addr & 0xFF);
  NVMCTRL.ADDRH = (uint8_t)(word_addr >> 8);

  NVMCTRL.CTRLA = NVMCTRL_CMD_ERASE_WRITE_PAGE_gc | NVMCTRL_CMDEX_KEY_gc;

  nvm_wait_ready();
}

// Detect I2C address from PA3, PA5, PA6 pins with pull-ups
uint8_t detect_i2c_addr() {
  PORTA.PIN3CTRL = PORT_PULLUPEN_bm;
  PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
  PORTA.PIN6CTRL = PORT_PULLUPEN_bm;

  uint8_t bit0 = (VPORTA.IN & (1 << 3)) ? 1 : 0;
  uint8_t bit1 = (VPORTA.IN & (1 << 5)) ? 1 : 0;
  uint8_t bit2 = (VPORTA.IN & (1 << 6)) ? 1 : 0;

  uint8_t offset = (bit2 << 2) | (bit1 << 1) | bit0;
  return BASE_ADDR + offset;
}

// Jump to application reset vector at 0x0000
void jump_to_app() {
  stay_in_bootloader = 0;
  void (*app_start)(void) = 0x0000;
  app_start();
}

// Initialize TWI0 as I2C slave at given address
void i2c_init(uint8_t addr) {
  TWI0.SADDR = addr << 1; // 7-bit addr shifted for SADDR
  TWI0.SCTRLA = TWI_ENABLE_bm | TWI_DIEN_bm | TWI_APIEN_bm | TWI_PIEN_bm;
  TWI0.SCTRLB = TWI_SMEN_bm | TWI_SCMD_NOACT_gc;
}

// TWI0 Slave ISR for RX handling and partial write accumulation
ISR(TWI0_TWIS_vect) {
  uint8_t status = TWI0.SSTATUS;

  if (status & TWI_APIF_bm) {
    TWI0.SSTATUS = TWI_APIF_bm; // Clear address/stop flag
    rx_len = 0;
  }

  if (status & TWI_DIF_bm) {
    uint8_t data = TWI0.SDATA;

    if (rx_len == 0) {
      // First byte = command
      if (data == CMD_WRITE_PART) {
        write_offset = 0; // reset write offset for new write
        page_address = 0xFFFF; // reset page address until addr received
      } else if (data == CMD_EXIT) {
        jump_to_app();
      }
      rx_len++;
    } else if (rx_len == 1) {
      // addr high byte for first packet of page
      page_address = (uint16_t)data << 8;
      rx_len++;
    } else if (rx_len == 2) {
      // addr low byte for first packet of page
      page_address |= data;
      rx_len++;
    } else {
      // Data bytes (partial page data)
      if (page_address == 0xFFFF) {
        // Invalid address, ignore data
        rx_len = 0;
        return;
      }
      if (write_offset < PAGE_SIZE) {
        write_buffer[write_offset++] = data;
      }
      rx_len++;
    }

    // If buffer full, write page and reset
    if (write_offset >= PAGE_SIZE) {
      flash_erase_write_page(page_address, (const uint8_t *)write_buffer);
      page_address = 0xFFFF;  // reset to invalid
      write_offset = 0;
      rx_len = 0;
    }

    TWI0.SSTATUS = TWI_DIF_bm;  // Clear data interrupt flag
    last_activity_ms = millis();
  }
}

int main(void) {
  cli();

  timer_init();

  if (stay_in_bootloader == 0) {
    jump_to_app();
  }

  uint8_t addr = detect_i2c_addr();
  i2c_init(addr);

  sei();

  last_activity_ms = millis();

  while (1) {
    uint32_t current_ms = millis();

    // Timeout auto exit bootloader after inactivity
    if ((current_ms - last_activity_ms) > BOOTLOADER_TIMEOUT_MS) {
      stay_in_bootloader = 0;
      jump_to_app();
    }
  }

  return 0;
}
