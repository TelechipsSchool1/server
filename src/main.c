#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

#include "fan.h"
#include "handle_client.h"
#include "sharedmemory.h"

#define SERVER_PORT 12345
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define ZONE2_PORT 8080
#define ZONE1_3_PORT 11111





int main() {
    int server_sock, *client_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pthread_t tid;

    init_shared_memory();
    fan_init();
    printf("%ld\n", sizeof(SharedMemory));

    // 서버 소켓 생성
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(SERVER_PORT);

    // 바인드 및 리슨
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_sock);
        return -1;
    }

    if (listen(server_sock, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_sock);
        return -1;
    }

    printf("Server listening on port %d\n", SERVER_PORT);

    while (1) {
        client_sock = malloc(sizeof(int));
        if ((*client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_len)) < 0) {
            perror("Accept failed");
            free(client_sock);
            continue;
        }

        printf("Client connected from %s:%d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        // 클라이언트 구분
        if (ntohs(client_addr.sin_port) == ZONE2_PORT) { // Client 2
            if (pthread_create(&tid, NULL, handle_client2_recv, client_sock) != 0) {
                perror("pthread_create failed for Client 2");
                free(client_sock);
            }
        } else if(ntohs(client_addr.sin_port) == ZONE1_3_PORT) {
             // Client 1과 Client 3
            if (pthread_create(&tid, NULL, handle_client1_3_recv, client_sock) != 0) {
                perror("pthread_create failed for Receiving Client 1 or 3");
                free(client_sock);
            }
            if (pthread_create(&tid, NULL, handle_client1_3_send, client_sock) != 0) {
                perror("pthread_create failed for Sending Client 1 or 3");
                free(client_sock);
            }

        } else if(ntohs(client_addr.sin_port) == 10101) {
             // Client 1과 Client 3
            if (pthread_create(&tid, NULL, handle_client1_3_send, client_sock) != 0) {
                perror("pthread_create failed for Sending Client 1 or 3");
                free(client_sock);
            }
        } else {

        }

        pthread_detach(tid); // 스레드 리소스 자동 해제
    }

    cleanup_shared_memory();
    close(server_sock);
    return 0;
}
