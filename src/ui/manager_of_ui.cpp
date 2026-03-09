//
// Created by Administrator on 26-1-7.
//

#include "manager_of_ui.h"

#include <iostream>
#include <QDir>
#include <QDateTime>

namespace MyApp::UI{
    // 构造函数
    UiManager::UiManager(QObject* parent) : QObject(parent)
    {
        // 初始化环境
        if (!this->init_filesDir())
            QMessageBox::warning(nullptr,"警告", "应用程序文件目录初始化创建失败", QMessageBox::Ok);
        if (!this->init_logFile())
            QMessageBox::warning(nullptr,"警告", "日志文件初始化失败", QMessageBox::Ok);

        // 初始化成员变量
        page_login_dialog = new login_dialog::login_dialog(nullptr, baseDir_AppData.toStdString());
        page_mainwindow = nullptr;
        page_mainwindow_addpatient = nullptr;
        page_mainwindow_searchpatient = nullptr;
        page_mainwindow_playbackdata = nullptr;

        // TODO
        // pagePrintPDF = nullptr;

        // // TODO 加载配置
        // loadConfig();

        logStream.setDevice(&logFile);
        logStream.setEncoding(QStringConverter::Utf8);
        logStream << this->write_logFile(0, "state the app").c_str() << Qt::endl;
    }

    // 析构函数
    UiManager::~UiManager() {
        // 清理所有窗口指针
        if (page_login_dialog) {
            page_login_dialog->close();
            page_login_dialog->deleteLater();
        }
        if (page_mainwindow) {
            page_mainwindow->close();
            page_mainwindow->deleteLater();
        }
        if (page_mainwindow_addpatient) {
            page_mainwindow_addpatient->close();
            page_mainwindow_addpatient->deleteLater();
        }
        if (page_mainwindow_searchpatient) {
            page_mainwindow_searchpatient->close();
            page_mainwindow_searchpatient->deleteLater();
        }
        if (page_mainwindow_playbackdata) {
            page_mainwindow_playbackdata->close();
            page_mainwindow_playbackdata->deleteLater();
        }
        // if printpdf
        // 关闭日志文件
        if (logFile.isOpen()) {
            // 写入退出程序的日志行
            logFile.close();
        }
        std::cerr << "exit this app: 执行了管理类的析构函数" << std::endl;
    }

    // 日志初始化 & 存放位置初始化
    bool UiManager::init_filesDir() {
        bool ok = true;
        const QDir dir_make;

        this->baseDir_AppData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);  // 应用数据
        ok &= dir_make.mkpath(baseDir_AppData);

        this->baseDir_userConfig = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);  // 用户配置
        ok &= dir_make.mkpath(baseDir_userConfig);

        this->baseDir_Cache = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);  // 缓存文件
        ok &= dir_make.mkpath(baseDir_Cache);

        this->baseDir_Temp = QStandardPaths::writableLocation(QStandardPaths::TempLocation);  // 临时文件
        ok &= dir_make.mkpath(baseDir_Temp);

        return ok;
    }
    bool UiManager::init_logFile() {
        const QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        const QDir dir(baseDir);

        if (!dir.mkpath("log")) {
            std::cerr << "Failed to create log dir:" << std::endl;
            return false;
        }
        const QString logFilePath = dir.filePath("log/" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".log");
        logFile.setFileName(logFilePath);
        if (!logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        {
            qWarning() << "Failed to open log file:" << logFilePath;
            return false;
        }
        return true;
    }
    // 日志写实现
    std::string UiManager::write_logFile(const uint8_t infoType, std::string message) {
        /*
         * 使用方式：
         * logStream << this->write_logFile(0-2, "info").c_str() << Qt::endl;
         */
        std::string _infoType;
        switch (infoType) {
            case 0: _infoType = "INFO"; break;
            case 1: _infoType = "WARNING"; break;
            case 2: _infoType = "ERROR"; break;
            default: _infoType = "UNKNOWN"; break;
        }
        return std::format("[{0}] -- {1} : {2}", _infoType, QDateTime::currentDateTime().toString("yyyy/MM/dd HH:mm:ss").toStdString(), message);
    }

    void UiManager::show_login_dialog() {
        std::cout << "Login Dialog" << std::endl;
        // 登陆成功 -> 调用删除 + show_main
        connect(page_login_dialog, &login_dialog::login_dialog::loginSuccess, this, &UiManager::show_mainwindow);
        page_login_dialog->show();
    }

    void UiManager::show_mainwindow(const QString& accountInfo, const QString& usertypeInfo, const int useridInfo) {
        this->account = accountInfo;
        this->usertype = usertypeInfo;
        this->user_id = useridInfo;

        if (!page_mainwindow) {
            page_mainwindow = new mainwindow::mainwindow(nullptr, this);
        }
        if (page_login_dialog) {
            page_login_dialog->close();
            page_login_dialog->deleteLater();
        }

        // 和PyQt同样的问题 真操蛋
        // page_mainwindow->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
        page_mainwindow->setAttribute(Qt::WA_TranslucentBackground, false);
        const QScreen *screen = page_mainwindow->screen();
        page_mainwindow->setGeometry(screen->geometry());
        page_mainwindow->showFullScreen();

        screen_width = page_mainwindow->width();
        screen_height = page_mainwindow->height();
    }
    void UiManager::show_mainwindow_addpatient(const QString& accountOrId) {
        if (!page_mainwindow_addpatient) {
            page_mainwindow_addpatient = new mainwindow_addpatient::mainwindow_addpatient(nullptr, this);
            page_mainwindow_addpatient->show();
        } else {
            page_mainwindow_addpatient->showNormal();
            page_mainwindow_addpatient->raise();
            page_mainwindow_addpatient->activateWindow();
        }
        page_mainwindow_addpatient->set_accountOrId(accountOrId);
        page_mainwindow_addpatient->check_Info();

    }
    void UiManager::show_mainwindow_searchpatient(const QString& accountOrId) {
        if (!page_mainwindow_searchpatient) {
            page_mainwindow_searchpatient = new mainwindow_searchpatient::mainwindow_searchpatient(nullptr, this);
            page_mainwindow_searchpatient->show();
        } else {
            page_mainwindow_searchpatient->showNormal();
            page_mainwindow_searchpatient->raise();
            page_mainwindow_searchpatient->activateWindow();
        }
        page_mainwindow_searchpatient->set_accountOrId(accountOrId);
        page_mainwindow_searchpatient->check_Info();
    }
    void UiManager::show_mainwindow_playback_data(const std::string& p_name, const std::string& p_id, const std::string& timestampInfo) {
        if (!page_mainwindow_playbackdata) {
            page_mainwindow_playbackdata = new mainwindow_playbackdata::mainwindow_playbackdata(nullptr, this, p_name, p_id, timestampInfo);
            page_mainwindow_playbackdata->setAttribute(Qt::WA_TranslucentBackground, false);
            const QScreen *screen = page_mainwindow_playbackdata->screen();
            page_mainwindow_playbackdata->setGeometry(screen->geometry());
            page_mainwindow_playbackdata->showFullScreen();
        } else {
            page_mainwindow_playbackdata->showFullScreen();
            page_mainwindow_playbackdata->raise();
            page_mainwindow_playbackdata->activateWindow();
            page_mainwindow_playbackdata->setInfo(p_name, p_id, timestampInfo);
        }
    }
}
