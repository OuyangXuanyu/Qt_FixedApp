//
// Created by Administrator on 26-5-11.
//

#ifndef MAINWINDOW_BOTTOM_MENU_H
#define MAINWINDOW_BOTTOM_MENU_H

#include <QWidget>
#include <QPropertyAnimation>
#include "../../manager_of_ui.h"
#include "../mainwindow.h"
#include "../../../tools/diy_ui_toast-notification/diy_ui_toast-notification.h"


namespace MyApp::UI::mainwindow {
    class mainwindow;
}
namespace MyApp::UI::mainwindow_serial {
    class mainwindow_serial;
}
namespace MyApp::UI::mainwindow_bl {
    class mainwindow_bl;
}
namespace MyApp::UI::mainwindow_bottom_menu {
    QT_BEGIN_NAMESPACE
    namespace Ui { class mainwindow_bottom_menu; }
    QT_END_NAMESPACE

    class mainwindow_bottom_menu : public QWidget {
    Q_OBJECT

    friend class mainwindow::mainwindow;

    public:
        explicit mainwindow_bottom_menu(QWidget *parent = nullptr);
        ~mainwindow_bottom_menu() override;
        // 留给主窗口调用的更新位置函数
        void updatePosition();

        QPointer<mainwindow_serial::mainwindow_serial> window_serialSet = nullptr;
        QPointer<mainwindow_bl::mainwindow_bl> window_blSet = nullptr;

        void initSerialWindow();
        void initBlWindow();

    protected:
        // 重写鼠标进入和离开事件
        void enterEvent(QEnterEvent *event) override;
        void leaveEvent(QEvent *event) override;

    private:
        Ui::mainwindow_bottom_menu *ui;
        QPropertyAnimation *animation;
        int hiddenY;
        int visibleY;

        void slideUp();
        void slideDown();

        QPointer<mainwindow::mainwindow> parent_ptr = nullptr;

    private slots:
        void on_BTN_Patient_StartTest_clicked();
        void on_BTN_Patient_AddANDUpdate_clicked();
        void on_BTN_Patient_SearchRecord_clicked();
        void on_BTN_Test_clicked();
        void on_BTN_Exit_clicked();

        void on_RBTN_BL_clicked();
        void on_RBTN_Serial_clicked();

    };
} // MyApp::UI::mainwindow_bottom_menu

#endif //MAINWINDOW_BOTTOM_MENU_H
