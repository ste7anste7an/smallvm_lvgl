#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <stddef.h>   // offsetof
#include <string.h>
#include <stdlib.h>

#ifndef CONFIG_FILE
#define CONFIG_FILE "/config.txt"
#endif

// ---- Your config structs (as provided) ----

typedef struct {
    char controller[16];
    int spi, mosi, miso, sck, cs;
    int dc, rst;
    int rotation;
    int color;
    bool invert;
    int backlight;
    int width, height;
    int col_offset, row_offset;
} LCDConfig;

typedef struct {
    int width, height;
} LVGLConfig;

typedef struct {
    char controller[16];
    char interface[8];    // "i2c" or "spi"
    int spi;
    int i2c;
    int irq;
    int miso, mosi, sck, cs;
    int rotation;
    int sda, scl;
} TouchConfig;

typedef struct {
    int advanced_serial_commands;
} OtherConfig;

typedef struct {
    LCDConfig lcd;
    LVGLConfig lvgl;
    TouchConfig touch;
    OtherConfig other;
} Config;

// ---- API ----
namespace configurator {
void trim(char* s);
bool loadConfig(Config* cfg);
void setWarnUnknownKeys(bool enable);
void setDebug(bool enable);
} // namespace cfg
