// Copyright 2019 BotanicFields, Inc.
// BF-007
// M5Stack/ESP32 + HT16K33 + LED Matrix 16 x 64 (8 x 8 x 16)
//
#include <M5Stack.h>
#include <WiFi.h>
#include <WiFiManager.h>
#include "BF_Pcf8563.h"
#include "BF_RtcxNtp.h"
#include "BF_HT16K33_matrix.h"
#include "BF_M5UTF8.h"

const char s1[] = "人生で必要なものは無知と自信だけだ。これだけで成功は間違いない。(マーク・トウェイン)";
const char s2[] = "正しい友人というものは、あなたが間違っているときに味方してくれる者のこと。正しいときには誰だって味方をしてくれるのだから。(マーク・トウェイン)";
const char s3[] = "やったことは例え失敗しても20年後には笑い話にできる。しかし、やらなかったことは20年後には後悔するだけだ。(マーク・トウェイン)";
const char s4[] = "An old saying tells us that if you want something done well, you should do it yourself.";

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// for NTP
const char* time_zone  = "JST-9";
const char* ntp_server = "pool.ntp.org";
bool localtime_valid(false);
struct tm   local_tm;
bool rtcx_exist(false);

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// for WiFi
const int wifi_config_portal_timeout_sec(60);
const unsigned int wifi_retry_interval_ms(60000);
      unsigned int wifi_retry_last_ms(0);
const int wifi_retry_max_times(3);
      int wifi_retry_times(0);
wl_status_t wifi_status(WL_NO_SHIELD);

const char* wl_status_str[] = {
  "WL_IDLE_STATUS",      // 0
  "WL_NO_SSID_AVAIL",    // 1
  "WL_SCAN_COMPLETED",   // 2
  "WL_CONNECTED",        // 3
  "WL_CONNECT_FAILED",   // 4
  "WL_CONNECTION_LOST",  // 5
  "WL_DISCONNECTED",     // 6
  "WL_NO_SHIELD",        // 7 <-- 255
  "wl_status invalid",   // 8
};

const char* WlStatus(wl_status_t wl_status)
{
  if (wl_status >= 0 && wl_status <= 6) {
    return wl_status_str[wl_status];
  }
  if (wl_status == 255) {
    return wl_status_str[7];
  }
  return wl_status_str[8];
}

void WifiCheck()
{
  wl_status_t wifi_status_new = WiFi.status();
  if (wifi_status != wifi_status_new) {
    wifi_status = wifi_status_new;
    Serial.printf("[WiFi]%s\n", WlStatus(wifi_status));
  }

  // retry interval
  if (millis() - wifi_retry_last_ms < wifi_retry_interval_ms) {
    return;
  }
  wifi_retry_last_ms = millis();

  // reboot if wifi connection fails
  if (wifi_status == WL_CONNECT_FAILED) {
    Serial.print("[WiFi]connect failed: rebooting..\n");
    ESP.restart();
    return;
  }

  // let the wifi process do if wifi is not disconnected
  if (wifi_status != WL_DISCONNECTED) {
    wifi_retry_times = 0;
    return;
  }

  // reboot if wifi is disconnected for a long time
  if (++wifi_retry_times > wifi_retry_max_times) {
    Serial.print("[WiFi]disconnect timeout: rebooting..\n");
    ESP.restart();
    return;
  }

  // reconnect, and reboot if reconnection fails
  Serial.printf("[WiFi]reconnect %d\n", wifi_retry_times);
  if (!WiFi.reconnect()) {
    Serial.print("[WiFi]reconnect failed: rebooting..\n");
    ESP.restart();
    return;
  };
}

void WifiConfigModeCallback(WiFiManager *wm)
{
  M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  M5.Lcd.println("WiFi config portal:");
  M5.Lcd.println(wm->getConfigPortalSSID().c_str());
  M5.Lcd.println(WiFi.softAPIP().toString().c_str());
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
void setup()
{
  const bool lcd_enable(true);
  const bool sd_enable(true);
  const bool serial_enable(true);
  const bool i2c_enable(true);
  M5.begin(lcd_enable, sd_enable, serial_enable, i2c_enable);

  M5.Lcd.wakeup();
  M5.Lcd.setBrightness(100);
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setTextSize(2);
  M5.Lcd.setCursor(0, 0);

  // RTCx PCF8563: connect and start
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);  M5.Lcd.print("I2C, RTCx:");
  if (rtcx.Begin(Wire) == 0) {
    rtcx_exist = true;
    M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);  M5.Lcd.println("OK");
    if (SetTimeFromRtcx(time_zone)) {
      localtime_valid = true;
      M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);  M5.Lcd.println("RTCx valid");
    }
  }
  else {
    M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);  M5.Lcd.println("NG");
  }
  if (!localtime_valid) {
    Serial.print("RTC not valid: set the localtime temporarily\n");
    local_tm.tm_year = 117;  // 2017 > 2016, getLocalTime() returns true
    local_tm.tm_mon  = 0;    // January
    local_tm.tm_mday = 1;
    local_tm.tm_hour = 0;
    local_tm.tm_min  = 0;
    local_tm.tm_sec  = 0;
    struct timeval tv = { mktime(&local_tm), 0 };
    settimeofday(&tv, NULL);
  }
  getLocalTime(&local_tm);
  Serial.print(&local_tm, "localtime: %A, %B %d %Y %H:%M:%S\n");
  // print sample: must be < 64
  //....:....1....:....2....:....3....:....4....:....5....:....6....
  //localtime: Wednesday, September 11 2021 11:10:46

  // WiFi start
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);  M5.Lcd.println("WiFi connecting.. ");
  WiFiManager wm;  // blocking mode only

  // erase SSID/Key to force rewrite
  if (digitalRead(BUTTON_A_PIN) == LOW) {
    wm.resetSettings();
    M5.Lcd.setTextColor(TFT_YELLOW, TFT_BLACK);  M5.Lcd.println("SSID/Key erased");
    delay(3000);
  }

  // WiFi connect
  wm.setConfigPortalTimeout(wifi_config_portal_timeout_sec);
  wm.setAPCallback(WifiConfigModeCallback);
  if (wm.autoConnect()) {
    M5.Lcd.setTextColor(TFT_GREEN, TFT_BLACK);  M5.Lcd.println("OK");
  }
  else {
    M5.Lcd.setTextColor(TFT_RED, TFT_BLACK);  M5.Lcd.println("NG");
  }
  WiFi.setSleep(false);  // https://macsbug.wordpress.com/2021/05/02/buttona-on-m5stack-does-not-work-properly/
  wifi_retry_last_ms = millis() - wifi_retry_interval_ms;

  // NTP start
  NtpBegin(time_zone, ntp_server);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);  M5.Lcd.println("NTP start");
  delay(3000);

  // wait button-C to proceed into loop
  M5.Lcd.setTextColor(TFT_CYAN, TFT_BLACK);  M5.Lcd.println("Button-C to continue:");
  while (!M5.BtnC.wasReleased()) {
    M5.update();
    WifiCheck();
    if (RtcxUpdate()) {
      localtime_valid = true;  // SNTP sync completed
    }
  }

  // RTCx PCF8563: disable CLKO and INT
  const bool enable_clock_out(true);
  rtcx.ClockOutForTrimmer(!enable_clock_out);
  rtcx.DisableTimer();
  rtcx.DisableTimerInterrupt();

  // HT16K33 + LED Matrix
  HT16K33_init();
  UTF8_init();
}

void loop()
{
  M5.update();

  // WiFi recovery
  WifiCheck();

  // NTP sync
  if (RtcxUpdate()) {
    localtime_valid = true;  // SNTP sync completed
  }

  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextSize(1);
  M5.Lcd.setTextColor(TFT_WHITE, TFT_BLACK);

  M5.Lcd.println("all dot");     demoAllDot();   delay(3000);
//  M5.Lcd.println("test");        demoTest();     delay(1000);
  M5.Lcd.println("utf8-s1");     demoUtf8(50, s1);
//  M5.Lcd.println("random");      demoRandom();   delay(1000);
  M5.Lcd.println("clock");       demoClock();
  M5.Lcd.println("ball");        demoBall();     delay(1000);
  M5.Lcd.println("utf8-s2");     demoUtf8(50, s2);
//  M5.Lcd.println("line1");       demoLine1();    delay(1000);
  M5.Lcd.println("line2");       demoLine2();    delay(1000);
  M5.Lcd.println("clock");       demoClock();
//  M5.Lcd.println("circle1");     demoCircle1();  delay(1000);
  M5.Lcd.println("circle2");     demoCircle2();  delay(1000);
  M5.Lcd.println("circle3");     demoCircle3();  delay(1000);
  M5.Lcd.println("utf8-s3");     demoUtf8(50, s3);
//  M5.Lcd.println("char");        demoChar();     delay(1000);
//  M5.Lcd.println("timer");       demoTimer();    delay(1000);
  M5.Lcd.println("flow");        demoFlow();
  M5.Lcd.println("utf8-s4");     demoUtf8(50, s4);
//  M5.Lcd.println("utf8 idx");    UTF8test_idx();
//  M5.Lcd.println("utf8 jis");    UTF8test_jis();
//  M5.Lcd.println("utf8 test1");  UTF8test1();
//  M5.Lcd.println("utf8 test2");  UTF8test2();
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// light all dots
void demoAllDot()
{
  Serial.printf("[BF-007] demo all dots start\n");
  HT16K33_clear(0xffff);
  HT16K33_update();
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// basic fnctions
void demoTest()
{
  Serial.printf("[BF-007] demo function tests start\n");
  // lamp test
  for(int c = 0; c <= 2; ++c) {
    for(int y = 0; y < HT16K33_y; ++y) {
      for(int x = 0; x < HT16K33_x; ++x) {
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

  for(int h = 0; h < HT16K33_qty; ++h) {
    HT16K33_dimmer(h, h * 2 + 1);
  }
  delay(3000);

  Serial.printf("[BF-007] demo_test dimmer 2 start\n");
  for(int i = 0; i < 16; ++i) {
    for(int h = 0; h < HT16K33_qty; ++h)
      HT16K33_dimmer(h, i);
    delay(500);
  }

  // blink test
  Serial.printf("[BF-007] demo_test blink start\n");
  for(int b = 3; b >= 0; b--) {
    for(int h = 0; h < HT16K33_qty; ++h)
      HT16K33_blink(h, b);
    delay(3000 * b);
  }  
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// random dots
void demoRandom()
{
  Serial.printf("[BF-007] demo random start\n");
  for(int j = 0; j < 5; ++j) {
    HT16K33_clear();
    for(int i = 0; i < 100 * HT16K33_qty; ++i) 
      HT16K33_plot(random(HT16K33_x), random(HT16K33_y),1);
    HT16K33_update();
    delay(1000);
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// balls
void demoBall()
{
  Serial.printf("[BF-007] demo balls start\n");
  const int MAG = 10;                // magnificaton
  const int XMX = HT16K33_x * MAG;
  const int YMX = HT16K33_y * MAG;
  const int BMX = HT16K33_qty * 10;  // quantity of balls
  const int VMX =  8;                // max velocity
  struct ball_t {
    int x;   // position_x: 0..319 --> 0..31
    int y;   // posiiton_y: 0..159 --> 0..15
    int vx;  // velocity_x: 1..8
    int vy;  // velocity_y: 1..8
  };
  ball_t ball[BMX];  

  // iniitial position
  for(int i = 0; i < BMX; ++i) {
    ball[i].x  = random(XMX);
    ball[i].y  = random(YMX);
    ball[i].vx = random(1, 2 * VMX) - VMX;
    ball[i].vy = random(1, 2 * VMX) - VMX;
  }

  // move ball
  HT16K33_clear();
  for(int t = 0; t < 500; ++t) {
    for(int i = 0; i < BMX; ++i) {
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
void demoLine1()
{
  Serial.printf("[BF-007] demo lines 1 start\n");
  int xm = HT16K33_x - 1;
  int ym = HT16K33_y - 1;
  for(int i =  0; i <= xm; ++i) {
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
  for(int j =  0; j <= ym; ++j) { 
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
  for(int i = 0; i < HT16K33_x + HT16K33_y; ++i) {
    HT16K33_line(i,     0, 0,     i, 1);
    HT16K33_line(i - 1, 0, 0, i - 1, 0);
    HT16K33_update();
    delay(20);  
  }
}
  
void demoLine2()
{
  Serial.printf("[BF-007] demo lines 2 start\n");
  for(int i = 0; i < 110; ++i) {
    if(i < 100)
      HT16K33_clear();
    HT16K33_line(random(HT16K33_x), random(HT16K33_y), random(HT16K33_x), random(HT16K33_y), 1);
    HT16K33_update();
    delay(50);  
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// circles
void demoCircle1()
{
  Serial.printf("[BF-007] demo circle 1 start\n");
  for(int r = 0; r < 500; ++r) {
    HT16K33_clear();
    int y = 7;
    if(r >= HT16K33_x / 8) y = HT16K33_y - 1 - r;
    HT16K33_circle(0, y, r, 1);
    HT16K33_update();
    delay(10);
  }
}

void demoCircle2()
{
  Serial.printf("[BF-007] demo circle 2 start\n");
  for(int i = 0; i < 50; ++i) {
    HT16K33_clear();
    HT16K33_circle(random(HT16K33_x), random(HT16K33_y), random(8) + 4, 1);
    HT16K33_update();
    delay(100);
  }
}

void demoCircle3()
{
  Serial.printf("[BF-007] demo circle 3 start\n");
  for(int i = 0; i < 5; ++i) {
    int x = random(HT16K33_x);
    int y = random(HT16K33_y);
    HT16K33_clear();
    for(int r = random(5); r < HT16K33_x; r += 3) {
      HT16K33_circle(x, y, r, 1);
      HT16K33_update();
      delay(50);
    }
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// characters
void demoChar()
{
  Serial.printf("[BF-007] demo character start\n");
  HT16K33_clear(0);
  for(uint8_t ch = 0x40; ch <=0x60; ++ch) {
    HT16K33_char(random(HT16K33_x - 6), random(HT16K33_y - 8), 1, ch);
    HT16K33_update();
    delay(200);
  }
  HT16K33_clear(0xffff);
  for(uint8_t ch = 0x40; ch <=0x60; ++ch) {
    HT16K33_char(random(HT16K33_x - 6), random(HT16K33_y - 8), 0, ch);
    HT16K33_update();
    delay(200);
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// counter
void demoTimer()
{
  Serial.printf("[BF-007] demo timer start\n");
  char s[11];
  HT16K33_clear();
  for(int i = 0; i < 300; ++i) {
    for(int i = 0; i < 11; ++i) s[i]='\0';
    unsigned long u = millis();
    int2hexStr(u, 0,  8, s);  HT16K33_string(15, 0, 1, s);
    int2decStr(u, 0, 10, s);  HT16K33_string( 3, 8, 1, s);
    HT16K33_update();
    delay(10);
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// text flow
void demoFlow()
{
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
void demoClock()
{
  Serial.printf("[BF-007] demo clock start\n");
  struct tm timeinfo;
  static char s[10];

  HT16K33_clear();
  HT16K33_plot(36, 10, 1);  // ":"
  HT16K33_plot(36, 13, 1);
  HT16K33_plot(50, 10, 1);  // ":"
  HT16K33_plot(50, 13, 1);

  for(int i = 0; i < 50; ++i) {
    getLocalTime(&timeinfo); 
    strftime(s, 11, "%Y/%m/%d", &timeinfo);  HT16K33_string( 1, 0, 1, s);  // "yyyy/mm/dd"
    strftime(s, 4, "%a", &timeinfo);         HT16K33_string( 4, 8, 1, s);  // "DDD"
    strftime(s, 3, "%H", &timeinfo);         HT16K33_string(24, 8, 1, s);  // "hh"
    strftime(s, 3, "%M", &timeinfo);         HT16K33_string(38, 8, 1, s);  // "mm"
    strftime(s, 3, "%S", &timeinfo);         HT16K33_string(52, 8, 1, s);  // "ss"
    HT16K33_update();
    delay(100);
  }
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// utf8
void demoUtf8(int d, const char* s)
{
  Serial.printf("[BF-007] demo utf8 start\n");
  demo_flow_utf(d, s);
}

//..:....1....:....2....:....3....:....4....:....5....:....6....:....7..
// flow 16 dot string
extern HT16K33_t Ht16k33;
extern uint8_t UTF8img[UTF8IMG_N][16];

void demo_flow_utf(int delay_ms, const char* s)
{
  int utf_length = strlen(s);
  int img_length = UTF8str2img(s);
  unsigned long t; 

  Serial.printf("utf_length = %d\n", utf_length);
  Serial.printf("img_length = %d\n", img_length);
  Serial.printf("%s\n", s);
  Serial.printf("start = %d, end = %d\n", - HT16K33_x, img_length * 8);

  t = millis();
  for(int p_start = - HT16K33_x; p_start <= img_length * 8; ++p_start) {
    for(int i = 0; i < HT16K33_x; ++i) {
      int p = p_start + i;
      uint16_t image = 0;
      if(p >= 0 && p < img_length * 8)
        for(int j = 0; j < 16; ++j)
          if((UTF8img[p / 8][j] & (0x80 >> (p % 8))) != 0)
            image |= 0x0001 << j;
      Ht16k33.img[i] = image;
    }
    delay(delay_ms - (millis() - t));
    t = millis();
    HT16K33_update();
  }
}
