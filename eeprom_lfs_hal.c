/*

  eeprom_lfs_hal.c - plugin for allocating unused part of larger EEPROM (>= 128 Kbit) as a littlefs partition

  Part of grblHAL

  Copyright (c) 2025 Terje Io

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

#if LITTLEFS_ENABLE && EEPROM_ENABLE >= 32

#include "littlefs/lfs.h"

#include "grbl/nvs_buffer.h"

typedef struct {
    uint32_t baseaddr;
} lfs_context_t;

static struct {
    memcpy_to_nvs_ptr write_block;
    memcpy_from_nvs_ptr read_block;
} eeprom;

static int lfs_hal_read (const struct lfs_config *c, lfs_block_t block, lfs_off_t off, void *buffer, lfs_size_t size)
{
    assert(block < c->block_count);
    assert(off + size <= c->block_size);

    eeprom.read_block((uint8_t *)buffer, ((lfs_context_t *)c->context)->baseaddr + (block * c->block_size) + off, size, false);

    return LFS_ERR_OK;
}

static int lfs_hal_prog (const struct lfs_config *c, lfs_block_t block, lfs_off_t off, const void *buffer, lfs_size_t size)
{
    assert(block < c->block_count);

    eeprom.write_block(((lfs_context_t *)c->context)->baseaddr + (block * c->block_size) + off, (uint8_t *)buffer, size, false);

    return LFS_ERR_OK;
}

static int lfs_hal_erase (const struct lfs_config *c, lfs_block_t block)
{
    return LFS_ERR_OK;
}

static int lfs_hal_sync (const struct lfs_config *c)
{
    return LFS_ERR_OK;
}

struct lfs_config *eeprom_littlefs_hal (void)
{
    static lfs_context_t ctx;

    static struct lfs_config lfs_cfg = {
        .context = &ctx,
        .read = lfs_hal_read,
        .prog = lfs_hal_prog,
        .erase = lfs_hal_erase,
        .sync = lfs_hal_sync,
        .read_size = 1,
        .prog_size = 256,
        .block_size = 256,
        .cache_size = 256,
        .lookahead_size = 32,
        .block_cycles = 500
    };

    nvs_io_t *nvs = nvs_buffer_get_physical();

    eeprom.read_block = nvs->memcpy_from_nvs;
    eeprom.write_block = nvs->memcpy_to_nvs;

    ctx.baseaddr = max(4096, nvs->size);
    lfs_cfg.block_count = (nvs->size_max - ctx.baseaddr) / 256;

    return &lfs_cfg;
}

#endif // LITTLEFS_ENABLE && EEPROM_ENABLE >= 32
