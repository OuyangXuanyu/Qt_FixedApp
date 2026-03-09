//
// Created by Administrator on 26-1-7.
//

#ifndef MAINWINDOW_ADDPATIENT_H
#define MAINWINDOW_ADDPATIENT_H

#include <QWidget>

#include "../manager_of_ui.h"
#include "../../tools/diy_database/patient_database.h"

namespace MyApp::UI{ class UiManager; }

namespace MyApp::UI::mainwindow_addpatient {
    QT_BEGIN_NAMESPACE
    namespace Ui { class mainwindow_addpatient; }
    QT_END_NAMESPACE

    class mainwindow_addpatient : public QWidget {
        Q_OBJECT

    public:
        explicit mainwindow_addpatient(QWidget *parent = nullptr, UiManager *ui_manager = nullptr);
        ~mainwindow_addpatient() override;

        void set_accountOrId(const QString& accountOrId);
        void check_Info();

    protected:
        void closeEvent(QCloseEvent *event) override;
    private slots:
        void on_BTN_CheckInfo_clicked();
        void on_BTN_EnterAddPatient_clicked();
        void on_BTN_ExitAddPatient_clicked();

        void on_CKBox_Invasive_ICP_toggled(bool checked);

    private:
        Ui::mainwindow_addpatient *ui;
        UiManager *ui_manager;

        // 病人数据库
        PatientDatabase *pd;

    };
} // MyApp::UI::mainwindow_addpatient

#endif //MAINWINDOW_ADDPATIENT_H
