#ifndef JY61P_MIN_H
#define JY61P_MIN_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define JY61P_FRAME_LEN 11u

typedef struct {
    uint8_t buffer[JY61P_FRAME_LEN];
    uint8_t index;
    uint32_t byte_count;
    uint32_t angle_frame_count;
    uint32_t gyro_frame_count;
    uint32_t bad_checksum_count;
    int16_t roll_raw;
    int16_t pitch_raw;
    int16_t yaw_raw;
    int16_t roll_cdeg;
    int16_t pitch_cdeg;
    int16_t yaw_cdeg;
} jy61p_min_t;

void Jy61pMin_Init(jy61p_min_t *imu);
void Jy61pMin_FeedByte(jy61p_min_t *imu, uint8_t byte);

#ifdef __cplusplus
}
#endif

#endif
