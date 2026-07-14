//
// Created by Administrator on 26-5-8.
//

// You may need to build the project (run Qt uic code generator) to get "ui_mainwindow_serial.h" resolved

#include "mainwindow_serial.h"
#include "ui_mainwindow_serial.h"

namespace MyApp::UI::mainwindow_serial {
    mainwindow_serial::mainwindow_serial(QWidget *parent, UiManager *ui_manager) : QDialog(parent), ui(new Ui::mainwindow_serial), ui_manager(ui_manager) {
        ui->setupUi(this);
    }

    mainwindow_serial::~mainwindow_serial() {
        delete ui;
    }


    // 1. 鼠标按下：开始记录偏移量
    void mainwindow_serial::mousePressEvent(QMouseEvent *event) {
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
    void mainwindow_serial::mouseMoveEvent(QMouseEvent *event) {
        if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
            // 移动窗口到：当前鼠标全局坐标 - 之前记录的偏移量
            move(event->globalPosition().toPoint() - m_dragPosition);
            event->accept();
        }
    }

    // 3. 鼠标释放：停止拖动
    void mainwindow_serial::mouseReleaseEvent(QMouseEvent *event) {
        m_isDragging = false;
        event->accept();
    }

    void mainwindow_serial::scan() {
        const auto items = ui_manager->page_mainwindow->serialMgr->serialGetPorts();
        ui->CBBox_Serial->clear();
        ui->CBBox_Serial->addItems(items);
    }

    void mainwindow_serial::on_BTN_SerialConnect_clicked() {
        if (!ui_manager->page_mainwindow->isSerialConnected) {
            emit ui_manager->page_mainwindow->startSerial(
                ui->CBBox_Serial->currentText(),
                3000000,
                QSerialPort::Data8,
                QSerialPort::OneStop,
                QSerialPort::NoParity
                );
        }
        else {
            emit ui_manager->page_mainwindow->stopSerial();
        }

    }


} // MyApp::UI::mainwindow_serial
