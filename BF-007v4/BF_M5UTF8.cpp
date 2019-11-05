// Copyright 2019 BotanicFields, Inc.
// for M5Stack
// UTF8 to Japanese font 8/16x16
//
#include "BF_M5UTF8.h"

// font files in the microSD
const char* UTF8JIS = "/utf8/utfjis.dat";  // UTF16 to JIS conversion table
const char* UTF8F08 = "/utf8/8x16r.dat";   // Shinonome font  8x16
const char* UTF8F16 = "/utf8/k16.dat";     // Shinonome font 16x16
const char* UTF8IDX = "/utf8/k16.idx";     // index for font 16x16

uint16_t UTF8idx[UTF8F16_N];      // k16_font index
uint8_t  UTF8img[UTF8IMG_N][16];  // image buffer

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// open font files and read index
void UTF8_init() {

  // read index of k16_font
  Serial.println("[UTF8] read UTF8idx from microSD");
  File UTF8idx_f = SD.open(UTF8IDX, FILE_READ);
  for(int i = 0; i < UTF8F16_N; i++) {
    uint8_t r[2];
    UTF8idx_f.read(r, 2);
    UTF8idx[i] = ((uint16_t)r[0] << 8) | r[1];
  }
  UTF8idx_f.close();
}

// UTF-8 string --> JIS --> font --> image
// returns image-length in byte
int UTF8str2img(const char* s) {
  uint16_t jis;
  int i = 0;
  int j = 0;
  while(s[i] != '\0') {
    i += UTF8utf2jis(s[i], s[i + 1], s[i + 2], &jis);
    j += UTF8jis2fnt(jis, UTF8img[j], UTF8img[j + 1]);
  }
  return j;
}

// JIS code --> font
// returns font-width in byte
int UTF8jis2fnt(uint16_t jis, uint8_t* f0, uint8_t* f1) {
  if(jis <= 0x00df) {
    File UTF8f08_f = SD.open(UTF8F08, FILE_READ);
    UTF8f08_f.seek(UTF8f08_adr(jis));
    UTF8f08_f.read(f0, 16);
    UTF8f08_f.close();
    return 1;
  } else {
    uint8_t f[32];
    File UTF8f16_f = SD.open(UTF8F16, FILE_READ);
    UTF8f16_f.seek(UTF8f16_adr(jis));
    UTF8f16_f.read(f, 32);
    UTF8f16_f.close();
    for(int i = 0; i < 16; i++) {
      f0[i] = f[i * 2    ];
      f1[i] = f[i * 2 + 1];
    }
    return 2;
  }
}

// JIS code --> seek address 8x16
uint32_t UTF8f08_adr(uint16_t jis) {
  if(jis >= 0x00a1)
    jis = jis - 0x00a1 + 0x007f;  // 0x00a1 .. 0x00df
  return ((uint32_t)jis - 1) * 16;
}

// JIS code --> seek address 16x16
uint32_t UTF8f16_adr(uint16_t jis) {
  int p = 0;
  int p_min = 0;
  int p_max = UTF8F16_N - 1;
  while(UTF8idx[p] != jis) {
    if(p_min > p_max) return 3040;  // not found --> seq=96 0x2222 "□"
    p = p_min + (p_max - p_min) / 2;
    if(UTF8idx[p] > jis) p_max = p - 1;
    if(UTF8idx[p] < jis) p_min = p + 1;
  }
  return (uint32_t)p * 32;
}

// character-code conversion
// UTF-8 --> UTF-16 --> JIS code, returns bytes of UTF-8 
int UTF8utf2jis(uint32_t u0, uint32_t u1, uint32_t u2, uint16_t* jis) {
  if(u0 >= 0xe0) {
    uint32_t u = ((u0 & 0x0f) << 12) | ((u1 & 0x3f) << 6) | (u2 & 0x3f);
    if(u == 0xff5e) {
      *jis = 0x2141;  // 0xefbd9e(0xff5e) --> 0x2141: fullwidth tilde "～"
      return 3;         
    }
    *jis = UTF8jis(u);
    return 3;
  }
  if(u0 >= 0xc0) {
    *jis = UTF8jis(((u0 & 0x1f) << 6) | (u1 & 0x3f));
    return 2;
  }
  if(u0 >= 0x20) {
    *jis = UTF8jis(u0);
    return 1;
  } else {
    *jis = u0;
    return 1;
  }
}

// UTF-16 --> JIS code 
uint16_t UTF8jis(uint32_t u) {
  uint8_t r[2];
  File UTF8jis_f = SD.open(UTF8JIS, FILE_READ);
  UTF8jis_f.seek(u * 2);
  UTF8jis_f.read(r, 2);
  UTF8jis_f.close();
  uint16_t jis = ((uint16_t)r[0] << 8) | r[1];
  if(jis == 0) jis = 0x2120;  // not assigned
  return jis;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// test - dump UTF8idx[]
void UTF8test_idx() {
  Serial.print("\n[UTF8] ======== UTF8idx[i] ========");
  for(int i = 0; i < UTF8F16_N; i++) {
    if(i % 16 == 0) Serial.printf("\n%x:  ", i);
    Serial.printf("%x ", UTF8idx[i]);
  }
  Serial.println();
}

// test - UTF-8 to JIS conversion
void UTF8test_jis() {
  Serial.print("\n[UTF8] ======== UTF8 to JIS ========");
  uint16_t jis;
  for(uint8_t u0 = 0xe0; u0 <= 0xef; u0++) {
    for(uint8_t u1 = 0x80; u1 <= 0xbf; u1++) {
      for(uint8_t u2 = 0x80; u2 <= 0xbf; u2++) {
        if(u2 % 0x20 == 0)
          Serial.printf("\n%x %x %x: ", u0, u1, u2);
        UTF8utf2jis(u0, u1, u2, &jis);
        Serial.printf("%x ", jis);
      }
    }
  }
  Serial.println();
}

// test1: UTF8 --> font --> Serial of horizontal way
void UTF8test1() {
  Serial.println("\n[UTF8] ======== UTF-8 to font ========");
  char s[224];  // 0x01..0xdf + 1

  UTF8test1_sub_0x(s);

  UTF8test1_sub_cx(s, 0xc2);
//  UTF8test1_sub_cx(s, 0xc3);
//  UTF8test1_sub_cx(s, 0xce);
//  UTF8test1_sub_cx(s, 0xcf);
//  UTF8test1_sub_cx(s, 0xd0);
//  UTF8test1_sub_cx(s, 0xd1);

  UTF8test1_sub_ex(s, 0xe2, 0x80, 0x99);
//  UTF8test1_sub_ex(s, 0xe3, 0x80, 0x83);
//  UTF8test1_sub_ex(s, 0xe4, 0xb8, 0xbf);
//  UTF8test1_sub_ex(s, 0xe5, 0x80, 0xbf);
//  UTF8test1_sub_ex(s, 0xe6, 0x80, 0xbf);
//  UTF8test1_sub_ex(s, 0xe7, 0x80, 0xbf);
//  UTF8test1_sub_ex(s, 0xe8, 0x80, 0xbf);
//  UTF8test1_sub_ex(s, 0xe9, 0x80, 0xbe);
  UTF8test1_sub_ex(s, 0xef, 0xbc, 0xbf);
}
void UTF8test1_sub_0x(char* s) {
  int i = 0;
  for(int u0 = 0x01; u0 <= 0xdf; u0++)
    s[i++] = u0;
  s[i] = '\0'; 
  UTF8test1_sub(s);
}

void UTF8test1_sub_cx(char* s, int u0) {
  int i = 0;
  for(int u1 = 0x80; u1 <= 0xbf; u1++) {
    s[i++] = u0;
    s[i++] = u1;
  }
  s[i] = '\0'; 
  UTF8test1_sub(s);
}

void UTF8test1_sub_ex(char* s, int u0, int u1s, int u1e) {
  for(int u1 = u1s; u1 <= u1e; u1++) {
    int i = 0;
    for(int u2 = 0x80; u2 <= 0xbf; u2++) {
      s[i++] = u0;
      s[i++] = u1;
      s[i++] = u2;
    }
    s[i] = '\0'; 
    UTF8test1_sub(s);
  }
}

void UTF8test1_sub(const char* s) {
  int utf_length = strlen(s);
  int img_length = UTF8str2img(s);
  Serial.printf("utf_length = %d\n", utf_length);
  Serial.printf("img_length = %d\n", img_length);
  for(int i = 0; i < utf_length; i++)
    Serial.printf("%x ", s[i]);
  Serial.println();
  Serial.printf("%s\n", s);
  
  uint8_t l_img[8];
  for(int j = 0; j < 16; j++) {
    for(int i = 0; i < img_length; i++) {
      for(int k = 0; k < 8; k++)
        l_img[k] = ((UTF8img[i][j] & (0x80 >> k)) == 0) ? '.' : '@';  
      Serial.write(l_img, 8);
    }
    Serial.println();
  }
}  

// test2: UTF8 string --> font --> Serial of virtical way
void UTF8test2() {
  Serial.println("\n[UTF8] ======== UTF-8 string to font ========");
  const char s[] = "本日は晴天なり。テスト文字列";
  int utf_length = strlen(s);
  int img_length = UTF8str2img(s);
  Serial.printf("utf_length = %d\n", utf_length);
  Serial.printf("img_length = %d\n", img_length);
  for(int i = 0; i < utf_length; i++)
    Serial.printf("%x ", s[i]);
  Serial.println();
  Serial.printf("%s\n", s);

  for(int i = 0; i < img_length; i++) {
    for(int k = 0; k < 8; k++) {
      Serial.printf("%d ", i);
      for(int j = 15; j >= 0; j--) {
        char c = ((UTF8img[i][j] & (0x80 >> k)) == 0) ? '.' : '@';
        Serial.write(c);
        Serial.write(c);
      }
      Serial.println();
      delay(50);
    }
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
