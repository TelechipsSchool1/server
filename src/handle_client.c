#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "handle_client.h"
#include "sharedmemory.h"
#include "fan.h"

void print_sharedmemory(){
    printf("Zone 1 recv\n");
    printf(" distance : %f\n temperature : %f\n humidity : %f\n pressure : %d\n door_status : %d\n window_status : %d\n",\
                      shared_memory->zone1_recv.ultrasonic_distance, shared_memory->zone1_recv.temperature,\
                      shared_memory->zone1_recv.humidity, shared_memory->zone1_recv.pressure,\
                       shared_memory->zone1_recv.door_status, shared_memory->zone1_recv.window_status);
    printf("Zone 2 recv\n");
    printf(" ID : %d\n CO2 : %f\n heart : %f\n sleep_score : %d\n",shared_memory->zone2_recv.ID,\
                     shared_memory->zone2_recv.co2, shared_memory->zone2_recv.heart, shared_memory->zone2_recv.sleep_score);

    printf("Zone 3 recv\n");
    printf(" distance : %f\n temperature : %f\n humidity : %f\n pressure : %d\n door_status : %d\n window_status : %d\n",\
                      shared_memory->zone3_recv.ultrasonic_distance, shared_memory->zone3_recv.temperature,\
                      shared_memory->zone3_recv.humidity, shared_memory->zone3_recv.pressure,\
                       shared_memory->zone3_recv.door_status, shared_memory->zone3_recv.window_status);
}

void* handle_client1_3_recv(void* arg) {
    int client_sock = *(int*)arg; // 클라이언트 소켓 디스크립터
    free(arg); // 동적 할당된 메모리 해제

    Zone1_3_Data_recv buffer; // 수신 데이터를 저장할 구조체
    int bytes_read;

    printf("handle_client1_3_recv\n");

    while ((bytes_read = recv(client_sock, &buffer, sizeof(buffer), 0)) > 0) {
        if (bytes_read != sizeof(buffer)) {
            fprintf(stderr, "Incomplete data received. Expected %lu bytes, got %d bytes\n",
                    sizeof(buffer), bytes_read);
            continue;
        }

        // 공유 메모리에 Zone2 수신 데이터 업데이트
        pthread_mutex_lock(&shared_memory->mutex); // 공유 메모리 접근 동기화
        if(buffer.ID == 1){
            shared_memory->zone1_recv = buffer;
        } 
        else if (buffer.ID == 3){
            shared_memory->zone3_recv = buffer;
        }// 구조체를 통째로 복사
        pthread_mutex_unlock(&shared_memory->mutex); // 공유 메모리 접근 해제

        // 수신 데이터 출력
        /*
        printf("Zone2 Data Updated:\n");
        printf("  ID: %d\n", shared_memory->zone1_recv.ID);
        printf("  CO2: %.2f\n", shared_memory->zone1_recv.ultrasonic_distance);
        printf("  Heart Rate: %.2f\n", shared_memory->zone1_recv.humidity);
        printf("  Sleep Score: %f\n", shared_memory->zone1_recv.pressure);
        */
        print_sharedmemory();
        sleep(1);
    }

    if (bytes_read == 0) {
        printf("Client disconnected.\n");
    } else if (bytes_read < 0) {
        perror("recv failed");
    }

    close(client_sock); // 클라이언트 소켓 닫기
    return NULL;
}

void* handle_client2_recv(void* arg) {
    int client_sock = *(int*)arg; // 클라이언트 소켓 디스크립터
    free(arg); // 동적 할당된 메모리 해제

    Zone2_Data_recv buffer; // 수신 데이터를 저장할 구조체
    int bytes_read;

    printf("handle_client2_recv\n");

    while ((bytes_read = recv(client_sock, &buffer, sizeof(buffer), 0)) > 0) {
        if (bytes_read != sizeof(buffer)) {
            fprintf(stderr, "Incomplete data received. Expected %lu bytes, got %d bytes\n",
                    sizeof(buffer), bytes_read);
            continue;
        }

        // 공유 메모리에 Zone2 수신 데이터 업데이트
        pthread_mutex_lock(&shared_memory->mutex); // 공유 메모리 접근 동기화

        shared_memory->zone2_recv = buffer; // 구조체를 통째로 복사
        if(buffer.sleep_score > 255){
            shared_memory->zone3_send.sleep_alert = 1;
            shared_memory->zone1_send.sleep_alert = 1;
            shared_memory->zone3_send.window_command = 1;
            shared_memory->zone1_send.window_command = 1;
        }
        else{
            shared_memory->zone3_send.sleep_alert = 0;
            shared_memory->zone1_send.sleep_alert = 0;
            shared_memory->zone3_send.window_command = 0;
            shared_memory->zone1_send.window_command = 0;
        }

        pthread_mutex_unlock(&shared_memory->mutex); // 공유 메모리 접근 해제
        if(buffer.sleep_score > 128 && buffer.sleep_score < 255){
            fan_set_speed(3);
        }
        else if(buffer.sleep_score > 512){
            fan_set_speed(3); 
        }
        else{
            fan_set_speed(0);
        }

        //////////////////////////////////
        sleep(1);
/*
        // 수신 데이터 출력
        printf("Zone2 Data Updated:\n");
        printf("  ID: %d\n", shared_memory->zone2_recv.ID);
        printf("  CO2: %.2f\n", shared_memory->zone2_recv.co2);
        printf("  Heart Rate: %.2f\n", shared_memory->zone2_recv.heart);
        printf("  Sleep Score: %d\n", shared_memory->zone2_recv.sleep_score);*/
    }

    if (bytes_read == 0) {
        printf("Client disconnected.\n");
    } else if (bytes_read < 0) {
        perror("recv failed");
    }

    close(client_sock); // 클라이언트 소켓 닫기
    return NULL;
}


void* handle_client1_3_send(void* arg) {
    int client_sock = *(int*)arg;
    free(arg); // 메모리 해제
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    printf("handle_client1_3_send\n");

    // 클라이언트 주소를 가져옴
    if (getpeername(client_sock, (struct sockaddr*)&client_addr, &addr_len) < 0) {
        perror("getpeername failed");
        close(client_sock);
        return NULL;
    }

    char* client_ip = inet_ntoa(client_addr.sin_addr);
    printf("Handling send for client with IP: %s\n", client_ip);

    Zone1_3_Data_send buffer;


    while (1) {
        // 클라이언트 연결 상태 확인
        // 클라이언트 IP에 따라 다른 메시지를 전송

        pthread_mutex_lock(&shared_memory->mutex);
        if (strcmp(client_ip, "192.168.137.2") == 0) {
            buffer = shared_memory->zone1_send;
        } else {
            buffer = shared_memory->zone3_send;
            shared_memory->zone1_send.window_command = !buffer.window_command ;
        }
        pthread_mutex_unlock(&shared_memory->mutex); // 공유 메모리 접근 해제

        if (send(client_sock, &buffer, sizeof(buffer), 0) < 0) {
            perror("Send failed, closing connection");
            break;
        }

        printf("Sent to client [%s] : %d\n", client_ip, buffer.window_command);

        // 주기적 대기 (예: 1초)
        sleep(1);
    }

    close(client_sock); // 연결 종료
    return NULL;
}

