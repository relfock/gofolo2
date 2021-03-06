#include "sdk_common.h"
#include "nrf_lcd.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "frame.h"
#include "frames/f00.h"
#include "frames/f10.h"
#include "frames/f20.h"
#include "frames/f30.h"
#include "frames/f40.h"
#include "frames/f50.h"
#include "frames/f60.h"
#include "frames/f70.h"
#include "frames/f80.h"
#include "frames/f90.h"
#include "frames/f100.h"
#include "frames/f110.h"
#include "frames/f120.h"
#include "frames/f130.h"
#include "frames/f140.h"
#include "frames/f150.h"
#include "frames/f160.h"
#include "frames/f170.h"
#include "frames/f180.h"
#include "frames/f190.h"
#include "frames/f200.h"
#include "frames/f210.h"
#include "frames/f220.h"
#include "frames/f230.h"
#include "frames/f240.h"
#include "frames/f250.h"
#include "frames/f260.h"
#include "frames/f270.h"
#include "frames/f280.h"
#include "frames/f290.h"
#include "frames/f300.h"
#include "frames/f310.h"
#include "frames/f320.h"
#include "frames/f330.h"
#include "frames/f340.h"
#include "frames/f350.h"

#include "nrf_delay.h"

#define DISP_EN_PIN 20
#define LCD_SPI_SS_PIN 28
#define LCD_SPI_SCK_PIN 4
#define LCD_SPI_MOSI_PIN 5

typedef uint8_t matrix_ptr_t[16];
matrix_ptr_t *m = (matrix_ptr_t *)frame;

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(1);

static void sharp_pixel_draw(uint16_t x, uint16_t y, uint32_t color)
{
    if(color)
        frame[x / 8 + 16 * y] = frame[x / 8 + 16 * y] | (1 << (7 - x % 8));
    else
        frame[x / 8 + 16 * y] = frame[x / 8 + 16 * y] & ~(1 << (7 - x % 8));
}

static void sharp_rect_draw(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color)
{
    uint16_t i = 0, j = 0;

    for(i = x; i < width; i++) {
        for(j = y; j < height; j++) {
          sharp_pixel_draw(i, j, color);
        }
    }
    
}

static void sharp_rotation_set(nrf_lcd_rotation_t rotation)
{
}

static void sharp_display_invert(bool invert)
{
}

static lcd_cb_t sharp_cb = {
    .height = 128,
    .width = 128
};

/* LCD resolution */
#define LCD_XRES 128
#define LCD_YRES 128
#define LCD_BYTES_LINE LCD_XRES / 8
#define LCD_BUF_SIZE LCD_YRES * LCD_BYTES_LINE

uint8_t swap(uint8_t b) 
{
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;

    return b;
}

void toggle_vcom()
{
    int i = 0;
    uint8_t vcom[2];

    vcom[0] = 0x00;
    vcom[1] = 0x00;

    while(i == 2) {
        vcom[0] = ((1 & (i / 2)) << 1);
        nrf_gpio_pin_set(LCD_SPI_SS_PIN); 
        nrf_delay_ms(5000);
        APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, vcom, 2, NULL, 0));
        nrf_gpio_pin_clear(LCD_SPI_SS_PIN); 
        i++;
    }
}

static void sharp_display(void)
{
    int i, j;
    uint8_t tx_data[LCD_BYTES_LINE + 4];

    memset(tx_data, 0, sizeof(tx_data));
    for(i = 1; i <= LCD_YRES; ++i) {
        tx_data[0] = 1 | ((1 & (i / 2)) << 1);
        tx_data[1] = i;
        memcpy(tx_data + 2, frame + (i - 1) * LCD_BYTES_LINE, LCD_BYTES_LINE);

        for(j = 2; j < sizeof(tx_data) - 2; ++j)
            tx_data[j] = swap(tx_data[j]);

        nrf_gpio_pin_set(LCD_SPI_SS_PIN); 
        nrf_drv_spi_transfer(&spi, tx_data, LCD_BYTES_LINE + 4, NULL, 0);
        nrf_gpio_pin_clear(LCD_SPI_SS_PIN); 
    }

    //toggle_vcom();
}

void clear_lcd()
{
    uint8_t data[2];

    data[0] = 4;
    data[1] = 0;
    data[1] = data[0];

    nrf_gpio_pin_set(LCD_SPI_SS_PIN);
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, data, 2, NULL, 0));
    nrf_gpio_pin_clear(LCD_SPI_SS_PIN); 
}

ret_code_t sharp_init(void)
{
    ret_code_t err_code;

    nrf_gpio_cfg_output(DISP_EN_PIN);
    nrf_gpio_pin_set(DISP_EN_PIN); 

    nrf_gpio_pin_set(LCD_SPI_SS_PIN); 
    nrf_gpio_cfg_output(LCD_SPI_SS_PIN);

    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.frequency = NRF_SPIM_FREQ_1M;
    spi_config.mosi_pin = LCD_SPI_MOSI_PIN;
    spi_config.sck_pin = LCD_SPI_SCK_PIN;

    spi_config.mode = NRF_DRV_SPI_MODE_0;
    spi_config.bit_order = NRF_DRV_SPI_BIT_ORDER_LSB_FIRST;

    err_code = nrf_drv_spi_init(&spi, &spi_config, NULL, NULL);

    clear_lcd();
    nrf_delay_ms(1000);
    sharp_display();

    return err_code;
}

static void sharp_uninit(void)
{
    nrf_drv_spi_uninit(&spi);
}

const nrf_lcd_t nrf_lcd_sharp = {
    .lcd_init = sharp_init,
    .lcd_uninit = sharp_uninit,
    .lcd_pixel_draw = sharp_pixel_draw,
    .lcd_rect_draw = sharp_rect_draw,
    .lcd_display = sharp_display,
    .lcd_rotation_set = sharp_rotation_set,
    .lcd_display_invert = sharp_display_invert,
    .p_lcd_cb = &sharp_cb
};

char select_frame ()
{
    static int f = 0;

    f++;

    switch(f % 36) {
        case 0:
            memcpy(frame, f0, sizeof(frame));
            return 0;
            break;
        case 1:
            memcpy(frame, f1, sizeof(frame));
            return 1;
            break;
        case 2:
            memcpy(frame, f2, sizeof(frame));
            return 2;
            break;
        case 3:
            memcpy(frame, f3, sizeof(frame));
            return 3;
            break;
        case 4:
            memcpy(frame, f4, sizeof(frame));
            return 4;
            break;
        case 5:
            memcpy(frame, f5, sizeof(frame));
            return 5;
            break;
        case 6:
            memcpy(frame, f6, sizeof(frame));
            return 6;
            break;
        case 7:
            memcpy(frame, f7, sizeof(frame));
            return 7;
            break;
        case 8:
            memcpy(frame, f8, sizeof(frame));
            return 8;
            break;
        case 9:
            memcpy(frame, f9, sizeof(frame));
            return 9;
            break;
        case 10:
            memcpy(frame, f10, sizeof(frame));
            return 10;
            break;
        case 11:
            memcpy(frame, f11, sizeof(frame));
            return 11;
            break;
        case 12:
            memcpy(frame, f12, sizeof(frame));
            return 12;
            break;
        case 13:
            memcpy(frame, f13, sizeof(frame));
            return 13;
            break;
        case 14:
            memcpy(frame, f14, sizeof(frame));
            return 14;
            break;
        case 15:
            memcpy(frame, f15, sizeof(frame));
            return 15;
            break;
        case 16:
            memcpy(frame, f16, sizeof(frame));
            return 16;
            break;
        case 17:
            memcpy(frame, f17, sizeof(frame));
            return 17;
            break;
        case 18:
            memcpy(frame, f18, sizeof(frame));
            return 18;
            break;
        case 19:
            memcpy(frame, f19, sizeof(frame));
            return 19;
            break;
        case 20:
            memcpy(frame, f20, sizeof(frame));
            return 20;
            break;
        case 21:
            memcpy(frame, f21, sizeof(frame));
            return 21;
            break;
        case 22:
            memcpy(frame, f22, sizeof(frame));
            return 22;
            break;
        case 23:
            memcpy(frame, f23, sizeof(frame));
            return 23;
            break;
        case 24:
            memcpy(frame, f24, sizeof(frame));
            return 24;
            break;
        case 25:
            memcpy(frame, f25, sizeof(frame));
            return 25;
            break;
        case 26:
            memcpy(frame, f26, sizeof(frame));
            return 26;
            break;
        case 27:
            memcpy(frame, f27, sizeof(frame));
            return 27;
            break;
        case 28:
            memcpy(frame, f28, sizeof(frame));
            return 28;
            break;
        case 29:
            memcpy(frame, f29, sizeof(frame));
            return 29;
            break;
        case 30:
            memcpy(frame, f30, sizeof(frame));
            return 30;
            break;
        case 31:
            memcpy(frame, f31, sizeof(frame));
            return 31;
            break;
        case 32:
            memcpy(frame, f32, sizeof(frame));
            return 32;
            break;
        case 33:
            memcpy(frame, f33, sizeof(frame));
            return 33;
            break;
        case 34:
            memcpy(frame, f34, sizeof(frame));
            return 34;
            break;
        case 35:
            memcpy(frame, f35, sizeof(frame));
            return 35;
            break;
        default:
            return 0;
            break;
    }

    return 0;
}
