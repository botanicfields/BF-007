// Copyright 2019 BotanicFields, Inc.
// BF-007
// M5Stack/ESP32 + HT16K33 + LED Matrix 16 x 64 (8 x 8 x 16)
//
#include "BF_M5Wire1.h"
#include "BF_HT16K33_matrix.h"

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// global valiables
HT16K33_t Ht16k33;

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// I2C controls
// initialize I2C registers
void HT16K33_init() {

  // setup registers 
  wire1_init();
  for(int i = 0; i < HT16K33_QTY; i++) {
    HT16K33_write(i, 0x21);  // system setup: turn on system oscillator  
    HT16K33_write(i, 0xa0);  // ROW/INT set: set to ROW driver output
    HT16K33_write(i, 0xef);  // dimming set: 16/16 duty
    HT16K33_write(i, 0x81);  // display setup: display on, blinking off 
  }

  // initial pattern: address
  for(int i = 0; i < HT16K33_QTY; i++)
    for(int j = 0; j < HT16K33_RAM; j++)
      Ht16k33.ram[i][j] = i * HT16K33_RAM + j;
  HT16K33_update();
}

// I2C read
void HT16K33_read(uint8_t address, uint8_t command, uint8_t rd_size, uint8_t* rd_data) {
  if(wire1_read(HT16K33_I2C + address, command, rd_size, rd_data) == 0)
    Serial.printf("[HT16K33] wire1 read error %d %d %d\n", address, command, rd_size);
}

// I2C write
void HT16K33_write(uint8_t address, uint8_t command, uint8_t wt_size, uint8_t* wt_data) {
  if(wire1_write(HT16K33_I2C + address, command, wt_size, wt_data) != 0)
    Serial.printf("[HT16K33] wire1 write error %d %d %d\n", address, command, wt_size);
}

// send image
void HT16K33_update() {
  for(int i = 0; i < HT16K33_QTY; i++)
    HT16K33_write(i, 0x00, HT16K33_RAM, Ht16k33.ram[i]);
}

// set dimmer
//   dimmer = 0..15 : 1/16 .. 16/16
void HT16K33_dimmer(uint8_t address, uint8_t dimmer) {
  HT16K33_write(address, 0xe0 + dimmer);
}

// set blinking
//   blink = 0(off), 1(2Hz), 2(1Hz), 3(0.5Hz)
void HT16K33_blink(uint8_t address, uint8_t blink) {
  HT16K33_write(address, 0x81 + (blink << 1));
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// draw images
// clear
void HT16K33_clear(uint16_t c) {
  for(int x = 0; x < HT16K33_X; x++)
    Ht16k33.img[x] = c;
}

// draw dot
void HT16K33_plot(int x, int y, int c) {
  if((x >= 0) && (x < HT16K33_X) && (y >= 0) && (y < HT16K33_Y)) {
    uint16_t m = 0x0001 << (y & 0x000f);
    switch(c) {
      case 0: Ht16k33.img[x] &= ~m;  break;  // dot reset
      case 1: Ht16k33.img[x] |=  m;  break;  // dot set
      case 2: Ht16k33.img[x] ^=  m;  break;  // dot revert
      default: break;
    }
  }
}

// draw font  
void HT16K33_char(int x, int y, int c, char ch) {
  if((ch < 0x20) || (ch > 0x7f))
    ch = 0x7f;
  if((y >= - FONT_Y) && (y < HT16K33_Y)) {
    for(int i = 0; i < FONT_X; i++) {
      int xi = x + i;
      if((xi >= 0) && (xi < HT16K33_X)) {
        uint16_t f = uint16_t(FONT[ch - 0x20][i]);
        uint16_t m = 0x00FF;
        if(y >= 0) {
          f <<= y;
          m <<= y;
        }
        else {
          f >>= - y;
          m >>= - y;
        }
        uint16_t* p = &Ht16k33.img[xi];
        switch(c) {
          case 0: *p = (*p |  m) & ~f;  break;
          case 1: *p = (*p & ~m) |  f;  break;
          case 2: *p = (*p     ) ^  f;  break;
          default: break;
        }
      }
    }
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// Bresenham's line algorithm
void HT16K33_line(int x1, int y1, int x2, int y2, int c) {
  if((x1 == x2) && (y1 == y2)) {
    HT16K33_plot(x1, y1, c);
  }
  else {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    if(dx >= dy) {
      // x-based
      if(x2 < x1) {
        swapInt(x2, x1);
        swapInt(y2, y1);
      }
      int s = (y2 > y1)? 1: -1;
      int d = 0;
      int y = y1;
      for(int x = x1; x <= x2; x++) {
        HT16K33_plot(x, y, c);
        d += dy * 2;
        if(d >= dx) {
          y += s;
          d -= dx * 2;
        }
      }
    }
    else {
      // y-based
      if(y2 < y1) {
        swapInt(x2, x1);
        swapInt(y2, y1);
      }
      int s = (x2 > x1)? 1: -1;
      int d = 0;
      int x = x1;
      for(int y = y1; y <= y2; y++) {
        HT16K33_plot(x, y, c);
        d += dx * 2;
        if(d >= dy) {
          x += s;
          d -= dy * 2;
        }
      }
    }
  }
  HT16K33_update();
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// Bresenham's circle algorithm
void HT16K33_circle(int x0, int y0, int r, int c) {
  int x = r;
  int y = 0;
  int d = -2 * r + 3;
  while(x >= y) {
    HT16K33_plot(x0 + x, y0 + y, c);
    HT16K33_plot(x0 - x, y0 + y, c);
    HT16K33_plot(x0 + x, y0 - y, c);
    HT16K33_plot(x0 - x, y0 - y, c);
    HT16K33_plot(x0 + y, y0 + x, c);
    HT16K33_plot(x0 - y, y0 + x, c);
    HT16K33_plot(x0 + y, y0 - x, c);
    HT16K33_plot(x0 - y, y0 - x, c);
    if(d >= 0) {
      x--;
      d -= 4 * x;
    }
    y++;
    d += 4 * y + 2;
  }    
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// print string
void HT16K33_string(int x, int y, int c, const char* s) {
  int len = strlen(s);
  for(int i = 0; i < len; i++)
    HT16K33_char(x + FONT_X * i, y, c, s[i]);
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// flow 2 strings
void HT16K33_flow2(int c1, int c2, int d1, int d2, const char* s1, const char* s2){
  int x1 = HT16K33_X;              // start at right end 
  int x2 = HT16K33_X;
  int e1 = - strlen(s1) * FONT_X;  // end at far left length
  int e2 = - strlen(s2) * FONT_X;
  boolean bsy1 = true;
  boolean bsy2 = true;
  long t = 0;

  while(bsy1 || bsy2) {
    boolean upd1 = false;
    if(t % d1 == 0) {
      HT16K33_string(x1, 0, c1, s1);
      upd1 = true;
      if(--x1 < e1) bsy1 = false;
    }
    boolean upd2 = false;
    if(t % d2 == 0) {
      HT16K33_string(x2, 8, c2, s2);
      upd2 = true;
      if(--x2 < e2) bsy2 = false;
    }
    if(upd1 || upd2) HT16K33_update();
    t += 10;
    delay(10);
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// subfuntions
// swap interger
void swapInt(int &x, int &y) {
  int t = x; x = y; y = t;
}

// insert a decimal into a string 
void int2decStr(unsigned long n, int pos, int len, char* s) {
  for(int i = 0; i < len; i++) {
    int m = n % 10;
    n /= 10;
    s[pos + len - 1 - i] = '0' + m;
  }
}

// insert a hexadecimal into a string 
void int2hexStr(unsigned long n, int pos, int len, char* s) {
  static const char hexch[] = "0123456789ABCDEF";
  for(int i = 0; i < len; i++) {
    int m = n & 0x000f;
    n >>= 4;
    s[pos + len - 1 - i] = hexch[m];
  }
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
