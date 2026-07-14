//
// Created by Administrator on 26-5-8.
//

#ifndef MAINWINDOW_SERIAL_H
#define MAINWINDOW_SERIAL_H

#include <QDialog>
#include <QComboBox>
#include "../../manager_of_ui.h"
#include "ui_mainwindow_serial.h" // 添加这一行

namespace MyApp::UI{ class UiManager; }

namespace MyApp::UI::mainwindow_serial {
    QT_BEGIN_NAMESPACE
    namespace Ui { class mainwindow_serial; }
    QT_END_NAMESPACE

    class mainwindow_serial : public QDialog {
    Q_OBJECT

    public:
        explicit mainwindow_serial(QWidget *parent = nullptr, UiManager *ui_manager = nullptr);
        ~mainwindow_serial() override;

        Ui::mainwindow_serial *ui;
        void scan();


    private:
        UiManager *ui_manager;
        bool m_isDragging = false;
        QPoint m_dragPosition;

    private slots:
        void on_BTN_SerialConnect_clicked();

    protected:
        void mousePressEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
    };


} // MyApp::UI::mainwindow_serial

#endif //MAINWINDOW_SERIAL_H
