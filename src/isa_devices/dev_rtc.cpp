/* Copyright (C) 2024 Freddy VETELE

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. 
If not, see <https://www.gnu.org/licenses/>.
*/

/* dev_rtv.cpp : PicoMEM RTC

  Default RTC port is 0x2C0

  Emulates AST/NS MM58167AN: SixPakPlus V1, Clone I/O boards

 */ 

#include <stdio.h>
#include "pico/stdlib.h"
#include "../pm_gvars.h"
#include "../pm_defines.h"
#include "dev_picomem_io.h"   // SetPortType / GetPortType
#include <sys/time.h>
#include <time.h>
#include <ctime>
#include "hardware/i2c.h"

#define RTC_I2C     0x32

#define RTC_PORT    0x2C0
#define RAM_SIZE    0x10

#define REG_MS      0x00
#define REG_HS      0x01
#define REG_SECONDS 0x02
#define REG_MINUTES 0x03
#define REG_HOURS   0x04
#define REG_DOTW    0x05
#define REG_DAY     0x06
#define REG_MONTH   0x07
#define REG_YEAR    0x0A //offset from 1980

uint8_t ram[RAM_SIZE];
uint8_t dotw_lut[] = {
  0, 1, 2, 0, 3, 0, 0, 0, 
  4, 0, 0, 0, 0, 0, 0, 0, 
  5, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 
  6, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0
};

uint8_t bcd2dec(uint8_t x) {
  return (x>>4)*10 + (x&0x0f);
}

uint8_t dec2bcd(uint8_t x) {
  return (x/10)*0x10 + x%10;
}

uint8_t read_register(uint8_t reg)
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    time_t tsec = tv.tv_sec;
    struct tm *t = gmtime(&tsec);
    uint8_t i2c_data;

    switch (reg)
    {
        case REG_MS:
          return ((tv.tv_usec/1000)%10)<<4;
          break;
        case REG_HS:
          i2c_data = 0x10;
          i2c_write_timeout_us(i2c1, RTC_I2C, &i2c_data, 1, false, 10000);
          if (i2c_read_timeout_us(i2c1, RTC_I2C, &i2c_data, 1, false, 10000))
            return i2c_data;
          else
            return dec2bcd(tv.tv_usec/10000);
          break;
        case REG_SECONDS:
          i2c_data = 0x11;
          i2c_write_timeout_us(i2c1, RTC_I2C, &i2c_data, 1, false, 10000);
          if (i2c_read_timeout_us(i2c1, RTC_I2C, &i2c_data, 1, false, 10000))
            return i2c_data;
          else
            return dec2bcd(t->tm_sec);
          break;
        case REG_MINUTES:
          i2c_data = 0x12;
          i2c_write_timeout_us(i2c1, RTC_I2C, &i2c_data, 1, false, 10000);
          if (i2c_read_timeout_us(i2c1, RTC_I2C, &i2c_data, 1, false, 10000))
            return i2c_data;
          else
            return dec2bcd(t->tm_min);
          break;
        case REG_HOURS:
          i2c_data = 0x13;
          i2c_write_timeout_us(i2c1, RTC_I2C, &i2c_data, 1, false, 10000);
          if (i2c_read_timeout_us(i2c1, RTC_I2C, &i2c_data, 1, false, 10000))
            return i2c_data;
          else
            return dec2bcd(t->tm_hour);
          
            break;
        case REG_DOTW:
          i2c_data = 0x14;
          i2c_write_timeout_us(i2c1, RTC_I2C, &i2c_data, 1, false, 10000);
          if (i2c_read_timeout_us(i2c1, RTC_I2C, &i2c_data, 1, false, 10000))
            return dotw_lut[i2c_data];
          else
            return dec2bcd(t->tm_wday);
          
            break;
        case REG_DAY:
          i2c_data = 0x15;
          i2c_write_timeout_us(i2c1, RTC_I2C, &i2c_data, 1, false, 10000);
          if (i2c_read_timeout_us(i2c1, RTC_I2C, &i2c_data, 1, false, 10000))
            return i2c_data;
          else
            return dec2bcd(t->tm_mday);
          
            break;
        case REG_MONTH:
          i2c_data = 0x16;
          i2c_write_timeout_us(i2c1, RTC_I2C, &i2c_data, 1, false, 10000);
          if (i2c_read_timeout_us(i2c1, RTC_I2C, &i2c_data, 1, false, 10000))
            return i2c_data;
          else
            return dec2bcd(t->tm_mon+1);
          break;
        case REG_YEAR:
          i2c_data = 0x17;
          i2c_write_timeout_us(i2c1, RTC_I2C, &i2c_data, 1, false, 10000);
          if (i2c_read_timeout_us(i2c1, RTC_I2C, &i2c_data, 1, false, 10000))
            return i2c_data + 20;
          else
            return (t->tm_year < 80) ? 0 : (t->tm_year - 80);
          
            break;
        default:
            return ram[reg];
            break;
    }
}

void write_register(uint8_t reg, uint8_t data)
{
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    time_t tsec = tv.tv_sec;
    struct tm *t = gmtime(&tsec);
    uint8_t i2c_data[2];

    i2c_data[1] = data;

    switch (reg)
    {
        case REG_MS:
            tv.tv_usec = 100 * bcd2dec(data);
            break;
        case REG_HS:
            tv.tv_usec = 10000 * bcd2dec(data); 
            break; 
        case REG_SECONDS:
            i2c_data[0] = 0x11; 
            i2c_write_timeout_us(i2c1, RTC_I2C, i2c_data, 2, false, 10000);
            t->tm_sec = bcd2dec(data);
            break;
        case REG_MINUTES:
            i2c_data[0] = 0x12; 
            i2c_write_timeout_us(i2c1, RTC_I2C, i2c_data, 2, false, 10000);
            t->tm_min = bcd2dec(data);
            break;
        case REG_HOURS:
            i2c_data[0] = 0x13; 
            i2c_write_timeout_us(i2c1, RTC_I2C, i2c_data, 2, false, 10000);
            t->tm_hour = bcd2dec(data);
            break;
        case REG_DOTW:
            i2c_data[0] = 0x14; 
            i2c_data[1] = 1 << data;
            i2c_write_timeout_us(i2c1, RTC_I2C, i2c_data, 2, false, 10000);
            t->tm_wday = bcd2dec(data);
            break;
        case REG_DAY:
            i2c_data[0] = 0x15; 
            i2c_write_timeout_us(i2c1, RTC_I2C, i2c_data, 2, false, 10000);
            t->tm_mday = bcd2dec(data);
            break;
        case REG_MONTH:
            i2c_data[0] = 0x16; 
            i2c_write_timeout_us(i2c1, RTC_I2C, i2c_data, 2, false, 10000);
            t->tm_mon = bcd2dec(data)-1;
            break;
        case REG_YEAR:
            if (data > 19) {
              i2c_data[0] = 0x17;
              i2c_data[1] = data - 20; 
              i2c_write_timeout_us(i2c1, RTC_I2C, i2c_data, 2, false, 10000);
            }
            t->tm_year = 80 + data;
            break;
    }
    ram[reg]= data;
    tv.tv_sec = mktime(t);
    tv.tv_usec = 0; /* microseconds */
    settimeofday(&tv, nullptr);
}

uint8_t dev_rtc_install()
{
  SetPortType(RTC_PORT,DEV_RTC,RAM_SIZE/0x08);

  return 0;
}

void dev_rtc_remove()
{
  SetPortType(RTC_PORT,DEV_NULL,RAM_SIZE/0x08);
}

// Return true if the Joystick is installed
bool dev_rtc_installed()
{
  return (GetPortType(RTC_PORT)==DEV_RTC);
}

// Started in the Main Command Wait Loop
void dev_rtc_update()
{
}

bool dev_rtc_ior(uint32_t CTRL_AL8,uint8_t *Data )
{
  *Data = read_register(CTRL_AL8&(RAM_SIZE-1));

  return true;
}

void dev_rtc_iow(uint32_t CTRL_AL8,uint32_t ISAIOW_Data)
{
  write_register(CTRL_AL8&(RAM_SIZE-1), ISAIOW_Data&0x00ff);
}


