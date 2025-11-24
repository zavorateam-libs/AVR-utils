// TinyDbg.h -- single-file tiny utilities: async blink + minimal logger
/*
Usage:
   #include "TinyDbg.h"
   DBG_begin(115200);            // optional (calls Serial.begin anyway)
   B(3);                         // schedule 3 blinks on LED_BUILTIN (non-blocking)
   void loop() { DBG_tick(); ... }    // call DBG_tick() frequently
   log("hello");                 // prints: [LOG] hello
   err("wifi fail: ", errCode);  // prints: [ERR] wifi fail: 123
*/
// Idea: async blink via millis() (no delay). Inspired by GyverBlinker async approach.

#ifndef TINYDBG_H
#define TINYDBG_H

#include <Arduino.h>

// ----------------- Config -----------------
#ifndef TINYDBG_LED_PIN
  #define TINYDBG_LED_PIN LED_BUILTIN
#endif
// on/off durations (ms) for blink
#ifndef TINYDBG_ON_MS
  #define TINYDBG_ON_MS 200U
#endif
#ifndef TINYDBG_OFF_MS
  #define TINYDBG_OFF_MS 200U
#endif

// ----------------- Internal state (static, single-file) -----------------
static uint8_t _td_pin = TINYDBG_LED_PIN;
static unsigned long _td_last = 0;
static bool _td_led_on = false;
static int _td_rem_blinks = 0;      // remaining blinks
static bool _td_active = false;
static const __FlashStringHelper* _TAG_LOG = F("[LOG]");
static const __FlashStringHelper* _TAG_WARN = F("[WARN]");
static const __FlashStringHelper* _TAG_ERR = F("[ERR]");

// ----------------- API -----------------

// Initialize serial if you want (optional).
// Call before using log/warn/err if Serial isn't initialized elsewhere.
static inline void DBG_begin(unsigned long baud = 115200UL) {
  // Always safe to call; some platforms ignore duplicate begin.
  Serial.begin(baud);
  // tiny pause to let Serial init on some boards (keeps code small).
  unsigned long t = millis();
  while (millis() - t < 2) {} // 2ms tiny wait
}

// Schedule n blinks.
// Blink is performed on TINYDBG_LED_PIN. If n <= 0, does nothing.
// Example: B(3); // 3 quick blinks
static inline void B(int n) {
  if (n <= 0) return;
  _td_pin = (uint8_t)TINYDBG_LED_PIN;
  pinMode(_td_pin, OUTPUT);
  digitalWrite(_td_pin, LOW);
  _td_rem_blinks = n;
  _td_led_on = false;
  _td_last = millis();
  _td_active = true;
}

// Must be called from loop() frequently to update blink state.
static inline void DBG_tick(void) {
  if (!_td_active) return;
  unsigned long now = millis();
  if (_td_led_on) {
    // currently HIGH, wait ON_MS then switch off
    if (now - _td_last >= (unsigned long)TINYDBG_ON_MS) {
      digitalWrite(_td_pin, LOW);
      _td_led_on = false;
      _td_last = now;
      // finished one blink (on->off), decrement remaining
      _td_rem_blinks--;
      if (_td_rem_blinks <= 0) {
        _td_active = false;
        digitalWrite(_td_pin, LOW);
      }
    }
  } else {
    // currently LOW, wait OFF_MS then turn on (if blinks remain)
    if (_td_rem_blinks > 0 && now - _td_last >= (unsigned long)TINYDBG_OFF_MS) {
      digitalWrite(_td_pin, HIGH);
      _td_led_on = true;
      _td_last = now;
    }
  }
}

// ---------- minimal printing helpers (very small) ----------
// Print tag and one or two args. To keep code tiny we accept two common types:
//  - const char* (C-string)
//  - long (numbers)  -- prints via Serial.print
// You can pass a variable of another printable type; Serial.print will usually handle it.

static inline void _dbg_print_tag(const __FlashStringHelper* tag) {
  Serial.print(tag);
  Serial.print(' ');
}

// 1 arg: const char*
static inline void _dbg_print1(const char* a) {
  Serial.println(a);
}
// 1 arg: long
static inline void _dbg_print1(long a) {
  Serial.println(a);
}
// 2 args: const char* + long
static inline void _dbg_print2(const char* a, long b) {
  Serial.print(a);
  Serial.print(b);
  Serial.println();
}
// 2 args: long + const char*  (rare, but included)
static inline void _dbg_print2(long a, const char* b) {
  Serial.print(a);
  Serial.print(' ');
  Serial.println(b);
}
// 2 args: const char* + const char*
static inline void _dbg_print2(const char* a, const char* b) {
  Serial.print(a);
  Serial.print(b);
  Serial.println();
}

// Public logger overloads (small set to cover most cases).
// 1-arg variants:
static inline void log(const char* a)      { _dbg_print_tag(_TAG_LOG); _dbg_print1(a); }
static inline void warn(const char* a)     { _dbg_print_tag(_TAG_WARN); _dbg_print1(a); }
static inline void err(const char* a)      { _dbg_print_tag(_TAG_ERR); _dbg_print1(a); }

static inline void log(long a)             { _dbg_print_tag(_TAG_LOG); _dbg_print1(a); }
static inline void warn(long a)            { _dbg_print_tag(_TAG_WARN); _dbg_print1(a); }
static inline void err(long a)             { _dbg_print_tag(_TAG_ERR); _dbg_print1(a); }

// 2-arg variants: (text, number) or (text, text)
static inline void log(const char* a, long b)  { _dbg_print_tag(_TAG_LOG); _dbg_print2(a,b); }
static inline void warn(const char* a, long b) { _dbg_print_tag(_TAG_WARN); _dbg_print2(a,b); }
static inline void err(const char* a, long b)  { _dbg_print_tag(_TAG_ERR); _dbg_print2(a,b); }

static inline void log(const char* a, const char* b)  { _dbg_print_tag(_TAG_LOG); _dbg_print2(a,b); }
static inline void warn(const char* a, const char* b) { _dbg_print_tag(_TAG_WARN); _dbg_print2(a,b); }
static inline void err(const char* a, const char* b)  { _dbg_print_tag(_TAG_ERR); _dbg_print2(a,b); }

#endif // TINYDBG_H
