//
// Created by Administrator on 26-5-9.
//

#ifndef MAINWINDOW_BL_H
#define MAINWINDOW_BL_H

#include "../../manager_of_ui.h"
#include "ui_mainwindow_bl.h" // 添加这一行

namespace MyApp::UI{ class UiManager; }

namespace MyApp::UI::mainwindow_bl {
    QT_BEGIN_NAMESPACE
    namespace Ui { class mainwindow_bl; }
    QT_END_NAMESPACE

    class mainwindow_bl : public QWidget {
    Q_OBJECT

    public:
        explicit mainwindow_bl(QWidget *parent = nullptr, UiManager *ui_manager = nullptr);
        ~mainwindow_bl() override;

        Ui::mainwindow_bl *ui;
        void scan();

    private:
        UiManager *ui_manager;
        bool m_isDragging = false;
        QPoint m_dragPosition;


    private slots:
        void on_BTN_BlConnect_clicked();

    protected:
        void mousePressEvent(QMouseEvent *event) override;
        void mouseMoveEvent(QMouseEvent *event) override;
        void mouseReleaseEvent(QMouseEvent *event) override;
    };
} // MyApp::UI::mainwindow_bl

#endif //MAINWINDOW_BL_H
