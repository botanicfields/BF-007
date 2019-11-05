// Copyright 2019 BotanicFields, Inc.
// BF-007
// M5Stack/ESP32 + HT16K33 + LED Matrix 16 x 64 (8 x 8 x 16)
//
#include <M5Stack.h>
#include "BF_HT16K33_matrix.h"
#include "BF_M5UTF8.h"
#include "BF_ESP32_NTP.h"
#include "BF_M5Wifi_Connect.h"
#include "BF_M5LCD_Print.h"

const char s1[] = "人生で必要なものは無知と自信だけだ。これだけで成功は間違いない。(マーク・トウェイン)";
const char s2[] = "正しい友人というものは、あなたが間違っているときに味方してくれる者のこと。正しいときには誰だって味方をしてくれるのだから。(マーク・トウェイン)";
const char s3[] = "やったことは例え失敗しても20年後には笑い話にできる。しかし、やらなかったことは20年後には後悔するだけだ。(マーク・トウェイン)";
const char s4[] = "An old saying tells us that if you want something done well, you should do it yourself.";

// LCD control
const char* BF_007_MSG[] = {"   LED Matrix / HT16K33   ",  // 0
                            " lamp test   ",               // 1
                            " ball        ",               // 2
                            " line        ",               // 3
                            " circle      ",               // 4
                            " char        ",               // 5
                            " flow        ",               // 6
                            " clock       ",               // 7
                            " utf8        ",               // 8
                           };

void setup() {
  M5.begin();
  BF_WifiScan();
  BF_WifiInit();
  ntp_get();

  HT16K33_init();
  UTF8_init();

  BF_M5LCD_init(8, BF_007_MSG);

  xTaskCreatePinnedToCore(task_HT16K33_demo,   // Function to implement the task
                         "task_HT16K33_demo",  // Name of the task
                          4096,                // Stack size in words
                          NULL,                // Task input parameter
                          2,                   // Priority of the task
                          NULL,                // Task handle
                          1);                  // Core where the task should run
}

void loop() {
  M5.update();
  delay(100);
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7
// main task of HT16K33 demonstrations
void task_HT16K33_demo(void * pvParameters) {
  BF_M5LCD_update(0);  delay(1500);

  for(;;) {
    BF_M5LCD_update(1);  demoCurrent();  delay(3000);
//    BF_M5LCD_update(1);  demoTest();     delay(1000);
    BF_M5LCD_update(8);  demoUtf8(50, s1);

    BF_M5LCD_update(2);  demoRandom();   delay(1000);
    BF_M5LCD_update(7);  demoClock();

    BF_M5LCD_update(2);  demoBall();    delay(1000);
    BF_M5LCD_update(8);  demoUtf8(50, s2);

    BF_M5LCD_update(3);  demoLine1();   delay(1000);
    BF_M5LCD_update(3);  demoLine2();   delay(1000);
    BF_M5LCD_update(7);  demoClock();

//    BF_M5LCD_update(4);  demoCircle1(); delay(1000);
    BF_M5LCD_update(4);  demoCircle2(); delay(1000);
    BF_M5LCD_update(4);  demoCircle3(); delay(1000);
    BF_M5LCD_update(8);  demoUtf8(50, s3);

//    BF_M5LCD_update(5);  demoChar();    delay(1000);
    BF_M5LCD_update(5);  demoTimer();   delay(1000);
    BF_M5LCD_update(6);  demoFlow();
    BF_M5LCD_update(8);  demoUtf8(50, s4);

//    BF_M5LCD_update(8);  UTF8test_idx();
//    BF_M5LCD_update(8);  UTF8test_jis();
//    BF_M5LCD_update(8);  UTF8test1();
//    BF_M5LCD_update(8);  UTF8test2();
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// light all dots
void demoCurrent() {
  Serial.printf("[BF-007] demo current start\n");
  HT16K33_clear(0xffff);
  HT16K33_update();
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// basic fnctions
void demoTest() {
  Serial.printf("[BF-007] demo function tests start\n");
  // lamp test
  for(int c = 0; c <= 2; c++) {
    for(int y = 0; y < HT16K33_Y; y++) {
      for(int x = 0; x < HT16K33_X; x++) {
        HT16K33_plot(x, y, c);
      }
      HT16K33_update();
      delay(100);
    }
  }

  // dimmer test
  Serial.printf("[BF-007] demo_test dimmer 1 start\n");
  HT16K33_clear(0xffff);
  HT16K33_update();

  for(int h = 0; h < HT16K33_QTY; h++) {
    HT16K33_dimmer(h, h * 2 + 1);
  }
  delay(3000);

  Serial.printf("[BF-007] demo_test dimmer 2 start\n");
  for(int i = 0; i < 16; i++) {
    for(int h = 0; h < HT16K33_QTY; h++)
      HT16K33_dimmer(h, i);
    delay(500);
  }

  // blink test
  Serial.printf("[BF-007] demo_test blink start\n");
  for(int b = 3; b >= 0; b--) {
    for(int h = 0; h < HT16K33_QTY; h++)
      HT16K33_blink(h, b);
    delay(3000 * b);
  }  
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// random dots
void demoRandom() {
  Serial.printf("[BF-007] demo random start\n");
  for(int j = 0; j < 5; j++) {
    HT16K33_clear();
    for(int i = 0; i < 100 * HT16K33_QTY; i++) 
      HT16K33_plot(random(HT16K33_X), random(HT16K33_Y),1);
    HT16K33_update();
    delay(1000);
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// balls
void demoBall() {
  Serial.printf("[BF-007] demo balls start\n");
  const int MAG = 10;                // magnificaton
  const int XMX = HT16K33_X * MAG;
  const int YMX = HT16K33_Y * MAG;
  const int BMX = HT16K33_QTY * 10;  // quantity of balls
  const int VMX =  8;                // max velocity
  struct ball_t {
    int x;   // position_x: 0..319 --> 0..31
    int y;   // posiiton_y: 0..159 --> 0..15
    int vx;  // velocity_x: 1..8
    int vy;  // velocity_y: 1..8
  };
  ball_t ball[BMX];  

  // iniitial position
  for(int i = 0; i < BMX; i++) {
    ball[i].x  = random(XMX);
    ball[i].y  = random(YMX);
    ball[i].vx = random(1, 2 * VMX) - VMX;
    ball[i].vy = random(1, 2 * VMX) - VMX;
  }

  // move ball
  HT16K33_clear();
  for(int t = 0; t < 500; t++) {
    for(int i = 0; i < BMX; i++) {
      int old_x = ball[i].x / MAG;
      int old_y = ball[i].y / MAG;

      // new x
      ball[i].x += ball[i].vx;
      if(ball[i].x < 0)    { ball[i].x *= -1;                     ball[i].vx *= -1; }
      if(ball[i].x >= XMX) { ball[i].x = 2 * XMX - 1 - ball[i].x; ball[i].vx *= -1; }
      
      // new y
      ball[i].y += ball[i].vy;
      if(ball[i].y < 0)    { ball[i].y *= -1;                     ball[i].vy *= -1; }
      if(ball[i].y >= YMX) { ball[i].y = 2 * YMX - 1 - ball[i].y; ball[i].vy *= -1; }

      // update LED
      int new_x = ball[i].x / MAG;
      int new_y = ball[i].y / MAG;
      if((new_x != old_x) || (new_y != old_y)) {
        HT16K33_plot(old_x, old_y, 0);
        HT16K33_plot(new_x, new_y, 1);
      }
    }
    HT16K33_update();
    if(     t < 10) delay(500);
    else if(t < 30) delay(200);
    else if(t < 80) delay(100);
    else            delay( 10);
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// lines
void demoLine1() {
  Serial.printf("[BF-007] demo lines 1 start\n");
  int xm = HT16K33_X - 1;
  int ym = HT16K33_Y - 1;
  for(int i =  0; i <= xm; i++) {
    HT16K33_clear();  
    HT16K33_line(0,  0,  i, ym, 1);  
    HT16K33_update();  
    delay(20);
  }
  for(int j = ym; j >=  0; j--) {
    HT16K33_clear();  
    HT16K33_line(0,  0, xm,  j, 1); 
    HT16K33_update();  
    delay(20);
  }
  for(int j =  0; j <= ym; j++) { 
    HT16K33_clear();  
    HT16K33_line(0,  j, xm,  0, 1); 
    HT16K33_update();  
    delay(20);
  }
  for(int i = xm; i >=  0; i--) { 
    HT16K33_clear();  
    HT16K33_line(0, ym,  i,  0, 1); 
    HT16K33_update();  
    delay(20);
  }
  for(int i = 0; i < HT16K33_X + HT16K33_Y; i++) {
    HT16K33_line(i,     0, 0,     i, 1);
    HT16K33_line(i - 1, 0, 0, i - 1, 0);
    HT16K33_update();
    delay(20);  
  }
}
  
void demoLine2() {
  Serial.printf("[BF-007] demo lines 2 start\n");
  for(int i = 0; i < 110; i++) {
    if(i < 100)
      HT16K33_clear();
    HT16K33_line(random(HT16K33_X), random(HT16K33_Y), random(HT16K33_X), random(HT16K33_Y), 1);
    HT16K33_update();
    delay(50);  
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// circles
void demoCircle1() {
  Serial.printf("[BF-007] demo circle 1 start\n");
  for(int r = 0; r < 500; r++) {
    HT16K33_clear();
    int y = 7;
    if(r >= HT16K33_X / 8) y = HT16K33_Y - 1 - r;
    HT16K33_circle(0, y, r, 1);
    HT16K33_update();
    delay(10);
  }
}

void demoCircle2() {
  Serial.printf("[BF-007] demo circle 2 start\n");
  for(int i = 0; i < 50; i++) {
    HT16K33_clear();
    HT16K33_circle(random(HT16K33_X), random(HT16K33_Y), random(8) + 4, 1);
    HT16K33_update();
    delay(100);
  }
}

void demoCircle3() {
  Serial.printf("[BF-007] demo circle 3 start\n");
  for(int i = 0; i < 5; i++) {
    int x = random(HT16K33_X);
    int y = random(HT16K33_Y);
    HT16K33_clear();
    for(int r = random(5); r < HT16K33_X; r += 3) {
      HT16K33_circle(x, y, r, 1);
      HT16K33_update();
      delay(50);
    }
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// characters
void demoChar() {
  Serial.printf("[BF-007] demo character start\n");
  HT16K33_clear(0);
  for(uint8_t ch = 0x40; ch <=0x60; ch++) {
    HT16K33_char(random(HT16K33_X - 6), random(HT16K33_Y - 8), 1, ch);
    HT16K33_update();
    delay(200);
  }
  HT16K33_clear(0xffff);
  for(uint8_t ch = 0x40; ch <=0x60; ch++) {
    HT16K33_char(random(HT16K33_X - 6), random(HT16K33_Y - 8), 0, ch);
    HT16K33_update();
    delay(200);
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// counter
void demoTimer() {
  Serial.printf("[BF-007] demo timer start\n");
  char s[11];
  HT16K33_clear();
  for(int i = 0; i < 300; i++) {
    for(int i = 0; i < 11; i++) s[i]='\0';
    unsigned long u = millis();
    int2hexStr(u, 0,  8, s);  HT16K33_string(15, 0, 1, s);
    int2decStr(u, 0, 10, s);  HT16K33_string( 3, 8, 1, s);
    HT16K33_update();
    delay(10);
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// text flow
void demoFlow() {
  Serial.printf("[BF-007] demo flow start\n");
  static const char s1[] = "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_0123456789:;<=>?";
  static const char s2[] = "`abcdefghijklmnopqrstuvwxyz{|}~ !\"#$%&'()*+,-./|";
  static const char s3[] = "A quick fox jumped over the lazy brown dog.";
  static const char s4[] = "An old saying tells us that if you want something done well, you should do it yourself.";

  HT16K33_flow2(1, 1, 80, 80, s1, s2);
  HT16K33_flow2(1, 0, 60, 30, s3, s4);
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// clock
void demoClock() {
  Serial.printf("[BF-007] demo clock start\n");
  struct tm timeinfo;
  static char s[10];

  HT16K33_clear();
  HT16K33_plot(36, 10, 1);  // ":"
  HT16K33_plot(36, 13, 1);
  HT16K33_plot(50, 10, 1);  // ":"
  HT16K33_plot(50, 13, 1);

  for(int i = 0; i < 50; i++) {
    getLocalTime(&timeinfo); 
    strftime(s, 11, "%Y/%m/%d", &timeinfo);  HT16K33_string(1, 0, 1, s);  // "yyyy/mm/dd"
    strftime(s, 4, "%a", &timeinfo);  HT16K33_string( 4, 8, 1, s);  // "DDD"
    strftime(s, 3, "%H", &timeinfo);  HT16K33_string(24, 8, 1, s);  // "hh"
    strftime(s, 3, "%M", &timeinfo);  HT16K33_string(38, 8, 1, s);  // "mm"
    strftime(s, 3, "%S", &timeinfo);  HT16K33_string(52, 8, 1, s);  // "ss"
    HT16K33_update();
    delay(100);
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// utf8
void demoUtf8(int d, const char* s) {
  Serial.printf("[BF-007] demo utf8 start\n");
  demo_flow_utf(d, s);
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// flow 16 dot string
extern HT16K33_t Ht16k33;
extern uint8_t UTF8img[UTF8IMG_N][16];
void demo_flow_utf(int delay_ms, const char* s) {
  int utf_length = strlen(s);
  int img_length = UTF8str2img(s);
  unsigned long t; 

  Serial.printf("utf_length = %d\n", utf_length);
  Serial.printf("img_length = %d\n", img_length);
  Serial.printf("%s\n", s);
  Serial.printf("start = %d, end = %d\n", - HT16K33_X, img_length * 8);

  t = millis();
  for(int p_start = - HT16K33_X; p_start <= img_length * 8; p_start++) {
    for(int i = 0; i < HT16K33_X; i++) {
      int p = p_start + i;
      uint16_t image = 0;
      if(p >= 0 && p < img_length * 8)
        for(int j = 0; j < 16; j++)
          if((UTF8img[p / 8][j] & (0x80 >> (p % 8))) != 0)
            image |= 0x0001 << j;
      Ht16k33.img[i] = image;
    }
    delay(delay_ms - (millis() - t));
    t = millis();
    HT16K33_update();
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
