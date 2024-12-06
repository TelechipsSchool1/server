//*******************************************
// 파일명 : pwm.c
// 작성자 : jude.kwon
// 작성일 : 24.10.31
// 수정일 : 24.11.07
// 
// 파일설명 : PWM 제어 기능을 위한 함수들을 정의
//********************************************



#include "pwm.h"

int initialize_pwm(const char *channel_path) {
    FILE *fp;

    // PWM Export
    fp = fopen("/sys/class/pwm/pwmchip0/export", "w");
    if (fp == NULL) {
        perror("Failed to export PWM");
        return -1;
    }
    fprintf(fp, "0");
    fclose(fp);

    // PWM Period 설정
    char period_path[256];
    snprintf(period_path, sizeof(period_path), "%s/period", channel_path);
    fp = fopen(period_path, "w");
    if (fp == NULL) {
        perror("Failed to set PWM period");
        return -1;
    }
    fprintf(fp, "%d", PWM_PERIOD);
    fclose(fp);

    // PWM Enable 설정
    char enable_path[256];
    snprintf(enable_path, sizeof(enable_path), "%s/enable", channel_path);
    fp = fopen(enable_path, "w");
    if (fp == NULL) {
        perror("Failed to enable PWM");
        return -1;
    }
    fprintf(fp, "1");
    fclose(fp);

    return 0;
}

void update_pwm_duty_cycle(const char *channel_path, int duty_cycle) {
    char duty_cycle_path[256];
    snprintf(duty_cycle_path, sizeof(duty_cycle_path), "%s/duty_cycle", channel_path);
    FILE *fp = fopen(duty_cycle_path, "w");
    if (fp == NULL) {
        perror("Failed to set PWM duty cycle");
        return;
    }
    fprintf(fp, "%d", duty_cycle);
    fclose(fp);
}

