#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 9999        // 포트 번호
#define BUFFER_SIZE 4096 // 데이터 버퍼 크기

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];
    FILE *file;

    // 1. 소켓 생성
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // 2. 주소 설정
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 3. 소켓 바인딩
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Socket binding failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 4. 연결 대기
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Waiting for connection on port %d...\n", PORT);

    // 5. 클라이언트 연결 수락
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("Connection acceptance failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    printf("Connection established.\n");

    // 6. 데이터 수신 및 파일 저장
    while (1) {
        // 데이터 크기 수신 (4바이트)
        uint32_t size;
        int bytes_read = recv(new_socket, &size, sizeof(size), 0);
        if (bytes_read <= 0) {
            printf("Connection closed or error occurred.\n");
            break;
        }

        size = ntohl(size); // 네트워크 바이트 오더를 호스트 바이트 오더로 변환
        printf("Receiving file of size %u bytes...\n", size);

        // hello.jpg 파일 열기
        file = fopen("hello.jpg", "wb");
        if (file == NULL) {
            perror("Failed to open file");
            break;
        }

        // 파일 데이터 수신
        uint32_t received = 0;
        while (received < size) {
            int chunk_size = recv(new_socket, buffer, BUFFER_SIZE, 0);
            if (chunk_size <= 0) {
                perror("Error while receiving file data");
                fclose(file);
                break;
            }

            fwrite(buffer, 1, chunk_size, file);
            received += chunk_size;
        }

        fclose(file);
        printf("File received and saved as hello.jpg.\n");
    }

    // 7. 소켓 및 리소스 정리
    close(new_socket);
    close(server_fd);

    return 0;
}
