#include "fan.h"
#include "pwm.h"
#include <stdio.h> 

// 팬 초기화
int fan_init() {
    if (initialize_pwm(PWM1_CHANNEL_PATH) < 0) {
        return -1;
    }
    return 0;
}

// 팬 속도 설정
void fan_set_speed(int speed) {
    int duty_cycle;

    switch (speed) {
        case 1: // 1단계
            duty_cycle = PWM_PERIOD * 0.3; // 30% 듀티 사이클
            break;
        case 2: // 2단계
            duty_cycle = PWM_PERIOD * 0.6; // 60% 듀티 사이클
            break;
        case 3: // 3단계
            duty_cycle = PWM_PERIOD * 0.9; // 90% 듀티 사이클
            break;
        default:
            duty_cycle = 0; // 속도가 1, 2, 3단계 외의 값이면 팬을 정지
    }

    update_pwm_duty_cycle(PWM1_CHANNEL_PATH, duty_cycle);
}

// 팬 종료
void fan_stop() {
    update_pwm_duty_cycle(PWM1_CHANNEL_PATH, 0); // 팬을 정지

    char enable_path[256];
    snprintf(enable_path, sizeof(enable_path), "%s/enable", PWM1_CHANNEL_PATH);

    FILE *fp = fopen(enable_path, "w");
    if (fp != NULL) {
        fprintf(fp, "0"); // PWM disable
        fclose(fp);
    }
}
