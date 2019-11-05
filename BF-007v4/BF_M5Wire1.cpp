// Copyright 2019 BotanicFields, Inc.
// another I2C interface named "Wire1"
// for HT16K33 to avoid duplicated address of I2C of M5Stack
//
#include "BF_M5Wire1.h"

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// I2C init .. another I2C interface named "Wire1"
void wire1_init() {
  Wire1.begin(WIRE1_SDA, WIRE1_SCL, WIRE1_FRQ);
}

// I2C read, return bytes transferred
uint8_t wire1_read(uint8_t address, uint8_t command, uint8_t rd_size, uint8_t* rd_data) {
  Wire1.beginTransmission(address);
  Wire1.write(command);
  if(Wire1.endTransmission(false) != 0)  // stop = false ..keep connection
    return 0;  // command error
  Wire1.requestFrom(address, rd_size);
  uint8_t i = 0;
  while(Wire1.available())
    rd_data[i++] = Wire1.read();
  return i;
}

// I2C write, return status
uint8_t wire1_write(uint8_t address, uint8_t command, uint8_t wt_size, uint8_t* wt_data) {
  Wire1.beginTransmission(address);
  Wire1.write(command);
  if(wt_size != 0)
    Wire1.write(wt_data, wt_size);
  return Wire1.endTransmission();
}

/* 
The MIT License
SPDX short identifier: MIT

Copyright 2019 BotanicFields, Inc.

Permission is hereby granted, free of charge, to any person obtaining a copy 
of this software and associated documentation files (the "Software"), to deal 
in the Software without restriction, including without limitation the rights 
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom the Software is 
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all 
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
SOFTWARE.
*/
