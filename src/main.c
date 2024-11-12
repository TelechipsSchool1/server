#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define SERVER_PORT 12345          // 서버의 포트 번호

typedef struct {
    int adc_value;          // ADC 값
    int pwm_duty_cycle;     // PWM Duty Cycle 값
    long distance;          // 초음파 거리 측정 값
    int led_state;          // LED GPIO 상태
} DataPacket;

void handle_client(int client_sock) {
    DataPacket packet;
    int bytes_received;

    while (1) {
        // DataPacket 크기만큼 데이터 수신
        bytes_received = recv(client_sock, &packet, sizeof(DataPacket), 0);
        if (bytes_received <= 0) {
            if (bytes_received == 0) {
                printf("Client disconnected\n");
            } else {
                perror("recv failed");
            }
            close(client_sock);
            break;
        }

        // 수신한 데이터 출력
        printf("Received Data:\n");
        printf("  ADC Value: %d\n", packet.adc_value);
        printf("  PWM Duty Cycle: %d\n", packet.pwm_duty_cycle);
        printf("  Distance: %ld cm\n", packet.distance);
        printf("  LED State: %s\n", packet.led_state ? "HIGH" : "LOW");
        printf("--------------------------\n");
    }
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // 서버 소켓 생성
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // 소켓 바인딩
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock);
        return -1;
    }

    // 클라이언트 연결 대기
    if (listen(server_sock, 1) < 0) {
        perror("Listen failed");
        close(server_sock);
        return -1;
    }

    printf("Server listening on port %d\n", SERVER_PORT);

    // 클라이언트 연결 허용
    while (1) {
        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            perror("Accept failed");
            continue;
        }

        printf("Client connected\n");
        handle_client(client_sock);
    }

    close(server_sock);
    return 0;
}
