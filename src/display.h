#pragma once

#ifdef __cplusplus
 extern "C" {
#endif

#include <hardware/platform_defs.h>
#include <stdbool.h>

#define NICK "wenzellabs.de"

#define DISPLAY_I2C                 i2c0

#define SLIM_DISPLAY_HEIGHT         32
#define LARGE_DISPLAY_HEIGHT        64
extern int SSD1306_HEIGHT;//        = SLIM_DISPLAY_HEIGHT;
#define SSD1306_WIDTH               128

#define DISPLAY_GEOM_JUMPER_1       14
#define DISPLAY_GEOM_JUMPER_2       15


#define FONT_WIDTH                  6
#define FONT_HEIGHT                 8

#define SSD1306_I2C_ADDR            _u(0x3C)

#define SSD1306_SET_MEM_MODE        _u(0x20)
#define SSD1306_SET_COL_ADDR        _u(0x21)
#define SSD1306_SET_PAGE_ADDR       _u(0x22)
#define SSD1306_SET_HORIZ_SCROLL    _u(0x26)
#define SSD1306_SET_SCROLL          _u(0x2E)

#define SSD1306_SET_DISP_START_LINE _u(0x40)

#define SSD1306_SET_CONTRAST        _u(0x81)
#define SSD1306_SET_CHARGE_PUMP     _u(0x8D)

#define SSD1306_SET_SEG_REMAP       _u(0xA0)
#define SSD1306_SET_ENTIRE_ON       _u(0xA4)
#define SSD1306_SET_ALL_ON          _u(0xA5)
#define SSD1306_SET_NORM_DISP       _u(0xA6)
#define SSD1306_SET_INV_DISP        _u(0xA7)
#define SSD1306_SET_MUX_RATIO       _u(0xA8)
#define SSD1306_SET_DISP            _u(0xAE)
#define SSD1306_SET_COM_OUT_DIR     _u(0xC0)
#define SSD1306_SET_COM_OUT_DIR_FLIP _u(0xC0)

#define SSD1306_SET_DISP_OFFSET     _u(0xD3)
#define SSD1306_SET_DISP_CLK_DIV    _u(0xD5)
#define SSD1306_SET_PRECHARGE       _u(0xD9)
#define SSD1306_SET_COM_PIN_CFG     _u(0xDA)
#define SSD1306_SET_VCOM_DESEL      _u(0xDB)

#define SSD1306_PAGE_HEIGHT         _u(8)
extern int SSD1306_NUM_PAGES;//            = (SSD1306_HEIGHT / SSD1306_PAGE_HEIGHT);
extern int SSD1306_BUF_LEN;  //            = (SSD1306_NUM_PAGES * SSD1306_WIDTH);

#define SSD1306_WRITE_MODE         _u(0xFE)
#define SSD1306_READ_MODE          _u(0xFF)

typedef struct render_area_s {
    uint8_t start_col;
    uint8_t end_col;
    uint8_t start_page;
    uint8_t end_page;

    uint32_t buflen;
} render_area_t;

extern render_area_t full_frame_area;

void set_pixel(int x,int y, bool on);

void draw_line(int x0, int y0, int x1, int y1, bool on);
void draw_line_vertical(int x, int y0, int y1, bool on);
void draw_line_horizontal(int x0, int y, int x1, bool on);
void draw_rect(int x0, int y0, int x1, int y1, bool on);

void scroll_down(int n_pixels);
bool is_large_display(void);

void write_string(int16_t x, int16_t y, const char *str);
void render(render_area_t *area);
void render_full(void);
void init_display(void);

extern uint8_t *display_buffer;

#ifdef __cplusplus
 }
#endif
