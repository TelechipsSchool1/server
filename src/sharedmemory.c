#include "sharedmemory.h"
#include "handle_client.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define SHM_NAME "/shared_memory" // 공유 메모리 이름
#define SHM_SIZE sizeof(SharedMemory)

//SharedMemory* shared_memory;

SharedMemory* shared_memory = NULL;

int init_shared_memory(){
    int shm_fd;

    // 공유 메모리 생성
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        return -1;
    }

    // 공유 메모리 크기 설정
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate failed");
        return -1;
    }

    // 공유 메모리 매핑
    shared_memory = mmap(NULL, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) {
        perror("mmap failed");
        return -1;
    }

    // 공유 메모리 초기화
    memset(shared_memory, 0, SHM_SIZE);


    pthread_mutexattr_t mutex_attr;
    pthread_mutexattr_init(&mutex_attr);
    pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&shared_memory->mutex, &mutex_attr);

    close(shm_fd); // shm_open 파일 디스크립터는 더 이상 필요 없음
    return 0;

}

void cleanup_shared_memory() {
    if (shared_memory != NULL) {
        // 매핑 해제
        munmap(shared_memory, SHM_SIZE);
        // 공유 메모리 삭제
        shm_unlink(SHM_NAME);
    }
}