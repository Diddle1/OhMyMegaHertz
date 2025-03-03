#pragma once


/////* LCD CONFIG */////
#define EXAMPLE_LCD_PIXEL_CLOCK_HZ   (6528000) //(10 * 1000 * 1000)
// The pixel number in horizontal and vertical
#define EXAMPLE_LCD_H_RES            320
#define EXAMPLE_LCD_V_RES            170
#define LVGL_LCD_BUF_SIZE            (EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES)
#define EXAMPLE_PSRAM_DATA_ALIGNMENT 64

////CC1101//////

int CC1101_SCK = 12;
int CC1101_MISO = 13;
int CC1101_MOSI = 11;
int CC1101_CS = 10;
int gdo0 = 44;
int gdo2 = 43;
// Radio config for ASK/OOK
int mod = 2;
float rxbw = 58;
float datarate = 5; 
float deviation = 0;
//Frequency
float currentFrequency;
const float freq_1 = 315;
const float freq_2 = 433.92;
const float freq_3 = 868.35;

////*ESP32S3*//////

#define PIN_LCD_BL                   38

#define PIN_LCD_D0                   39
#define PIN_LCD_D1                   40
#define PIN_LCD_D2                   41
#define PIN_LCD_D3                   42
#define PIN_LCD_D4                   45
#define PIN_LCD_D5                   46
#define PIN_LCD_D6                   47
#define PIN_LCD_D7                   48

#define PIN_POWER_ON                 15

#define PIN_LCD_RES                  5
#define PIN_LCD_CS                   6
#define PIN_LCD_DC                   7
#define PIN_LCD_WR                   8
#define PIN_LCD_RD                   9


#define BUTTON_PIN_1                 14
#define PIN_BAT_VOLT                 4

#define PIN_IIC_SCL                  17
#define PIN_IIC_SDA                  18

#define PIN_TOUCH_INT                16
#define PIN_TOUCH_RES                21