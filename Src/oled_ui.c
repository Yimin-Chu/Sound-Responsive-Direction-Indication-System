/**
 * @file    oled_ui.c
 * @brief   OLED display for dual-mic volume + direction
 */
#include "oled_ui.h"
#include "ssd1306.h"
#include <stdio.h>

/* max RMS for bar scaling (tune to your gain) */
#define BAR_MAX_VOL   600U
#define BAR_X_START   4U
#define BAR_X_END     123U
#define BAR_WIDTH     (BAR_X_END - BAR_X_START)

static uint8_t vol_to_pixels(uint16_t vol)
{
    if (vol > BAR_MAX_VOL) vol = BAR_MAX_VOL;
    return (uint8_t)((uint32_t)vol * BAR_WIDTH / BAR_MAX_VOL);
}

static void draw_bar(uint8_t y_start, uint8_t y_end, uint8_t filled_w)
{
    for (uint16_t x = BAR_X_START; x < BAR_X_END; x++)
    {
        SSD1306_COLOR_t c = ((x - BAR_X_START) < filled_w)
                            ? SSD1306_COLOR_WHITE : SSD1306_COLOR_BLACK;
        for (uint16_t y = y_start; y < y_end; y++)
            SSD1306_SetPixel(x, y, c);
    }
}

void oled_draw_dual(const mic_result_t *result)
{
    char line[22];

    SSD1306_Fill(SSD1306_COLOR_BLACK);

    /* Row 0: direction indicator */
    sprintf(line, "DIR: %s", mic_dir_str(result->direction));
    SSD1306_Puts(line, &Font_11x18);

    /* Row 1: left channel bar  (y 20..31) */
    sprintf(line, "\nL:%3u", result->left_rms);
    SSD1306_Puts(line, &Font_11x18);
    draw_bar(38, 44, vol_to_pixels(result->left_rms));

    /* Row 2: right channel bar (y 46..57) */
    sprintf(line, "\n\nR:%3u", result->right_rms);
    SSD1306_Puts(line, &Font_11x18);
    draw_bar(56, 62, vol_to_pixels(result->right_rms));

    SSD1306_UpdateScreen();
}