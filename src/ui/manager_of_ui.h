//
// Created by Administrator on 26-1-7.
//

#ifndef UI_CLASSES_H
#define UI_CLASSES_H
#pragma once

#include <QObject>
#include <QFile>
#include <QJsonObject>
#include <QCloseEvent>
#include <QPointer>
#include <QVector>
#include <QScreen>
#include <QListView>
#include <QWindow>
#include <QRandomGenerator>

#include "login_dialog/login_dialog.h"
#include "mainwindow/mainwindow.h"
#include "mainwindow_addpatient/mainwindow_addpatient.h"
#include "mainwindow_searchpatient/mainwindow_searchpatient.h"
#include "mainwindow_playbackdata/mainwindow_playbackdata.h"
#include "mainwindow/serial_ui/mainwindow_serial.h"
#include "mainwindow/bottom_menu/mainwindow_bottom_menu.h"

// === 前向声明（推荐，减少编译依赖）===
namespace MyApp::UI::login_dialog {
    class login_dialog;
}
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
    class mainwindow_bottom_menu;
}
namespace MyApp::UI::mainwindow_addpatient {
    class mainwindow_addpatient;
}
namespace MyApp::UI::mainwindow_searchpatient {
    class mainwindow_searchpatient;
}
namespace MyApp::UI::mainwindow_playbackdata {
    class mainwindow_playbackdata;
}
// namespace MyApp::UI {
//     class MainWindowPrintPDF;
// }

namespace MyApp::UI {
    class UiManager final : public QObject
    {
        Q_OBJECT

    public:
        explicit UiManager(QObject* parent = nullptr);
        ~UiManager() override;

        // ===== 全局状态 =====
        // 使用者相关信息
        QString account;
        QString usertype;
        int user_id;
        // 屏幕长宽
        int screen_width;
        int screen_height;

        // 各路径地址
        QString baseDir_AppData;
        QString baseDir_userConfig;
        QString baseDir_Cache;
        QString baseDir_Temp;

        // 日志文件
        QFile logFile;
        QTextStream logStream;

        // 文件位置初始化
        bool init_filesDir();
        // 日志初始化
        bool init_logFile();
        // 日志书写
        std::string write_logFile(uint8_t infoType, std::string message);

        // ===== 窗口调度 =====
        void show_login_dialog();
        void show_mainwindow(const QString& accountInfo = {}, const QString& usertypeInfo = 0, int useridInfo = 0);
        void exit_mainwindow();

        void show_mainwindow_addpatient(const QString& accountOrId = {});
        void show_mainwindow_searchpatient(const QString& accountOrId = {});
        void show_mainwindow_playback_data(
            const std::string& p_name,
            const std::string& p_id,
            const std::string& timestampInfo = {}
        );

    public:
        // ===== 窗口实例 =====
        QPointer<login_dialog::login_dialog>                            page_login_dialog;
        QPointer<mainwindow::mainwindow>                                page_mainwindow;
        QPointer<mainwindow_addpatient::mainwindow_addpatient>          page_mainwindow_addpatient;
        QPointer<mainwindow_searchpatient::mainwindow_searchpatient>    page_mainwindow_searchpatient;
        QPointer<mainwindow_playbackdata::mainwindow_playbackdata>      page_mainwindow_playbackdata;
        // QPointer<MainWindowPrintPDF>                                    pagePrintPDF;


        void loadConfig();
        void ensureDirectories();
    };

} // namespace MyApp::UI

#endif // UI_CLASSES_H