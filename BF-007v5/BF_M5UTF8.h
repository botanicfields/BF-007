// Copyright 2019 BotanicFields, Inc.
// for M5Stack
// UTF-8 to Japanese font 8/16x16
//
#pragma once
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
