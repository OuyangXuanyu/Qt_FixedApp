//
// Created by Administrator on 26-1-7.
//

// You may need to build the project (run Qt uic code generator) to get "ui_mainwindow_addpatient.h" resolved

#include "mainwindow_addpatient.h"
#include "ui_mainwindow_addpatient.h"

using Row = std::map<std::string, std::string>;

namespace MyApp::UI::mainwindow_addpatient {
    mainwindow_addpatient::mainwindow_addpatient(QWidget *parent, UiManager *ui_manager) : QWidget(parent), ui(new Ui::mainwindow_addpatient), ui_manager(ui_manager) {
        ui->setupUi(this);
        pd = new PatientDatabase(ui_manager->baseDir_AppData.toStdString());
        connect(pd, &PatientDatabase::createNewTable, this, &mainwindow_addpatient::createNewTable);
        this->resize(ui_manager->screen_width / 2, ui_manager->screen_height / 1.5);

        // connect(ui->CKBox_Invasive_ICP, &QCheckBox::toggled, this, &mainwindow_addpatient::on_CKBox_Invasive_ICP_toggled);
    }
    mainwindow_addpatient::~mainwindow_addpatient() {
        std::cout << "~mainwindow_addpatient()" << std::endl;
        delete ui;
    }
    void mainwindow_addpatient::closeEvent(QCloseEvent *event) {
        const auto btn = QMessageBox::question(this, tr("确认退出"),
                                     tr("是否确定要退出添加病人信息？未更改的病人信息将丢失！"),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
        if (btn == QMessageBox::No) {
            event->ignore();
        } else {
            event->accept();
        }
    }
    void mainwindow_addpatient::on_BTN_CheckInfo_clicked() {
        const auto id = ui->Edit_ID_diy->text();
        const auto name = ui->Edit_Name->text();
        std::vector<Row> search_result;

        bool found = false;

        if (id.isEmpty() && name.isEmpty()) {
            QMessageBox::warning(this, "错误", "ID和Name不能同时为空");
        } else if (!id.isEmpty()) {
            found = pd->searchPatient(id.toStdString(), search_result);
            ui->Label_Status->setText(QStringLiteral("正在使用ID查询"));
        } else if (id.isEmpty() && !name.isEmpty()) {
            found = pd->searchPatient(name.toStdString(), search_result);
            ui->Label_Status->setText(QStringLiteral("正在使用Name查询"));
        }
        if (!found) {
            QMessageBox::warning(this, "错误", "未找到相关病人信息");
        }

        for (const auto& row : search_result) {
            ui->Edit_Name->setText(QString::fromStdString(row.at("patient_name")));
            ui->Edit_ID_diy->setText(QString::fromStdString(row.at("patient_id_diy")));
            ui->CBBox_Gender->setCurrentText(QString::fromStdString(row.at("gender")));
            ui->SPBox_Age->setValue(std::stoi(row.at("age")));
            ui->SPBox_Height_cm->setValue(std::stoi(row.at("height_cm")));
            ui->SPBox_Weight_kg->setValue(std::stoi(row.at("weight_kg")));
            ui->SPBox_BPsys_mmHg->setValue(std::stoi(row.at("blood_pressure_sys")));
            ui->SPBox_BPdia_mmHg->setValue(std::stoi(row.at("blood_pressure_dia")));
            if (row.at("clinical_diagnosis") == "-1") {
                ui->Edit_CDI_Info->setText("");
            } else {
                ui->Edit_CDI_Info->setText(QString::fromStdString(row.at("clinical_diagnosis")));
            }

            if (std::stoi(row.at("invasive_icp")) < 0) {
                ui->CKBox_Invasive_ICP->setChecked(false);
                ui->SPBox_Invasive_ICP->setEnabled(false);
                ui->SPBox_Invasive_ICP->setValue(0);
            } else {
                ui->CKBox_Invasive_ICP->setChecked(true);
                ui->SPBox_Invasive_ICP->setEnabled(true);
                ui->SPBox_Invasive_ICP->setValue(std::stoi(row.at("invasive_icp")));
            }

        }
    }
    void mainwindow_addpatient::on_BTN_EnterAddPatient_clicked() {
        // 注意：double 与 int {SPBox->value()}
        std::string CDI_Info;
        if (ui->Edit_CDI_Info->toPlainText().toStdString() == "") {
            CDI_Info = "-1";
        } else {
            CDI_Info = ui->Edit_CDI_Info->toPlainText().toStdString();
        }
        int ICP_value;
        if (ui->SPBox_Invasive_ICP->isEnabled()) {
            ICP_value = ui->SPBox_Invasive_ICP->value();
        } else {
            ICP_value = -1;
        }

        bool test = pd->addPatient(
            std::stoi(ui->Edit_ID_diy->text().toStdString()),
            ui->Edit_Name->text().toStdString(),
            ui->CBBox_Gender->currentText().toStdString(),
            ui->SPBox_Age->value(),
            ui->SPBox_Height_cm->value(),
            ui->SPBox_Weight_kg->value(),
            ui->SPBox_BPsys_mmHg->value(),
            ui->SPBox_BPdia_mmHg->value(),
            CDI_Info,
            ICP_value,
            ui_manager->account.toStdString(),
            ui_manager->user_id
        );
        std::cout<<"add_patient test: "<<test<<std::endl;
    }
    void mainwindow_addpatient::on_BTN_ExitAddPatient_clicked() {
        this->close();
    }
    void mainwindow_addpatient::on_CKBox_Invasive_ICP_toggled(const bool checked) {
        ui->SPBox_Invasive_ICP->setEnabled(checked);
    }

    void mainwindow_addpatient::set_accountOrId(const QString& accountOrId) {
        bool isId;
        accountOrId.toLong(&isId);
        if (isId) {
            ui->Edit_ID_diy->setText(accountOrId);
            ui->Edit_Name->setText("");
        } else {
            ui->Edit_Name->setText(accountOrId);
            ui->Edit_ID_diy->setText("");
        }

    }
    void mainwindow_addpatient::check_Info() {
        const auto id = ui->Edit_ID_diy->text();
        const auto name = ui->Edit_Name->text();
        if (id.isEmpty() && name.isEmpty()) return;
        on_BTN_CheckInfo_clicked();
    }
    void mainwindow_addpatient::createNewTable(int patient_id_diy) {
        EachPatientDatabase *epd = new EachPatientDatabase(ui_manager->baseDir_AppData.toStdString());
        const bool is_success = epd->createNewTable(patient_id_diy);
        if (!is_success) {
            QMessageBox::warning(this, "错误", "创建表失败");
        }
    }



} // MyApp::UI::mainwindow_addpatient
