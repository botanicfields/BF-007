// Copyright 2019 BotanicFields, Inc.
// another I2C interface named "Wire1"
// for HT16K33 to avoid duplicated address of I2C of M5Stack
//
#ifndef _BF_M5WIRE1_H_INCLUDED
#define _BF_M5WIRE1_H_INCLUDED

#include <Arduino.h>
#include <Wire.h>

// pin assignment
const uint8_t  WIRE1_SDA = 26;      // GPIO26
const uint8_t  WIRE1_SCL = 13;      // GPIO13
const uint32_t WIRE1_FRQ = 400000;  // 400kHz

void wire1_init();
uint8_t wire1_read(uint8_t address, uint8_t command, uint8_t rd_size = 0, uint8_t* rd_data = NULL);
uint8_t wire1_write(uint8_t address, uint8_t command, uint8_t wt_size = 0, uint8_t* wt_data = NULL);

#endif  // #ifndef _BF_M5WIRE1_H_INCLUDED

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
