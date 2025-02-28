/*

  eeprom.h - plugin for for I2C EEPROM or FRAM

  Part of grblHAL

  Copyright (c) 2017-2025 Terje Io

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

#pragma once

void i2c_eeprom_init (void);

static inline void eeprom_write_delay (void)
{
#if EEPROM_ENABLE && !EEPROM_IS_FRAM
    hal.delay_ms(5, NULL);
#endif
}
