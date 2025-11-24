#pragma once
#include <Arduino.h>
#include <stdint.h>

// ----- Типы -----
#define i8   int8_t
#define i16  int16_t
#define i32  int32_t
#define i64  int64_t
#define u8   uint8_t
#define u16  uint16_t
#define u32  uint32_t
#define u64  uint64_t

// ----- Псевдонимы -----
#define let   auto
#define NaN   NAN
#define Inf   INFINITY

// ----- Вывод -----
#define print(x) Serial.println(x)

/*
======================================================================
             MINIMAL AND FAST RandomNumberGenerator (LCG)
======================================================================
*/

static u32 _rnd_state = 1;

static inline void rnd_seed(u32 s) {
    _rnd_state = s ? s : 1;
}

static inline u32 rnd(void) {
    _rnd_state = _rnd_state * 1664525UL + 1013904223UL;
    return _rnd_state;
}
