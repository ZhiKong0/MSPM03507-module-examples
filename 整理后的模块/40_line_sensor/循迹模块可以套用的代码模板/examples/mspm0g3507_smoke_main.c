#include "line_trace_template.h"

#include <stddef.h>
#include <stdint.h>

extern uint32_t _estack;
extern uint32_t _sidata;
extern uint32_t _sdata;
extern uint32_t _edata;
extern uint32_t _sbss;
extern uint32_t _ebss;

extern void App_LineTraceInit(void);
extern void App_LineTraceSetSafetyState(line_trace_safety_state_t state);
extern void App_LineTraceTask20ms(void);

volatile uint32_t g_line_smoke_counter;
volatile uint32_t g_line_smoke_cookie = 0x4c54524bu;
volatile uint8_t g_line_smoke_raw_bits = 0xe7u;
volatile int16_t g_line_smoke_last_left;
volatile int16_t g_line_smoke_last_right;

void *memset(void *dest, int value, size_t count)
{
    uint8_t *d = (uint8_t *)dest;

    while (count != 0u) {
        *d++ = (uint8_t)value;
        --count;
    }
    return dest;
}

void *memcpy(void *dest, const void *src, size_t count)
{
    uint8_t *d = (uint8_t *)dest;
    const uint8_t *s = (const uint8_t *)src;

    while (count != 0u) {
        *d++ = *s++;
        --count;
    }
    return dest;
}

void __aeabi_memcpy(void *dest, const void *src, size_t count)
{
    (void)memcpy(dest, src, count);
}

void __aeabi_memcpy4(void *dest, const void *src, size_t count)
{
    (void)memcpy(dest, src, count);
}

void __aeabi_memset(void *dest, size_t count, int value)
{
    (void)memset(dest, value, count);
}

void __aeabi_memclr(void *dest, size_t count)
{
    (void)memset(dest, 0, count);
}

uint8_t Board_LineReadLevel(uint8_t channel, void *user)
{
    (void)user;

    if (channel >= LINE_TRACE_MAX_CHANNELS) {
        return 1u;
    }
    return (uint8_t)((g_line_smoke_raw_bits >> channel) & 1u);
}

uint8_t Board_LineBuildSampleFrame(const line_trace_sensor_params_t *sensor,
                                   line_trace_sample_frame_t *out)
{
    return LineTrace_BuildSampleFrame(sensor, g_line_smoke_raw_bits, 0u,
                                      LINE_TRACE_SENSOR_OK, out);
}

uint8_t Board_LineReadSampleFrame(line_trace_sample_frame_t *out)
{
    const line_trace_sensor_params_t sensor = {
        .channel_count = 8u,
        .active_level = LINE_TRACE_ACTIVE_LOW,
        .threshold_mode = LINE_TRACE_THRESHOLD_DIGITAL_GPIO,
        .i2c_addr = 0u,
        .sensor_enable_mask = 0xffu,
    };

    return Board_LineBuildSampleFrame(&sensor, out);
}

void Motor_SetTargetSpeed(int16_t left, int16_t right)
{
    g_line_smoke_last_left = left;
    g_line_smoke_last_right = right;
}

static void smoke_update_pattern(void)
{
    switch (g_line_smoke_counter & 3u) {
    case 0u:
        g_line_smoke_raw_bits = 0xe7u;
        break;
    case 1u:
        g_line_smoke_raw_bits = 0xcfu;
        break;
    case 2u:
        g_line_smoke_raw_bits = 0xf3u;
        break;
    default:
        g_line_smoke_raw_bits = 0xffu;
        break;
    }
}

int main(void)
{
    App_LineTraceInit();
    App_LineTraceSetSafetyState(LINE_TRACE_SAFETY_RUNNING_TUNE_SAFE);

    for (;;) {
        smoke_update_pattern();
        App_LineTraceTask20ms();
        ++g_line_smoke_counter;
        g_line_smoke_cookie ^= (0x01010101u + g_line_smoke_counter);
    }
}

void Reset_Handler(void)
{
    uint32_t *src = &_sidata;
    uint32_t *dst = &_sdata;

    while (dst < &_edata) {
        *dst++ = *src++;
    }

    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0u;
    }

    (void)main();
    for (;;) {
    }
}

void Default_Handler(void)
{
    for (;;) {
    }
}

__attribute__((section(".isr_vector"), used))
void (*const g_mspm0g3507_vectors[])(void) = {
    (void (*)(void))(&_estack),
    Reset_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
    Default_Handler,
};
