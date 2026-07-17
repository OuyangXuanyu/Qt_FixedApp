//
// Created by ouyangxuanyu on 2026/7/17.
//

#ifndef QT_PROJECT_FIXEDAPP_MAINWINDOW_SETTINGS_H
#define QT_PROJECT_FIXEDAPP_MAINWINDOW_SETTINGS_H

#include "../../manager_of_ui.h"
#include "ui_mainwindow_settings.h" // 添加这一行

namespace MyApp::UI{ class UiManager; }

namespace MyApp::UI::mainwindow_settings {
    QT_BEGIN_NAMESPACE
    namespace Ui { class mainwindow_settings; }
    QT_END_NAMESPACE

    class mainwindow_settings : public QWidget {
        Q_OBJECT

    public:
        explicit mainwindow_settings(QWidget *parent = nullptr, UiManager *ui_manager = nullptr);
        ~mainwindow_settings() override;

        Ui::mainwindow_settings *ui;

    private:
        UiManager *ui_manager;
        bool m_isDragging = false;
        QPoint m_dragPosition;

    private slots:

    protected:
        void mousePressEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
    };
} // MyApp::UI::mainwindow_settings

#endif //QT_PROJECT_FIXEDAPP_MAINWINDOW_SETTINGS_H
