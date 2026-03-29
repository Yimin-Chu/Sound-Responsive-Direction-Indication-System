/**
 * @file    oled_ui.h
 * @brief   OLED display interface for dual-mic volume + direction
 */
#ifndef OLED_UI_H
#define OLED_UI_H

#include "../../Application/User/Core/Inc/mic_audio.h"

/**
 * @brief  Draw dual-channel volume bars + direction indicator on OLED.
 * @param  result  pointer to mic processing result
 */
void oled_draw_dual(const mic_result_t *result);

#endif /* OLED_UI_H */