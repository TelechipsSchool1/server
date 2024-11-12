#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#define SERVER_PORT 12345          // 서버의 포트 번호
#define MAX_CLIENTS 10             // 최대 클라이언트 수

typedef struct {
    int adc_value;         // ADC 값
    int pwm_duty_cycle;    // PWM Duty Cycle 값
    int distance;          // 초음파 거리 측정 값
    int led_state;         // LED GPIO 상태
} DataPacket;

typedef struct {
    DataPacket client_data[MAX_CLIENTS];   // 각 클라이언트의 데이터 저장 공간
    int client_count;                      // 현재 연결된 클라이언트 수
    pthread_mutex_t mutex;                 // 데이터 접근 동기화를 위한 뮤텍스
} SharedMemory;

SharedMemory *shared_mem;
int client_id_counter = 1;  // 클라이언트 ID 할당을 위한 카운터

void* handle_client(void* arg) {
    int client_sock = *(int*)arg;
    free(arg);

    int client_id;
    pthread_mutex_lock(&shared_mem->mutex);
    client_id = client_id_counter++;
    pthread_mutex_unlock(&shared_mem->mutex);

    DataPacket packet;
    ssize_t bytes_received;

    while (1) {
        memset(&packet, 0, sizeof(DataPacket));

        bytes_received = recv(client_sock, &packet, sizeof(DataPacket), 0);
        if (bytes_received < sizeof(DataPacket)) {
            if (bytes_received <= 0) {
                if (bytes_received == 0) {
                    printf("Client %d disconnected\n", client_id);
                } else {
                    fprintf(stderr, "recv failed: %s\n", strerror(errno));
                }
                close(client_sock);
                break;
            } else {
                fprintf(stderr, "Partial packet received: %ld bytes (expected %ld bytes)\n", 
                        bytes_received, sizeof(DataPacket));
                continue;
            }
        }

        pthread_mutex_lock(&shared_mem->mutex);
        if (shared_mem->client_count <= MAX_CLIENTS) {
            shared_mem->client_data[client_id - 1] = packet;
            printf("Stored data from client %d. Total clients: %d\n", client_id, shared_mem->client_count);
        } else {
            printf("Maximum client limit reached.\n");
        }
        pthread_mutex_unlock(&shared_mem->mutex);

        // 수신된 데이터 출력
        printf("Received Data from Client %d:\n", client_id);
        printf("  ADC Value: %d\n", packet.adc_value);
        printf("  PWM Duty Cycle: %d\n", packet.pwm_duty_cycle);
        printf("  Distance: %d cm\n", packet.distance);
        printf("  LED State: %s\n", packet.led_state ? "HIGH" : "LOW");
        printf("--------------------------\n");
    }

    pthread_mutex_lock(&shared_mem->mutex);
    shared_mem->client_count--;
    pthread_mutex_unlock(&shared_mem->mutex);

    return NULL;
}

int main() {
    int server_sock, *client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pthread_t tid;

    int shm_fd = shm_open("/shared_memory", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(SharedMemory));
    shared_mem = mmap(0, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    memset(shared_mem, 0, sizeof(SharedMemory));
    pthread_mutex_init(&shared_mem->mutex, NULL);

    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, "192.168.137.7", &server_addr.sin_addr) <= 0) {
        perror("Invalid address/Address not supported");
        close(server_sock);
        return -1;
    }
    server_addr.sin_port = htons(SERVER_PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock);
        return -1;
    }

    if (listen(server_sock, 10) < 0) {
        perror("Listen failed");
        close(server_sock);
        return -1;
    }

    printf("Server listening on IP 192.168.137.7, port %d\n", SERVER_PORT);

    while (1) {
        client_sock = malloc(sizeof(int));
        if ((*client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
            perror("Accept failed");
            free(client_sock);
            continue;
        }

        printf("Client connected\n");

        pthread_mutex_lock(&shared_mem->mutex);
        shared_mem->client_count++;
        pthread_mutex_unlock(&shared_mem->mutex);

        if (pthread_create(&tid, NULL, handle_client, client_sock) != 0) {
            perror("pthread_create failed");
            free(client_sock);
            continue;
        }

        pthread_detach(tid);
    }

    pthread_mutex_destroy(&shared_mem->mutex);
    munmap(shared_mem, sizeof(SharedMemory));
    close(shm_fd);
    shm_unlink("/shared_memory");
    close(server_sock);
    return 0;
}

