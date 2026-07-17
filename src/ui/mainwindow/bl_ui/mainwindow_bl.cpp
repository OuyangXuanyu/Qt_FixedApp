//
// Created by Administrator on 26-5-9.
//

// You may need to build the project (run Qt uic code generator) to get "ui_mainwindow_bl.h" resolved

#include "mainwindow_bl.h"
#include "ui_mainwindow_bl.h"

namespace MyApp::UI::mainwindow_bl {
    mainwindow_bl::mainwindow_bl(QWidget *parent, UiManager *ui_manager) : QWidget(parent), ui(new Ui::mainwindow_bl), ui_manager(ui_manager) {
        ui->setupUi(this);
    }

    mainwindow_bl::~mainwindow_bl() {
        delete ui;
    }

    // 1. 鼠标按下：开始记录偏移量
    void mainwindow_bl::mousePressEvent(QMouseEvent *event) {
        if (event->button() == Qt::LeftButton) {
            if (this->rect().contains(event->pos())) {
                m_isDragging = true;
                m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
                event->accept();
            }
        }
        QWidget::mousePressEvent(event);
    }

    // 2. 鼠标移动：实时同步窗口位置
    void mainwindow_bl::mouseMoveEvent(QMouseEvent *event) {
        if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
            // 移动窗口到：当前鼠标全局坐标 - 之前记录的偏移量
            move(event->globalPosition().toPoint() - m_dragPosition);
            event->accept();
        }
    }

    // 3. 鼠标释放：停止拖动
    void mainwindow_bl::mouseReleaseEvent(QMouseEvent *event) {
        m_isDragging = false;
        event->accept();
    }

    void mainwindow_bl::scan() {
        const auto items = ui_manager->page_mainwindow->serialMgr->serialGetPorts();
        ui->CBBox_Bl->clear();
        ui->CBBox_Bl->addItems(items);
    }

    void mainwindow_bl::on_BTN_BlConnect_clicked() {
        auto *mainWindow = ui_manager->page_mainwindow.data();
        if (!mainWindow)
            return;

        if (!mainWindow->isBLConnected) {
            const int index = ui->CBBox_Bl->currentIndex();
            if (index < 0 || index >= mainWindow->BLscannedDevices.size()) {
                std::cout<<"[ERROR] 未选择有效设备"<<std::endl;
                return;
            }

            const QBluetoothDeviceInfo device = mainWindow->BLscannedDevices.at(index);
            const auto startBluetoothConnection = [mainWindow, device] {
                std::cout << "[INFO] 正在连接..." << std::endl;
                mainWindow->blMgr->connectToDevice(device);
            };

            if (!mainWindow->isSerialConnected) {
                startBluetoothConnection();
                return;
            }

            // 串口关闭在工作线程中执行，确认关闭后再连接蓝牙。
            const QMetaObject::Connection pendingConnection = connect(
                mainWindow, &mainwindow::mainwindow::serialDisconnected,
                mainWindow, startBluetoothConnection,
                Qt::SingleShotConnection);
            if (!mainWindow->disconnectSerial())
                QObject::disconnect(pendingConnection);
            return;
        }

        mainWindow->disconnectBluetooth();
    }
} // MyApp::UI::mainwindow_bl
