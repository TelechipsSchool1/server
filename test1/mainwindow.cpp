#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <cstring>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <QTimer>
#include <pthread.h>

// 서버와 동일한 구조체 정의
#define MAX_CLIENTS 10

//struct DataPacket {
//    int adc_value;         // ADC 값
//    int pwm_duty_cycle;    // PWM Duty Cycle 값
//    int distance;          // 초음파 거리 측정 값
//    int led_state;         // LED GPIO 상태
//};

struct SharedMemory {
    DataPacket client_data[MAX_CLIENTS];   // 각 클라이언트의 데이터 저장 공간
    int client_count;                      // 현재 연결된 클라이언트 수
    pthread_mutex_t mutex;                 // 데이터 접근 동기화를 위한 뮤텍스
};

SharedMemory *sharedMem = nullptr;
int shm_fd;

// MainWindow 클래스 생성자
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initSharedMemory();  // 공유 메모리 초기화

    // 타이머 설정 (500ms마다 업데이트)
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateLabels);
    timer->start(100);  // 500ms마다 업데이트
}

// MainWindow 클래스 소멸자
MainWindow::~MainWindow() {
    if (sharedMem != nullptr) {
        munmap(sharedMem, sizeof(SharedMemory));
        close();
    }
    delete ui;
}

// 공유 메모리 초기화 함수
void MainWindow::initSharedMemory() {
    shm_fd = shm_open("/shared_memory", O_RDWR, 0666);
    if (shm_fd == -1) {
        qDebug() << "Failed to open shared memory";
        return;
    }

    sharedMem = static_cast<SharedMemory *>(mmap(nullptr, sizeof(SharedMemory), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
    if (sharedMem == MAP_FAILED) {
        qDebug() << "Failed to map shared memory";
        sharedMem = nullptr;
        close();
        return;
    }
    qDebug() << "Shared memory initialized successfully";
}

// 공유 메모리에서 데이터 읽기 함수
DataPacket MainWindow::readSharedMemoryData(int client_id) {
    DataPacket packet = {0, 0, 0, 0};

    if (sharedMem != nullptr) {
        pthread_mutex_lock(&sharedMem->mutex); // 동기화를 위해 뮤텍스 잠금
        if (client_id < sharedMem->client_count && client_id < MAX_CLIENTS) {
            packet = sharedMem->client_data[client_id];
        }
        pthread_mutex_unlock(&sharedMem->mutex); // 뮤텍스 잠금 해제
    } else {
        qDebug() << "Shared memory not initialized";
    }

    return packet;
}

// Label 업데이트 함수
void MainWindow::updateLabels() {
    // 0번 클라이언트의 데이터를 읽어와서 표시 (필요에 따라 client_id를 변경 가능)
    int client_id = 0;
    DataPacket data = readSharedMemoryData(client_id);

    // Label 업데이트
    ui->label_2->setText("ADC Value: " + QString::number(data.adc_value));
    ui->label_3->setText("PWM Duty Cycle: " + QString::number(data.pwm_duty_cycle));
    ui->label_4->setText("Distance: " + QString::number(data.distance) + " cm");

    // LED 상태를 텍스트로 표시
    QString ledState = (data.led_state == 1) ? "ON" : "OFF";
    ui->label_5->setText("LED State: " + ledState);
}
