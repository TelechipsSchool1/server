#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_PORT 8080     // 서버 포트 번호
#define BUFFER_SIZE 1024     // 버퍼 크기

int main() {
    int server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size;
    char buffer[BUFFER_SIZE];
    int str_len;

    // 소켓 생성
    server_sock = socket(PF_INET, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket error");
        exit(1);
    }

    // 서버 주소 설정
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(SERVER_PORT);

    // 소켓 바인딩
    if (bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind error");
        close(server_sock);
        exit(1);
    }

    // 연결 대기
    if (listen(server_sock, 5) == -1) {
        perror("listen error");
        close(server_sock);
        exit(1);
    }
    printf("Waiting for a client to connect...\n");

    // 클라이언트 연결 수락
    client_addr_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_size);
    if (client_sock == -1) {
        perror("accept error");
        close(server_sock);
        exit(1);
    }
    printf("Client connected...\n");

    // 클라이언트로부터 메시지 수신
    str_len = read(client_sock, buffer, BUFFER_SIZE - 1);
    if (str_len == -1) {
        perror("read error");
        close(client_sock);
        close(server_sock);
        exit(1);
    }

    buffer[str_len] = '\0';  // 문자열 끝에 NULL 추가
    printf("Message from client: %s\n", buffer);

    // 클라이언트에게 메시지 회신
    write(client_sock, buffer, str_len);

    // 소켓 종료
    close(client_sock);
    close(server_sock);
    printf("Server closed.\n");

    return 0;
}

