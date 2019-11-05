// Copyright 2019 BotanicFields, Inc.
// for M5Stack
// UTF-8 to Japanese font 8/16x16
//
#ifndef _BF_M5UTF8_INCLUDED
#define _BF_M5UTF8_INCLUDED

#include <M5Stack.h>

const int UTF8F16_N = 6879;  // entries of k16-font
const int UTF8IMG_N = 512;   // image-buffer size: characters x (8 x 16 dot)

void UTF8_init();

int UTF8str2img(const char* s);
int UTF8jis2fnt(uint16_t jis, uint8_t* f0, uint8_t* f1);
uint32_t UTF8f08_adr(uint16_t jis);
uint32_t UTF8f16_adr(uint16_t jis);
int UTF8utf2jis(uint32_t u0, uint32_t u1, uint32_t u2, uint16_t* jis);
uint16_t UTF8jis(uint32_t u);

void UTF8test_idx();
void UTF8test_jis();
void UTF8test1();
void UTF8test1_sub_0x(char* s);
void UTF8test1_sub_cx(char* s, int u0);
void UTF8test1_sub_ex(char* s, int u0, int u1s, int u1e);
void UTF8test1_sub(const char* s);
void UTF8test2();

#endif // #ifndef _BF_M5UTF8_INCLUDED

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
