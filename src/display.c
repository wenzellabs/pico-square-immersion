#include <hardware/i2c.h>
#include <pico/stdlib.h>
#include <font6x8.h>
#include <display.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#define font font6x8_ascii

int SSD1306_HEIGHT = SLIM_DISPLAY_HEIGHT;
int SSD1306_NUM_PAGES;
int SSD1306_BUF_LEN;

uint8_t *display_buffer;

void calc_render_area_buflen(render_area_t *area)
{
    area->end_page = SSD1306_NUM_PAGES - 1;
    // calculate how long the flattened buffer will be for a render area
    area->buflen = (area->end_col - area->start_col + 1) * (area->end_page - area->start_page + 1);
}

void SSD1306_send_cmd(uint8_t cmd)
{
    // I2C write process expects a control byte followed by data
    // this "data" can be a command or data to follow up a command
    // Co = 1, D/C = 0 => the driver expects a command
    uint8_t buf[2] = {0x80, cmd};
    i2c_write_blocking(DISPLAY_I2C, SSD1306_I2C_ADDR, buf, 2, false);
}

void SSD1306_send_cmd_list(uint8_t *buf, int num)
{
    for (int i=0;i<num;i++)
        SSD1306_send_cmd(buf[i]);
}

void SSD1306_send_buf(uint8_t buf[], int buflen)
{
    // in horizontal addressing mode, the column address pointer auto-increments
    // and then wraps around to the next page, so we can send the entire frame
    // buffer in one go

    // copy our frame buffer into a new buffer because we need to add the control byte
    // to the beginning

    uint8_t *temp_buf = malloc(buflen + 1); // FIXME

    temp_buf[0] = 0x40;
    memcpy(temp_buf+1, buf, buflen);

    i2c_write_blocking(DISPLAY_I2C, SSD1306_I2C_ADDR, temp_buf, buflen + 1, false);

    free(temp_buf);
}

void render(render_area_t *area)
{
    // update a portion of the display with a render area
    uint8_t cmds[] = {
        SSD1306_SET_COL_ADDR,
        area->start_col,
        area->end_col,
        SSD1306_SET_PAGE_ADDR,
        area->start_page,
        area->end_page
    };
    
    SSD1306_send_cmd_list(cmds, count_of(cmds));
    SSD1306_send_buf(display_buffer, area->buflen);
}

static void write_char(int16_t x, int16_t y, uint8_t ch)
{
    // Cull out any character off the screen
    if ((x > SSD1306_WIDTH - FONT_WIDTH) || (y > SSD1306_HEIGHT - FONT_HEIGHT))
        return;

    // For the moment, only write on Y row boundaries (every 8 vertical pixels)
    y = y/8;

    int idx = ch - ' ';
    int fb_idx = y * 128 + x;

    display_buffer[0] = 0x00;
    for (int i=0;i<5;i++) {
        display_buffer[fb_idx++] = font[idx * (FONT_WIDTH-1) + i];
    }
}

void write_string(int16_t x, int16_t y, const char *str)
{
    while (*str) {
        write_char(x, y, *str++);
        x += FONT_WIDTH;
    }
}

void set_pixel(int x,int y, bool on)
{
    // The calculation to determine the correct bit to set depends on which address
    // mode we are in. This code assumes horizontal

    // The video ram on the SSD1306 is split up in to 8 rows, one bit per pixel.
    // Each row is 128 long by 8 pixels high, each byte vertically arranged, so byte 0 is x=0, y=0->7,
    // byte 1 is x = 1, y=0->7 etc

    // This code could be optimised, but is like this for clarity. The compiler
    // should do a half decent job optimising it anyway.

    const int BytesPerRow = SSD1306_WIDTH ; // x pixels, 1bpp, but each row is 8 pixel high, so (x / 8) * 8

    int byte_idx = (y / 8) * BytesPerRow + x;
    uint8_t byte = display_buffer[byte_idx];

    if (on)
        byte |=  1 << (y % 8);
    else
        byte &= ~(1 << (y % 8));

    display_buffer[byte_idx] = byte;
}

void scroll_down(int n_pixels)
{
    // Calculate number of rows (each row represents 8 vertical pixels)
    int row_height = SSD1306_HEIGHT / 8;
    int n_rows = n_pixels / 8;

    if (n_pixels <= 0) return; // No scrolling for non-positive input

    // Shift the entire buffer down by the number of full rows
    if (n_rows > 0) {
        memmove(display_buffer + (n_rows * SSD1306_WIDTH), display_buffer, (row_height - n_rows) * SSD1306_WIDTH);
        memset(display_buffer, 0, n_rows * SSD1306_WIDTH); // Clear the top rows that have been vacated
    }

    // Handle pixel scrolling if n_pixels is not a multiple of 8
    int extra_pixels = n_pixels % 8;
    if (extra_pixels > 0) {
        for (int x = 0; x < SSD1306_WIDTH; x++) {
            for (int row = row_height - 1; row >= 0; row--) {
                uint8_t current_byte = display_buffer[row * SSD1306_WIDTH + x];
                uint8_t next_byte = (row > 0) ? display_buffer[(row - 1) * SSD1306_WIDTH + x] : 0;

                // Shift the current byte down and fill with bits from the previous row
                display_buffer[row * SSD1306_WIDTH + x] = (current_byte >> extra_pixels) | (next_byte << (8 - extra_pixels));
            }
        }
    }
}

void draw_line(int x0, int y0, int x1, int y1, bool on)
{

    int dx =  abs(x1-x0);
    int sx = x0<x1 ? 1 : -1;
    int dy = -abs(y1-y0);
    int sy = y0<y1 ? 1 : -1;
    int err = dx+dy;
    int e2;

    while (true) {
        set_pixel(x0, y0, on);
        if (x0 == x1 && y0 == y1)
            break;
        e2 = 2*err;

        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void draw_line_vertical(int x, int y0, int y1, bool on)
{
    for (int i = y0; i <= y1; i++)
    {
        set_pixel(x, i, on);
    }
}

void draw_line_horizontal(int x0, int y, int x1, bool on)
{
    for (int i = x0; i <= x1; i++)
    {
        set_pixel(i, y, on);
    }
}

void draw_rect(int x0, int y0, int x1, int y1, bool on)
{
    for (int i = y0; i <= y1; i++)
    {
        draw_line_horizontal(x0, i, x1, on);
    }
}

void SSD1306_init()
{
    // Some of these commands are not strictly necessary as the reset
    // process defaults to some of these but they are shown here
    // to demonstrate what the initialization sequence looks like
    // Some configuration values are recommended by the board manufacturer

    uint8_t cmds[] = {
        SSD1306_SET_DISP,               // set display off
        /* memory mapping */
        SSD1306_SET_MEM_MODE,           // set memory address mode 0 = horizontal, 1 = vertical, 2 = page
        0x00,                           // horizontal addressing mode
        /* resolution and layout */
        SSD1306_SET_DISP_START_LINE,    // set display start line to 0
        SSD1306_SET_SEG_REMAP | 0x01,   // set segment re-map, column address 127 is mapped to SEG0
        SSD1306_SET_MUX_RATIO,          // set multiplex ratio
        SSD1306_HEIGHT - 1,             // Display height - 1
        SSD1306_SET_COM_OUT_DIR | 0x08, // set COM (common) output scan direction. Scan from bottom up, COM[N-1] to COM0
        SSD1306_SET_DISP_OFFSET,        // set display offset
        0x00,                           // no offset
        SSD1306_SET_COM_PIN_CFG,        // set COM (common) pins hardware configuration. Board specific magic number. 
                                        // 0x02 Works for 128x32, 0x12 Possibly works for 128x64. Other options 0x22, 0x32
        (SSD1306_HEIGHT == 64) ? 0x12 : 0x02,
        /* timing and driving scheme */
        SSD1306_SET_DISP_CLK_DIV,       // set display clock divide ratio
        0x80,                           // div ratio of 1, standard freq
        SSD1306_SET_PRECHARGE,          // set pre-charge period
        0xF1,                           // Vcc internally generated on our board
        SSD1306_SET_VCOM_DESEL,         // set VCOMH deselect level
        0x30,                           // 0.83xVcc
        /* display */
        SSD1306_SET_CONTRAST,           // set contrast control
        0x1F, // FIXME was 0xff
        SSD1306_SET_ENTIRE_ON,          // set entire display on to follow RAM content
        SSD1306_SET_NORM_DISP,           // set normal (not inverted) display
        SSD1306_SET_CHARGE_PUMP,        // set charge pump
        0x14,                           // Vcc internally generated on our board
        SSD1306_SET_SCROLL | 0x00,      // deactivate horizontal scrolling if set. This is necessary as memory writes will corrupt if scrolling was enabled
        SSD1306_SET_DISP | 0x01, // turn display on
    };

    SSD1306_send_cmd_list(cmds, count_of(cmds));
}

render_area_t full_frame_area = {
    start_col: 0,
    end_col : SSD1306_WIDTH - 1,
    start_page : 0,
};

void wipe_display(void)
{
    memset(display_buffer, 0, SSD1306_BUF_LEN);
    render(&full_frame_area);
}

void render_full(void)
{
    static int64_t last_render_us;
    int64_t now_us = get_absolute_time();
    if (now_us - last_render_us < 100*1000) return; // no more than 10 fps
    last_render_us = now_us;
    render(&full_frame_area);
}

void detect_display_type(void)
{
    gpio_init(DISPLAY_GEOM_JUMPER_1);
    gpio_init(DISPLAY_GEOM_JUMPER_2);

    gpio_set_dir(DISPLAY_GEOM_JUMPER_1, GPIO_OUT);
    gpio_put(DISPLAY_GEOM_JUMPER_1, 1);

    gpio_set_dir(DISPLAY_GEOM_JUMPER_2, GPIO_IN);
    gpio_pull_down(DISPLAY_GEOM_JUMPER_2);

    if (gpio_get(DISPLAY_GEOM_JUMPER_2))
    {
        SSD1306_HEIGHT = LARGE_DISPLAY_HEIGHT;

    }
    SSD1306_NUM_PAGES   = (SSD1306_HEIGHT / SSD1306_PAGE_HEIGHT);
    SSD1306_BUF_LEN     = (SSD1306_NUM_PAGES * SSD1306_WIDTH);
}

bool is_large_display(void)
{
    return (SSD1306_HEIGHT == LARGE_DISPLAY_HEIGHT);
}

void init_display(void)
{
    detect_display_type();
    display_buffer = malloc(SSD1306_BUF_LEN);
    i2c_init(DISPLAY_I2C, 1000 * 1000); // 1MHz
    gpio_set_function(I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_PIN);
    gpio_pull_up(I2C_SCL_PIN);
    SSD1306_init();

    calc_render_area_buflen(&full_frame_area);

    wipe_display();
}
