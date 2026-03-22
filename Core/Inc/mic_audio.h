/**
 * @file    mic_audio.h
 * @brief   Dual-microphone audio processing module
 *          Computes per-channel RMS volume and sound direction.
 *          Designed for FreeRTOS-ready modular architecture.
 */
#ifndef MIC_AUDIO_H
#define MIC_AUDIO_H

#include <stdint.h>

/* ── Configuration ──────────────────────────────────────────────── */
#define MIC_BUF_SAMPLES   1024U   /* samples per channel per frame  */

/* Direction detection thresholds (tune on real hardware) */
#define MIC_NOISE_FLOOR     50U   /* RMS below this = silence       */
#define MIC_DIR_RATIO_THR   30U   /* |L-R| > threshold → directional*/

/* ── Types ──────────────────────────────────────────────────────── */
typedef enum {
    MIC_DIR_SILENT = 0,
    MIC_DIR_LEFT,
    MIC_DIR_RIGHT,
    MIC_DIR_CENTER
} mic_direction_t;

typedef struct {
    uint16_t left_rms;            /* RMS volume of left  channel    */
    uint16_t right_rms;           /* RMS volume of right channel    */
    int16_t  diff;                /* left_rms - right_rms (signed)  */
    mic_direction_t direction;    /*判断结果                         */
} mic_result_t;

/* ── API ────────────────────────────────────────────────────────── */

/**
 * @brief  Unpack dual-ADC DMA buffer and compute volumes + direction.
 *
 * @param  dual_buf   pointer to uint32_t DMA buffer
 *                    low 16 bits  = ADC1 (left)
 *                    high 16 bits = ADC2 (right)
 * @param  len        number of uint32_t entries (= samples per channel)
 * @param  out        pointer to result struct (filled on return)
 */
void mic_process(const uint32_t *dual_buf, uint32_t len, mic_result_t *out);

/**
 * @brief  Compute RMS volume from a single-channel 12-bit sample array.
 * @param  buf  sample buffer
 * @param  len  number of samples
 * @return RMS value (0 – ~2048)
 */
uint16_t mic_compute_rms(const uint16_t *buf, uint32_t len);

/**
 * @brief  Return a short string for the direction enum.
 */
const char *mic_dir_str(mic_direction_t d);

#endif /* MIC_AUDIO_H */