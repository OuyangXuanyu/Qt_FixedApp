//
// Created by ouyangxuanyu on 2026/7/17.
//

// You may need to build the project (run Qt uic code generator) to get "ui_mainwindow_settings.h" resolved

#include "mainwindow_settings.h"
#include "ui_mainwindow_settings.h"

namespace MyApp::UI::mainwindow_settings {
    mainwindow_settings::mainwindow_settings(QWidget *parent, UiManager *ui_manager) : QWidget(parent), ui(new Ui::mainwindow_settings), ui_manager(ui_manager) {
        ui->setupUi(this);
    }

    mainwindow_settings::~mainwindow_settings() {
        delete ui;
    }

    // 1. 鼠标按下：开始记录偏移量
    void mainwindow_settings::mousePressEvent(QMouseEvent *event) {
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
    void mainwindow_settings::mouseMoveEvent(QMouseEvent *event) {
        if (m_isDragging && (event->buttons() & Qt::LeftButton)) {
            // 移动窗口到：当前鼠标全局坐标 - 之前记录的偏移量
            move(event->globalPosition().toPoint() - m_dragPosition);
            event->accept();
        }
    }

    // 3. 鼠标释放：停止拖动
    void mainwindow_settings::mouseReleaseEvent(QMouseEvent *event) {
        m_isDragging = false;
        event->accept();
    }
} // MyApp::UI::mainwindow_settings
