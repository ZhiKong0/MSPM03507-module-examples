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
