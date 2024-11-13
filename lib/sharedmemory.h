// sharedmemory.h
#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

// DataPacket 구조체 정의
typedef struct {
    int adc_value;         // ADC 값
    int pwm_duty_cycle;    // PWM Duty Cycle 값
    int distance;          // 초음파 거리 측정 값
    int led_state;         // LED GPIO 상태
} DataPacket;

#endif // SHAREDMEMORY_H

