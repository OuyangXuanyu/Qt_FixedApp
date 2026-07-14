//
// Created by Administrator on 26-4-3.
//

#ifndef DIY_UI_TOAST_NOTIFICATION_H
#define DIY_UI_TOAST_NOTIFICATION_H

#include <QWidget>
#include <QLabel>
#include <QPropertyAnimation>
#include <QTimer>

class ToastNotification : public QWidget {
    Q_OBJECT
public:
    // 参数：parent (主窗口), message (显示内容), duration (停留时间，毫秒)
    // todo: 设置通知类型 (info, warning, error) 与日志文件对其颗粒度
    explicit ToastNotification(QWidget *parent, const QString &message, int duration = 2000, int width = 200);

    // 静态快捷调用方法
    static void showMessage(QWidget *parent, const QString &message, int duration = 2000, int width = 200);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void fadeIn();
    void fadeOut();
    void updatePosition(int widther);

    QLabel *m_label;
    int m_duration;
};


#endif //DIY_UI_TOAST_NOTIFICATION_H
