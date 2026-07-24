#include "tft_lcd_min.h"

#include "ti_msp_dl_config.h"

#ifndef TFT_LCD_SPI_HALF_PERIOD_CYCLES
#define TFT_LCD_SPI_HALF_PERIOD_CYCLES (2u)
#endif

#ifndef TFT_LCD_BLK_ACTIVE_LEVEL
#define TFT_LCD_BLK_ACTIVE_LEVEL (1u)
#endif

#define TFT_CMD_SWRESET 0x01u
#define TFT_CMD_SLPOUT 0x11u
#define TFT_CMD_NORON 0x13u
#define TFT_CMD_INVOFF 0x20u
#define TFT_CMD_MADCTL 0x36u
#define TFT_CMD_COLMOD 0x3Au
#define TFT_CMD_CASET 0x2Au
#define TFT_CMD_RASET 0x2Bu
#define TFT_CMD_RAMWR 0x2Cu
#define TFT_CMD_DISPON 0x29u
#define FONT5X7_WIDTH 5u
#define FONT5X7_HEIGHT 7u
#define FONT5X7_CELL_WIDTH 6u
#define FONT5X7_CELL_HEIGHT 8u

static const uint8_t g_font_digit[10][5] = {
    { 0x3Eu, 0x51u, 0x49u, 0x45u, 0x3Eu },
    { 0x00u, 0x42u, 0x7Fu, 0x40u, 0x00u },
    { 0x42u, 0x61u, 0x51u, 0x49u, 0x46u },
    { 0x21u, 0x41u, 0x45u, 0x4Bu, 0x31u },
    { 0x18u, 0x14u, 0x12u, 0x7Fu, 0x10u },
    { 0x27u, 0x45u, 0x45u, 0x45u, 0x39u },
    { 0x3Cu, 0x4Au, 0x49u, 0x49u, 0x30u },
    { 0x01u, 0x71u, 0x09u, 0x05u, 0x03u },
    { 0x36u, 0x49u, 0x49u, 0x49u, 0x36u },
    { 0x06u, 0x49u, 0x49u, 0x29u, 0x1Eu },
};

static const uint8_t g_font_upper[26][5] = {
    { 0x7Eu, 0x11u, 0x11u, 0x11u, 0x7Eu },
    { 0x7Fu, 0x49u, 0x49u, 0x49u, 0x36u },
    { 0x3Eu, 0x41u, 0x41u, 0x41u, 0x22u },
    { 0x7Fu, 0x41u, 0x41u, 0x22u, 0x1Cu },
    { 0x7Fu, 0x49u, 0x49u, 0x49u, 0x41u },
    { 0x7Fu, 0x09u, 0x09u, 0x09u, 0x01u },
    { 0x3Eu, 0x41u, 0x49u, 0x49u, 0x7Au },
    { 0x7Fu, 0x08u, 0x08u, 0x08u, 0x7Fu },
    { 0x00u, 0x41u, 0x7Fu, 0x41u, 0x00u },
    { 0x20u, 0x40u, 0x41u, 0x3Fu, 0x01u },
    { 0x7Fu, 0x08u, 0x14u, 0x22u, 0x41u },
    { 0x7Fu, 0x40u, 0x40u, 0x40u, 0x40u },
    { 0x7Fu, 0x02u, 0x0Cu, 0x02u, 0x7Fu },
    { 0x7Fu, 0x04u, 0x08u, 0x10u, 0x7Fu },
    { 0x3Eu, 0x41u, 0x41u, 0x41u, 0x3Eu },
    { 0x7Fu, 0x09u, 0x09u, 0x09u, 0x06u },
    { 0x3Eu, 0x41u, 0x51u, 0x21u, 0x5Eu },
    { 0x7Fu, 0x09u, 0x19u, 0x29u, 0x46u },
    { 0x46u, 0x49u, 0x49u, 0x49u, 0x31u },
    { 0x01u, 0x01u, 0x7Fu, 0x01u, 0x01u },
    { 0x3Fu, 0x40u, 0x40u, 0x40u, 0x3Fu },
    { 0x1Fu, 0x20u, 0x40u, 0x20u, 0x1Fu },
    { 0x3Fu, 0x40u, 0x38u, 0x40u, 0x3Fu },
    { 0x63u, 0x14u, 0x08u, 0x14u, 0x63u },
    { 0x07u, 0x08u, 0x70u, 0x08u, 0x07u },
    { 0x61u, 0x51u, 0x49u, 0x45u, 0x43u },
};

static const uint8_t g_glyph_space[5] = { 0u, 0u, 0u, 0u, 0u };
static const uint8_t g_glyph_dash[5] = { 0x08u, 0x08u, 0x08u, 0x08u, 0x08u };
static const uint8_t g_glyph_colon[5] = { 0x00u, 0x36u, 0x36u, 0x00u, 0x00u };
static const uint8_t g_glyph_slash[5] = { 0x20u, 0x10u, 0x08u, 0x04u, 0x02u };
static const uint8_t g_glyph_gt[5] = { 0x41u, 0x22u, 0x14u, 0x08u, 0x00u };
static const uint8_t g_glyph_lt[5] = { 0x08u, 0x14u, 0x22u, 0x41u, 0x00u };
static const uint8_t g_glyph_dot[5] = { 0x00u, 0x60u, 0x60u, 0x00u, 0x00u };
static const uint8_t g_glyph_plus[5] = { 0x08u, 0x08u, 0x3Eu, 0x08u, 0x08u };
static const uint8_t g_glyph_star[5] = { 0x14u, 0x08u, 0x3Eu, 0x08u, 0x14u };

static void gpio_write(GPIO_Regs *port, uint32_t pin, uint8_t high)
{
    if (high != 0u) {
        DL_GPIO_setPins(port, pin);
    } else {
        DL_GPIO_clearPins(port, pin);
    }
}

static void lcd_delay_ms(uint16_t ms)
{
    delay_cycles((uint32_t)ms * (CPUCLK_FREQ / 1000u));
}

static void spi_delay(void)
{
    delay_cycles(TFT_LCD_SPI_HALF_PERIOD_CYCLES);
}

static void lcd_select(uint8_t selected)
{
    gpio_write(TFT_LCD_CS_PORT, TFT_LCD_CS_PIN, (selected == 0u) ? 1u : 0u);
}

static void lcd_write_byte(uint8_t value)
{
    for (uint8_t mask = 0x80u; mask != 0u; mask >>= 1u) {
        gpio_write(TFT_LCD_SCL_PORT, TFT_LCD_SCL_PIN, 0u);
        gpio_write(TFT_LCD_SDA_PORT, TFT_LCD_SDA_PIN, (value & mask) ? 1u : 0u);
        spi_delay();
        gpio_write(TFT_LCD_SCL_PORT, TFT_LCD_SCL_PIN, 1u);
        spi_delay();
    }
    gpio_write(TFT_LCD_SCL_PORT, TFT_LCD_SCL_PIN, 0u);
}

static void lcd_write_command(uint8_t command)
{
    gpio_write(TFT_LCD_DC_PORT, TFT_LCD_DC_PIN, 0u);
    lcd_select(1u);
    lcd_write_byte(command);
    lcd_select(0u);
}

static void lcd_write_data8(uint8_t data)
{
    gpio_write(TFT_LCD_DC_PORT, TFT_LCD_DC_PIN, 1u);
    lcd_select(1u);
    lcd_write_byte(data);
    lcd_select(0u);
}

static void lcd_write_data16_stream(uint16_t data)
{
    lcd_write_byte((uint8_t)(data >> 8));
    lcd_write_byte((uint8_t)data);
}

static void lcd_reset_panel(void)
{
    gpio_write(TFT_LCD_RES_PORT, TFT_LCD_RES_PIN, 1u);
    lcd_delay_ms(10u);
    gpio_write(TFT_LCD_RES_PORT, TFT_LCD_RES_PIN, 0u);
    lcd_delay_ms(20u);
    gpio_write(TFT_LCD_RES_PORT, TFT_LCD_RES_PIN, 1u);
    lcd_delay_ms(120u);
}

static void lcd_set_window(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    uint16_t x0 = (uint16_t)(x + TFT_LCD_X_OFFSET);
    uint16_t y0 = (uint16_t)(y + TFT_LCD_Y_OFFSET);
    uint16_t x1 = (uint16_t)(x0 + w - 1u);
    uint16_t y1 = (uint16_t)(y0 + h - 1u);

    lcd_write_command(TFT_CMD_CASET);
    lcd_write_data8((uint8_t)(x0 >> 8));
    lcd_write_data8((uint8_t)x0);
    lcd_write_data8((uint8_t)(x1 >> 8));
    lcd_write_data8((uint8_t)x1);

    lcd_write_command(TFT_CMD_RASET);
    lcd_write_data8((uint8_t)(y0 >> 8));
    lcd_write_data8((uint8_t)y0);
    lcd_write_data8((uint8_t)(y1 >> 8));
    lcd_write_data8((uint8_t)y1);

    lcd_write_command(TFT_CMD_RAMWR);
}

static const uint8_t *font5x7_glyph(char ch)
{
    if ((ch >= 'a') && (ch <= 'z')) {
        ch = (char)(ch - ('a' - 'A'));
    }
    if ((ch >= '0') && (ch <= '9')) {
        return g_font_digit[(uint8_t)(ch - '0')];
    }
    if ((ch >= 'A') && (ch <= 'Z')) {
        return g_font_upper[(uint8_t)(ch - 'A')];
    }

    switch (ch) {
    case '-':
        return g_glyph_dash;
    case ':':
        return g_glyph_colon;
    case '/':
        return g_glyph_slash;
    case '>':
        return g_glyph_gt;
    case '<':
        return g_glyph_lt;
    case '.':
        return g_glyph_dot;
    case '+':
        return g_glyph_plus;
    case '*':
        return g_glyph_star;
    default:
        return g_glyph_space;
    }
}

void TftLcd_SetBacklight(uint8_t on)
{
    uint8_t active = (TFT_LCD_BLK_ACTIVE_LEVEL != 0u) ? 1u : 0u;
    gpio_write(TFT_LCD_BLK_PORT, TFT_LCD_BLK_PIN, (on != 0u) ? active : (uint8_t)!active);
}

uint8_t TftLcd_Init(void)
{
    TftLcd_SetBacklight(0u);
    lcd_select(0u);
    gpio_write(TFT_LCD_DC_PORT, TFT_LCD_DC_PIN, 1u);
    gpio_write(TFT_LCD_SCL_PORT, TFT_LCD_SCL_PIN, 0u);
    gpio_write(TFT_LCD_SDA_PORT, TFT_LCD_SDA_PIN, 0u);

    lcd_reset_panel();

    lcd_write_command(TFT_CMD_SWRESET);
    lcd_delay_ms(150u);
    lcd_write_command(TFT_CMD_SLPOUT);
    lcd_delay_ms(120u);

    lcd_write_command(TFT_CMD_COLMOD);
    lcd_write_data8(0x05u);

    lcd_write_command(TFT_CMD_MADCTL);
    lcd_write_data8(0x00u);

    lcd_write_command(TFT_CMD_INVOFF);
    lcd_write_command(TFT_CMD_NORON);
    lcd_delay_ms(10u);
    lcd_write_command(TFT_CMD_DISPON);
    lcd_delay_ms(120u);

    TftLcd_SetBacklight(1u);
    TftLcd_FillScreen(TFT_LCD_COLOR_BLACK);
    return 1u;
}

void TftLcd_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t rgb565)
{
    uint32_t pixels;

    if ((w == 0u) || (h == 0u) ||
        (x >= TFT_LCD_WIDTH) || (y >= TFT_LCD_HEIGHT)) {
        return;
    }
    if ((uint32_t)x + w > TFT_LCD_WIDTH) {
        w = (uint16_t)(TFT_LCD_WIDTH - x);
    }
    if ((uint32_t)y + h > TFT_LCD_HEIGHT) {
        h = (uint16_t)(TFT_LCD_HEIGHT - y);
    }

    lcd_set_window(x, y, w, h);
    pixels = (uint32_t)w * h;

    gpio_write(TFT_LCD_DC_PORT, TFT_LCD_DC_PIN, 1u);
    lcd_select(1u);
    for (uint32_t i = 0u; i < pixels; ++i) {
        lcd_write_data16_stream(rgb565);
    }
    lcd_select(0u);
}

void TftLcd_FillScreen(uint16_t rgb565)
{
    TftLcd_FillRect(0u, 0u, TFT_LCD_WIDTH, TFT_LCD_HEIGHT, rgb565);
}

void TftLcd_DrawChar5x7(uint16_t x,
                        uint16_t y,
                        char ch,
                        uint16_t fg,
                        uint16_t bg,
                        uint8_t scale)
{
    const uint8_t *glyph;
    uint16_t pixel_scale = (scale == 0u) ? 1u : scale;

    if ((x >= TFT_LCD_WIDTH) || (y >= TFT_LCD_HEIGHT)) {
        return;
    }

    TftLcd_FillRect(x, y,
                    (uint16_t)(FONT5X7_CELL_WIDTH * pixel_scale),
                    (uint16_t)(FONT5X7_CELL_HEIGHT * pixel_scale),
                    bg);

    glyph = font5x7_glyph(ch);
    for (uint8_t col = 0u; col < FONT5X7_WIDTH; ++col) {
        uint8_t bits = glyph[col];
        for (uint8_t row = 0u; row < FONT5X7_HEIGHT; ++row) {
            if (((bits >> row) & 1u) != 0u) {
                TftLcd_FillRect((uint16_t)(x + (uint16_t)col * pixel_scale),
                                (uint16_t)(y + (uint16_t)row * pixel_scale),
                                pixel_scale,
                                pixel_scale,
                                fg);
            }
        }
    }
}

void TftLcd_DrawString(uint16_t x,
                       uint16_t y,
                       const char *text,
                       uint16_t fg,
                       uint16_t bg,
                       uint8_t scale)
{
    uint16_t cursor = x;
    uint16_t pixel_scale = (scale == 0u) ? 1u : scale;
    uint16_t step = (uint16_t)(FONT5X7_CELL_WIDTH * pixel_scale);

    if (text == 0) {
        return;
    }

    while ((*text != '\0') && (cursor < TFT_LCD_WIDTH)) {
        if ((uint32_t)cursor + step > TFT_LCD_WIDTH) {
            break;
        }
        TftLcd_DrawChar5x7(cursor, y, *text, fg, bg, scale);
        cursor = (uint16_t)(cursor + step);
        ++text;
    }
}

void TftLcd_DrawBootPattern(void)
{
    static const uint16_t colors[] = {
        TFT_LCD_COLOR_RED,
        TFT_LCD_COLOR_GREEN,
        TFT_LCD_COLOR_BLUE,
        TFT_LCD_COLOR_YELLOW,
        TFT_LCD_COLOR_CYAN,
        TFT_LCD_COLOR_MAGENTA,
        TFT_LCD_COLOR_WHITE,
        TFT_LCD_COLOR_BLACK,
    };
    uint16_t band_w = (uint16_t)(TFT_LCD_WIDTH / (uint16_t)(sizeof(colors) / sizeof(colors[0])));
    uint16_t x = 0u;

    for (uint8_t i = 0u; i < (uint8_t)(sizeof(colors) / sizeof(colors[0])); ++i) {
        uint16_t w = (i == ((uint8_t)(sizeof(colors) / sizeof(colors[0])) - 1u)) ?
                     (uint16_t)(TFT_LCD_WIDTH - x) : band_w;
        TftLcd_FillRect(x, 0u, w, TFT_LCD_HEIGHT, colors[i]);
        x = (uint16_t)(x + w);
    }
}
