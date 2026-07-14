//
// Created by Administrator on 26-1-7.
//

#ifndef LOGIN_DIALOG_H
#define LOGIN_DIALOG_H

#include <QMessageBox>
#include <QButtonGroup>
#include <QStandardPaths>
#include "../../tools/diy_database/user_database.h"

namespace MyApp::UI::login_dialog {
    QT_BEGIN_NAMESPACE
    namespace Ui { class login_dialog; }
    QT_END_NAMESPACE

    class login_dialog : public QDialog {
        Q_OBJECT

    public:
        explicit login_dialog(QWidget *parent = nullptr, std::string appPath = {});
        ~login_dialog() override;

        std::string appPath;

        QString accountInfo;
        QString passwordInfo;
        uint8_t usertype;

    signals:
        void loginSuccess(const QString &accountInfo, const QString &usertypeInfo, int useridInfo);

    private slots:
        void on_BTN_Login_clicked();

    private:
        Ui::login_dialog *ui;

        QButtonGroup *rbtnGroup;

        void initUi();
        void initConnect();

        UserInfoDatabase *ud;

    };
} // MyApp::UI::login_dialog

#endif //LOGIN_DIALOG_H
