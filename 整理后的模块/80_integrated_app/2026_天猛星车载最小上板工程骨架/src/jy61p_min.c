#include "jy61p_min.h"

#include <stddef.h>
#include <string.h>

static int16_t read_i16_le(const uint8_t *p)
{
    return (int16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static int16_t raw_angle_to_cdeg(int16_t raw)
{
    return (int16_t)(((int32_t)raw * 18000L) / 32768L);
}

static uint8_t checksum_ok(const uint8_t *frame)
{
    uint8_t sum = 0u;

    for (uint8_t i = 0u; i < (JY61P_FRAME_LEN - 1u); ++i) {
        sum = (uint8_t)(sum + frame[i]);
    }

    return (sum == frame[JY61P_FRAME_LEN - 1u]) ? 1u : 0u;
}

static void parse_frame(jy61p_min_t *imu)
{
    const uint8_t *f = imu->buffer;

    if (checksum_ok(f) == 0u) {
        imu->bad_checksum_count++;
        return;
    }

    if (f[1] == 0x52u) {
        imu->gyro_frame_count++;
        return;
    }

    if (f[1] == 0x53u) {
        imu->roll_raw = read_i16_le(&f[2]);
        imu->pitch_raw = read_i16_le(&f[4]);
        imu->yaw_raw = read_i16_le(&f[6]);
        imu->roll_cdeg = raw_angle_to_cdeg(imu->roll_raw);
        imu->pitch_cdeg = raw_angle_to_cdeg(imu->pitch_raw);
        imu->yaw_cdeg = raw_angle_to_cdeg(imu->yaw_raw);
        imu->angle_frame_count++;
    }
}

void Jy61pMin_Init(jy61p_min_t *imu)
{
    if (imu == NULL) {
        return;
    }

    memset(imu, 0, sizeof(*imu));
}

void Jy61pMin_FeedByte(jy61p_min_t *imu, uint8_t byte)
{
    if (imu == NULL) {
        return;
    }

    imu->byte_count++;

    if (imu->index == 0u) {
        if (byte == 0x55u) {
            imu->buffer[0] = byte;
            imu->index = 1u;
        }
        return;
    }

    imu->buffer[imu->index] = byte;
    imu->index++;

    if ((imu->index == 2u) &&
        (byte != 0x51u) && (byte != 0x52u) && (byte != 0x53u)) {
        imu->index = (byte == 0x55u) ? 1u : 0u;
        imu->buffer[0] = byte;
        return;
    }

    if (imu->index >= JY61P_FRAME_LEN) {
        parse_frame(imu);
        imu->index = 0u;
    }
}
