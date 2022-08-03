#include <Arduino.h>
#ifndef _CONFIG_H_
#define _CONFIG_H_

#define DEBUG true
#define DEBUG_TIMER false

#define UART_BAUDRATE 115200

#if DEBUG_TIMER
#define DEBUG_PRIMT_TIMER(x) Serial.print(x)
#define DEBUG_PRIMTLN_TIMER(x) Serial.println(x)
#else
#define DEBUG_PRIMT_TIMER(x)
#define DEBUG_PRIMTLN_TIMER(x)
#endif

#if DEBUG
#define DEBUG_PRIMT(x) Serial.print(x)
#define DEBUG_PRIMTLN(x) Serial.println(x)
#else
#define DEBUG_PRIMT(x)
#define DEBUG_PRIMTLN(x)
#endif

#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASS "your_wifi_pass"

#define EEPROM_ADDR 0x10
#define CHECK_DATA 0x12345678

#define FUZZY_P 10     // %
// Pin define
#define PUMP_PIN 32
#define HUMI_SNESOR_PIN 33

#define MIN(a, b) ((a) > (b) ? (b) : (a))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef struct {
    uint32_t check;
    int      lowp;
    int      interval;
    int      waterSec;
    int      maxLimit;
    int      minLimit;
    uint8_t  mode;
} config_t;

#endif
