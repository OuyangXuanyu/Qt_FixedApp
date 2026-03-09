//
// Created by Administrator on 26-1-7.
//

// You may need to build the project (run Qt uic code generator) to get "ui_login_dialog.h" resolved

#include "login_dialog.h"
#include "ui_login_dialog.h"
#include <iostream>
#include <QButtonGroup>

namespace MyApp::UI::login_dialog {
    login_dialog::login_dialog(QWidget *parent, std::string appPath) : QDialog(parent), appPath(std::move(appPath)), ui(new Ui::login_dialog) {
        ui->setupUi(this);
        ud = new UserInfoDatabase(this->appPath);
        // 三个RBTN构建一个Group
        rbtnGroup = new QButtonGroup(this);
        rbtnGroup->addButton(ui->RBTN_Doctor, 0);   // 第二个参数是 id
        rbtnGroup->addButton(ui->RBTN_Nurse, 1);
        rbtnGroup->addButton(ui->RBTN_Debugger, 2);

        initUi();
        initConnect();
    }

    login_dialog::~login_dialog() {
        // 奇怪: 在进入main窗口时会执行此析构函数，但是在login界面直接退出不会执行
        printf("login_dialog::~login_dialog()\n");
        delete ui;
    }
    void login_dialog::on_BTN_Login_clicked() {
        accountInfo = ui->Edit_Account->text().trimmed(); // 去除首尾空格
        passwordInfo = ui->Edit_Password->text();
        usertype = rbtnGroup->checkedId();

        if (accountInfo.isEmpty() || passwordInfo.isEmpty()) {
            QMessageBox::warning(this, "错误", "用户名或密码不能为空");
            return;
        }

        // // TODO
        // printf("%s\n", accountInfo.toStdString().c_str());
        // printf("%s\n", passwordInfo.toStdString().c_str());

        // TODO 验证账户和密码是否匹配
        const QString usertype_str = usertype_to_string(usertype);
        const int ok = ud->login(accountInfo, passwordInfo, usertype_str);
        // ok存的是user的id信息
        std::cout << ok << std::endl;
        if (ok!=0) {
            emit loginSuccess(accountInfo, usertype_str, ok);
        }else {
            QMessageBox::warning(this, "错误", "用户名与密码不匹配，请检查重试！");
        }
    }



    void login_dialog::initUi() {
        ui->RBTN_Debugger->setChecked(true);

        // QString baseDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);  // 应用数据
        // std::cout << baseDir.toStdString() << std::endl;
        // baseDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);  // 用户配置
        // std::cout << baseDir.toStdString() << std::endl;
        // baseDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);  // 缓存文件
        // std::cout << baseDir.toStdString() << std::endl;
        // baseDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);  // 临时文件
        // std::cout << baseDir.toStdString() << std::endl;
    }

    void login_dialog::initConnect() {

    }
} // MyApp::UI::login_dialog


