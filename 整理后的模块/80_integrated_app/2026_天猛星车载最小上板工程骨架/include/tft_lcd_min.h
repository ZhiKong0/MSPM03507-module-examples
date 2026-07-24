#ifndef TFT_LCD_MIN_H
#define TFT_LCD_MIN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define TFT_LCD_COLOR_BLACK 0x0000u
#define TFT_LCD_COLOR_BLUE 0x001Fu
#define TFT_LCD_COLOR_RED 0xF800u
#define TFT_LCD_COLOR_GREEN 0x07E0u
#define TFT_LCD_COLOR_WHITE 0xFFFFu
#define TFT_LCD_COLOR_YELLOW 0xFFE0u
#define TFT_LCD_COLOR_CYAN 0x07FFu
#define TFT_LCD_COLOR_MAGENTA 0xF81Fu
#define TFT_LCD_COLOR_GRAY 0x8410u
#define TFT_LCD_COLOR_ORANGE 0xFD20u

uint8_t TftLcd_Init(void);
void TftLcd_SetBacklight(uint8_t on);
void TftLcd_FillScreen(uint16_t rgb565);
void TftLcd_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t rgb565);
void TftLcd_DrawChar5x7(uint16_t x,
                        uint16_t y,
                        char ch,
                        uint16_t fg,
                        uint16_t bg,
                        uint8_t scale);
void TftLcd_DrawString(uint16_t x,
                       uint16_t y,
                       const char *text,
                       uint16_t fg,
                       uint16_t bg,
                       uint8_t scale);
void TftLcd_DrawBootPattern(void);

#ifdef __cplusplus
}
#endif

#endif
