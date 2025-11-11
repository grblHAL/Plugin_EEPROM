/*

  eeprom_24AAxxx.c - plugin for I2C EEPROM (Microchip 24AAxxx > 16kbit, 2 byte address)

  NOTE: only tested with 24AA256

  Part of grblHAL

  Copyright (c) 2020-2025 Terje Io

  grblHAL is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  grblHAL is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with grblHAL. If not, see <http://www.gnu.org/licenses/>.

*/

#include "driver.h"

#if EEPROM_ENABLE >= 32 || EEPROM_ENABLE == 2 || EEPROM_ENABLE == 3

#include "eeprom.h"

#include "grbl/hal.h"
#include "grbl/crc.h"
#include "grbl/nuts_bolts.h"

#define EEPROM_I2C_ADDRESS (0xA0 >> 1)
#if EEPROM_ENABLE == 2 || EEPROM_ENABLE >= 128
#define EEPROM_PAGE_SIZE 64
#else
#define EEPROM_PAGE_SIZE 32
#endif

static nvs_transfer_t i2c = { .word_addr_bytes = 2 };

static uint8_t getByte (uint32_t addr)
{
    uint8_t value = 0;

    i2c.address = EEPROM_I2C_ADDRESS;
    i2c.word_addr = addr;
    i2c.data = &value;
    i2c.count = 1;

    i2c_nvs_transfer(&i2c, true);

    return value;
}

static void putByte (uint32_t addr, uint8_t new_value)
{
    i2c.address = EEPROM_I2C_ADDRESS;
    i2c.word_addr = addr;
    i2c.data = &new_value;
    i2c.count = 1;

    i2c_nvs_transfer(&i2c, false);
}

static nvs_transfer_result_t writeBlock (uint32_t destination, uint8_t *source, uint32_t size, bool with_checksum)
{
    uint32_t remaining = size;
    uint8_t *target = source;
    nvs_transfer_result_t res = NVS_TransferResult_OK;

    while(remaining > 0) {

        i2c.address = EEPROM_I2C_ADDRESS;
        i2c.word_addr = destination;
        i2c.count = EEPROM_PAGE_SIZE - (destination & (EEPROM_PAGE_SIZE - 1));
        i2c.count = remaining < i2c.count ? remaining : i2c.count;
        i2c.data = target;
        remaining -= i2c.count;
        target += i2c.count;
        destination += i2c.count;

        if((res = i2c_nvs_transfer(&i2c, false)) != NVS_TransferResult_OK)
            break;

        eeprom_write_delay();
    }

    if(size > 0 && with_checksum) {
        uint16_t checksum = calc_checksum(source, size);
        putByte(destination, checksum & 0xFF);
#if NVS_CRC_BYTES > 1
        eeprom_write_delay();
        putByte(++destination, checksum >> 8);
#endif
        eeprom_write_delay();
    }

    return res;
}

static nvs_transfer_result_t readBlock (uint8_t *destination, uint32_t source, uint32_t size, bool with_checksum)
{
    uint32_t remaining = size;
    uint8_t *target = destination;

    while(remaining) {

        i2c.address = EEPROM_I2C_ADDRESS;
        i2c.word_addr = source;
        i2c.count = remaining > 255 ? 255 : (uint8_t)remaining;
        i2c.data = target;
        remaining -= i2c.count;
        target += i2c.count;
        source += i2c.count;

        i2c_nvs_transfer(&i2c, true);
    }

#if NVS_CRC_BYTES == 1
    return !with_checksum || calc_checksum(destination, size) == getByte(source);
#else
    return !with_checksum || calc_checksum(destination, size) == (getByte(source) | (getByte(source + 1) << 8));
#endif
}

void i2c_eeprom_init (void)
{
    if(i2c_start().ok && i2c_probe(EEPROM_I2C_ADDRESS)) {
#if EEPROM_IS_FRAM
        hal.nvs.type = NVS_FRAM;
#else
        hal.nvs.type = NVS_EEPROM;
#endif
#if EEPROM_ENABLE >= 32
        hal.nvs.size_max = (EEPROM_ENABLE / 8) * 1024;
#endif
        hal.nvs.get_byte = getByte;
        hal.nvs.put_byte = putByte;
        hal.nvs.memcpy_to_nvs = writeBlock;
        hal.nvs.memcpy_from_nvs = readBlock;
    }
}

#endif
