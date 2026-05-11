//
// Created by Administrator on 26-5-11.
//

// You may need to build the project (run Qt uic code generator) to get "ui_mainwindow_bottom_menu.h" resolved

#include "mainwindow_bottom_menu.h"
#include "ui_mainwindow_bottom_menu.h"
#include <QMenu>
#include <QPainter>
#include "../ui_mainwindow.h"

namespace MyApp::UI::mainwindow_bottom_menu {
    mainwindow_bottom_menu::mainwindow_bottom_menu(QWidget *parent) :
        QWidget(parent), ui(new Ui::mainwindow_bottom_menu) {
        ui->setupUi(this);
        setAttribute(Qt::WA_AlwaysStackOnTop);

        try {
            parent_ptr = qobject_cast<mainwindow::mainwindow*>(parent);
        } catch (const std::exception& e) {
            std::cout << "Exception: " << e.what() << std::endl;
        }

        // 初始化动画器
        animation = new QPropertyAnimation(this, "pos");
        animation->setDuration(250);
        animation->setEasingCurve(QEasingCurve::OutCubic);
        // 允许背景透明绘制
        this->setAttribute(Qt::WA_TranslucentBackground);

        // 初始化 serial 和 bl 的 Popup 窗口
        initSerialWindow();
        initBlWindow();

        // // ==========================================
        // // 在这里你可以继续用代码绑定二级菜单等复杂逻辑
        // // ==========================================
        // QMenu *moreMenu = new QMenu(this);
        // moreMenu->addAction("开始采集");
        // moreMenu->addAction("停止采集");
        // // 假设你在 ui 里有个按钮叫 btn_settings
        // ui->BTN_Connection->setMenu(moreMenu);

        // 防误触逻辑：菜单关闭时如果鼠标不在面板上，收起面板
        // connect(moreMenu, &QMenu::aboutToHide, this, [this]() {
        //     QTimer::singleShot(50, this, [this]() {
        //         if (!this->underMouse()) {
        //             slideDown();
        //         }
        //     });
        // });
        // 确保它能捕获鼠标事件
        this->setAttribute(Qt::WA_Hover);
    }

    mainwindow_bottom_menu::~mainwindow_bottom_menu() {
        delete ui;
    }

    void mainwindow_bottom_menu::updatePosition()
    {
        if (!parentWidget()) return;
        // 获取父窗口当前的实时尺寸
        const int parentWidth = parentWidget()->width();
        const int parentHeight = parentWidget()->height();

        // 重新设置自己的宽度
        this->setFixedWidth(parentWidth);

        // 计算位置
        // 隐藏状态：只露出顶部 5 像素
        hiddenY = parentHeight - 20;
        // 显示状态：完全露出来（假设你 UI 设定的高度是 80）
        visibleY = parentHeight - this->height() + 2;

        // 核心调试：如果在这里 move，它应该立刻去到底部
        this->move(0, hiddenY);

        // 再次提升层级，防止被 late-init 的控件遮挡
        this->raise();
    }

    void mainwindow_bottom_menu::enterEvent(QEnterEvent *event)
    {
        slideUp();
        QWidget::enterEvent(event);
    }

    void mainwindow_bottom_menu::leaveEvent(QEvent *event)
    {
        if ((window_blSet && window_blSet->isVisible()) || (window_serialSet && window_serialSet->isVisible())) {
            return;
        }
        // 2. 获取全局鼠标位置
        QPoint globalPos = QCursor::pos();

        // 3. 获取主窗口当前的屏幕位置和高度
        // 注意：这里用 this->window() 确保拿到的坐标系是正确的
        QRect windowRect = this->window()->geometry();

        /* * 判定逻辑：
         * 如果鼠标离开菜单时，其全局坐标 Y 值大于等于（窗口底边缘 - 3像素）
         * 这说明鼠标实际上是划到了屏幕最底下的 1-3 像素“死区”或更靠下的位置。
         * 此时我们【不执行】slideDown，让菜单保持弹出。
         */
        constexpr int threshold = 2; // 判定阈值，根据你 WS_BORDER 带来的边框感微调
        if (globalPos.y() >= (windowRect.bottom() - threshold)) {
            // 虽然触发了 leaveEvent，但因为鼠标在最底边，我们强行拦截，不让它缩回去
            return;
        }
        slideDown();
        QWidget::leaveEvent(event);
    }

    void mainwindow_bottom_menu::slideUp()
    {
        animation->stop();
        animation->setEndValue(QPoint(x(), visibleY));
        animation->start();
    }

    void mainwindow_bottom_menu::slideDown()
    {
        animation->stop();
        animation->setEndValue(QPoint(x(), hiddenY));
        animation->start();
    }

    void mainwindow_bottom_menu::on_BTN_Patient_StartTest_clicked() {
        parent_ptr->on_BTN_Patient_StartTest_clicked();
    }

    void mainwindow_bottom_menu::on_BTN_Patient_AddANDUpdate_clicked(){
        parent_ptr->on_BTN_Patient_AddANDUpdate_clicked();
    }

    void mainwindow_bottom_menu::on_BTN_Patient_SearchRecord_clicked(){
        parent_ptr->on_BTN_Patient_SearchRecord_clicked();
    }

    void mainwindow_bottom_menu::on_BTN_Test_clicked(){
        parent_ptr->on_BTN_Test_clicked();
    }

    void mainwindow_bottom_menu::on_BTN_Exit_clicked(){
        parent_ptr->on_BTN_EXIT_clicked();
    }

    void mainwindow_bottom_menu::on_RBTN_BL_clicked(){
        ToastNotification::showMessage(parent_ptr, "开始扫描蓝牙", 2500);

        // parent_ptr->on_RBTN_Serial_clicked();
        if (!window_blSet) return;
        QPoint pos = ui->RBTN_Serial->mapToGlobal(QPoint(0, 0));
        pos.setY(pos.y() - window_blSet->height() - 2);
        window_blSet->move(pos);
        window_blSet->show();
        window_blSet->activateWindow();
        if (parent_ptr->isBLConnected) return;
        window_blSet->ui->CBBox_Bl->clear();
        parent_ptr->blMgr->blScr->startScan();
    }
    void mainwindow_bottom_menu::on_RBTN_Serial_clicked(){
        // parent_ptr->on_RBTN_BL_clicked();
        ToastNotification::showMessage(parent_ptr, "开始扫描串口", 2500, parent_ptr->ui->Label_Welcome->width());
        if (!window_serialSet) return;
        QPoint pos = ui->RBTN_Serial->mapToGlobal(QPoint(0, 0));
        pos.setY(pos.y() - window_serialSet->height() - 2);
        window_serialSet->move(pos);
        window_serialSet->show();
        window_serialSet->activateWindow();
        if (parent_ptr->isSerialConnected) return;
        window_serialSet->scan();
    }

    void mainwindow_bottom_menu::initSerialWindow() {
        if (!window_serialSet) {
            window_serialSet = new mainwindow_serial::mainwindow_serial(this, parent_ptr->ui_manager);
            window_serialSet->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        }
    }

    void mainwindow_bottom_menu::initBlWindow() {
        if (!window_blSet) {
            window_blSet = new mainwindow_bl::mainwindow_bl(this, parent_ptr->ui_manager);
            window_blSet->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        }
    }

} // MyApp::UI::mainwindow_bottom_menu
