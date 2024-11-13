#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QSharedMemory>
#include "/root/server/sharedmemory.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// MainWindow 클래스 정의
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateLabels();  // Label 업데이트 함수

private:
    Ui::MainWindow *ui;
    QTimer *timer;

    void initSharedMemory();          // 공유 메모리 초기화 함수
    DataPacket readSharedMemoryData(int client_id);  // 공유 메모리에서 데이터 읽기 함수
};

#endif // MAINWINDOW_H
