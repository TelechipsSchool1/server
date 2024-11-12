#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#define MAX_CLIENTS 10

typedef struct {
    int client_id;         // 클라이언트 ID
    int adc_value;         // ADC 값
    int pwm_duty_cycle;    // PWM Duty Cycle 값
    long distance;         // 초음파 거리 측정 값
    int led_state;         // LED GPIO 상태
} DataPacket;

typedef struct {
    DataPacket client_data[MAX_CLIENTS];   // 각 클라이언트의 데이터 저장 공간
    int client_count;                      // 현재 연결된 클라이언트 수
    pthread_mutex_t mutex;                 // 데이터 접근 동기화를 위한 뮤텍스
} SharedMemory;

int main() {
    int shm_fd;
    SharedMemory *shared_mem;

    // 공유 메모리 열기
    shm_fd = shm_open("/shared_memory", O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("Failed to open shared memory");
        return -1;
    }

    // 공유 메모리 매핑
    shared_mem = mmap(0, sizeof(SharedMemory), PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shared_mem == MAP_FAILED) {
        perror("Failed to map shared memory");
        close(shm_fd);
        return -1;
    }

    printf("Enter the client number to view the latest data (or 0 to exit):\n");

    while (1) {
        int client_num;
        printf("Client number: ");
        scanf("%d", &client_num);

        if (client_num == 0) {
            break;  // 프로그램 종료
        }

        if (client_num < 1 || client_num > MAX_CLIENTS) {
            printf("Invalid client number. Please enter a number between 1 and %d.\n", MAX_CLIENTS);
            continue;
        }

        if (client_num > shared_mem->client_count) {
            printf("Client %d is not connected.\n", client_num);
            continue;
        }

        // 선택한 클라이언트의 가장 최근 데이터 출력
        DataPacket packet = shared_mem->client_data[client_num - 1];
        printf("Latest data from Client %d:\n", client_num);
        printf("  ADC Value: %d\n", packet.adc_value);
        printf("  PWM Duty Cycle: %d\n", packet.pwm_duty_cycle);
        printf("  Distance: %ld cm\n", packet.distance);
        printf("  LED State: %s\n", packet.led_state ? "HIGH" : "LOW");
        printf("--------------------------\n");
    }

    // 공유 메모리 정리
    munmap(shared_mem, sizeof(SharedMemory));
    close(shm_fd);

    return 0;
}

