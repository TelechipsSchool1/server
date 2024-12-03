// 
#ifndef HANDLE_CLIENT_H
#define HANDLE_CLIENT_H

#include <stdbool.h>
#include <pthread.h>
// zone1_3 recv 11111
// zone2 recv 8080
// zone1_3 send port 12345


// DataPacket_zone1 & 3 구조체 정의
struct Zone1_3_Data_recv {
    int ID; // ID
    float ultrasonic_distance;  // 초음파 거리
    float temperature;          // 온도
    float humidity;             // 습도
    float pressure;             // 압력 센서  
    int door_status;        //  문 상태 
    int window_status;          // LED GPIO 상태
};

struct Zone2_Data_recv{
    int ID;         // ADC 값
    float co2;    // PWM Duty Cycle 값
    float heart;          // 초음파 거리 측정 값
    int sleep_score;         // LED GPIO 상태
};

struct Zone1_3_Data_send{
    int window_command; // 창문 여닫기 여부
};


typedef struct Zone1_3_Data_recv Zone1_3_Data_recv;
typedef struct Zone2_Data_recv Zone2_Data_recv;
typedef struct Zone1_3_Data_send Zone1_3_Data_send;


typedef struct {
    Zone1_3_Data_recv zone1_recv;
    Zone2_Data_recv zone2_recv;
    Zone1_3_Data_recv zone3_recv;
    Zone1_3_Data_send zone1_send;
    Zone1_3_Data_send zone3_send;
    pthread_mutex_t mutex;
} SharedMemory;

void* handle_client1_3_send(void* arg);
void* handle_client2_recv(void* arg);
void* handle_client1_3_recv(void* arg);


extern SharedMemory* shared_memory;


#endif // HANDLE_CLIENT_H

