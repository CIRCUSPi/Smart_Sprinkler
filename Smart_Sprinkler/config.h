#include <Arduino.h>
#ifndef _CONFIG_H_
#define _CONFIG_H_

#define SHOW_LOG true
#define DEBUG false
#define DEBUG_TIMER false

#define UART_BAUDRATE 115200

#if DEBUG_TIMER
#define DEBUG_PRINT_TIMER(x) Serial.print(x)
#define DEBUG_PRINTLN_TIMER(x) Serial.println(x)
#else
#define DEBUG_PRINT_TIMER(x)
#define DEBUG_PRINTLN_TIMER(x)
#endif

#if DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

#if SHOW_LOG
#define LOG_PRINT(x) Serial.print(x)
#define LOG_PRINTLN(x) Serial.println(x)
#else
#define LOG_PRINT(x)
#define LOG_PRINTLN(x)
#endif

#define DEVICE_NAME "Smart_Sprinkler"
#define WIFI_AP_SSID DEVICE_NAME "_AP"
#define WIFI_AP_PASS "12345678"

#define EEPROM_ADDR 0x10
#define CHECK_DATA 0x12345678

#define FUZZY_P 10     // %
// Pin define
#define PUMP_PIN 32
#define HUMI_SNESOR_PIN 33
#define BTN_PIN 39

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

typedef enum
{
    AUTO,
    MANUAL,
    TIMING,
} MODE_E;

#endif
