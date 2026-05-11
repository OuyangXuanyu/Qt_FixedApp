//
// Created by Administrator on 26-4-3.
//

#include "diy_ui_toast-notification.h"
#include <QVBoxLayout>
#include <QPainter>
#include <QScreen>
#include <QGuiApplication>

ToastNotification::ToastNotification(QWidget *parent, const QString &message, int duration, const int width)
    : QWidget(parent), m_duration(duration)
{
    // 1. 窗口属性设置
    // Qt::ToolTip 保证不抢夺焦点，Qt::FramelessWindowHint 去掉边框
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground); // 背景透明
    setAttribute(Qt::WA_DeleteOnClose);         // 关闭后自动释放内存

    // 2. UI 布局
    auto *layout = new QVBoxLayout(this);
    m_label = new QLabel(message, this);
    m_label->setStyleSheet(
        "QLabel {"
        "  color: white;"
        "  background-color: rgba(192, 192, 192, 200);" // 半透明深灰色
        "  border-radius: 10px;"
        "  padding: 12px 25px;"
        "  font-family: 'Microsoft YaHei';"
        "  font-size: 13px;"
        "}"
    );
    m_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_label);

    // 3. 初始位置计算
    m_label->setFixedHeight(40);
    m_label->setFixedWidth(width);
    updatePosition(width);

    // 4. 启动动画流程
    fadeIn();
}

void ToastNotification::showMessage(QWidget *parent, const QString &message, const int duration, const int width) {
    auto *toast = new ToastNotification(parent, message, duration, width);
    toast->show();
}

void ToastNotification::updatePosition(const int widther) {
    if (parentWidget()) {
        // 相对于父窗口右上角
        const QPoint parentGlobalPos = parentWidget()->mapToGlobal(QPoint(0, 0));
        const int x = parentGlobalPos.x() + parentWidget()->width() - width() - widther + 90; // 靠右偏移 30
        const int y = parentGlobalPos.y() - 10; // 距离顶部 50
        move(x, y);
    }
}

void ToastNotification::fadeIn() {
    auto *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(100);
    animation->setStartValue(0.0);
    animation->setEndValue(1.0);
    animation->start(QAbstractAnimation::DeleteWhenStopped);

    // 停留一段时间后自动退出
    QTimer::singleShot(m_duration, this, &ToastNotification::fadeOut);
}

void ToastNotification::fadeOut() {
    auto *animation = new QPropertyAnimation(this, "windowOpacity");
    animation->setDuration(200);
    animation->setStartValue(1.0);
    animation->setEndValue(0.0);
    connect(animation, &QPropertyAnimation::finished, this, &QWidget::close);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
}

void ToastNotification::paintEvent(QPaintEvent *) {
    // 这里可以留空，样式表已处理背景
}