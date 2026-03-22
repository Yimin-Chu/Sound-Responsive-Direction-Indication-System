/**
 * @file    mic_audio.c
 * @brief   Dual-microphone audio processing implementation
 */
#include "mic_audio.h"
#include <math.h>

/* ── Static working buffers (avoid malloc, FreeRTOS-safe as static) ── */
static uint16_t left_buf[MIC_BUF_SAMPLES];
static uint16_t right_buf[MIC_BUF_SAMPLES];

/* ── Internal helpers ─────────────────────────────────────────────── */

/**
 * @brief  Unpack interleaved dual-ADC DMA data into two separate arrays.
 *         STM32 dual mode packs: [31:16] = ADC2, [15:0] = ADC1
 */
static void unpack_dual(const uint32_t *dual_buf, uint32_t len,
                        uint16_t *left, uint16_t *right)
{
    for (uint32_t i = 0; i < len; i++)
    {
        left[i]  = (uint16_t)(dual_buf[i] & 0xFFFFU);         /* ADC1 = left  */
        right[i] = (uint16_t)((dual_buf[i] >> 16U) & 0xFFFFU);/* ADC2 = right */
    }
}

/* ── Public API ───────────────────────────────────────────────────── */

uint16_t mic_compute_rms(const uint16_t *buf, uint32_t len)
{
    /* Pass 1: DC offset (mean) */
    uint32_t sum = 0;
    for (uint32_t i = 0; i < len; i++)
        sum += buf[i];
    int32_t dc = (int32_t)(sum / len);

    /* Pass 2: sum of (sample - dc)^2 */
    uint32_t sq_sum = 0;
    for (uint32_t i = 0; i < len; i++)
    {
        int32_t ac = (int32_t)buf[i] - dc;
        sq_sum += (uint32_t)(ac * ac);
    }

    return (uint16_t)sqrtf((float)sq_sum / (float)len);
}

void mic_process(const uint32_t *dual_buf, uint32_t len, mic_result_t *out)
{
    /* Clamp to buffer size */
    if (len > MIC_BUF_SAMPLES)
        len = MIC_BUF_SAMPLES;

    /* Step 1: unpack */
    unpack_dual(dual_buf, len, left_buf, right_buf);

    /* Step 2: compute RMS for each channel */
    out->left_rms  = mic_compute_rms(left_buf,  len);
    out->right_rms = mic_compute_rms(right_buf, len);
    out->diff      = (int16_t)out->left_rms - (int16_t)out->right_rms;

    /* Step 3: direction decision */
    if (out->left_rms < MIC_NOISE_FLOOR && out->right_rms < MIC_NOISE_FLOOR)
    {
        out->direction = MIC_DIR_SILENT;
    }
    else if (out->diff > (int16_t)MIC_DIR_RATIO_THR)
    {
        out->direction = MIC_DIR_LEFT;
    }
    else if (out->diff < -(int16_t)MIC_DIR_RATIO_THR)
    {
        out->direction = MIC_DIR_RIGHT;
    }
    else
    {
        out->direction = MIC_DIR_CENTER;
    }
    /* Step 4: suppress noise floor for display */
    /*if (out->left_rms < MIC_NOISE_FLOOR)
        out->left_rms = 0;
    if (out->right_rms < MIC_NOISE_FLOOR)
        out->right_rms = 0;*/
}

const char *mic_dir_str(mic_direction_t d)
{
    switch (d)
    {
        case MIC_DIR_LEFT:   return "LEFT ";
        case MIC_DIR_RIGHT:  return "RIGHT";
        case MIC_DIR_CENTER: return "CNTR ";
        default:             return "QUIET";
    }
}