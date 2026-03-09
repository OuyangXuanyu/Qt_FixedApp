//
// Created by Administrator on 26-1-7.
//

// You may need to build the project (run Qt uic code generator) to get "ui_mainwindow_searchpatient.h" resolved

#include "mainwindow_searchpatient.h"
#include "ui_mainwindow_searchpatient.h"

using Row = std::map<std::string, std::string>;

namespace MyApp::UI::mainwindow_searchpatient {
    mainwindow_searchpatient::mainwindow_searchpatient(QWidget *parent, UiManager *ui_manager) : QWidget(parent), ui(new Ui::mainwindow_searchpatient), ui_manager(ui_manager) {
        ui->setupUi(this);

        // 表格模型设置
        TB_model = new QStandardItemModel(this);
        ui->TBView_PatientRecord->setModel(TB_model);

        // 表格表头设置
        TB_header = ui->TBView_PatientRecord->horizontalHeader();
        TB_header->setSectionResizeMode(QHeaderView::Interactive);

        // 事件
        ui->TBView_PatientRecord->viewport()->installEventFilter(this);
        ui->TBView_PatientRecord->installEventFilter(this);

        // 病人数据库初始化
        pd = new PatientDatabase(ui_manager->baseDir_AppData.toStdString());
        epd = new EachPatientDatabase(ui_manager->baseDir_AppData.toStdString());

    }
    mainwindow_searchpatient::~mainwindow_searchpatient() {
        delete ui;
    }

    bool mainwindow_searchpatient::eventFilter(QObject *obj, QEvent *event) {
        if (obj == ui->TBView_PatientRecord->viewport() && event->type() == QEvent::Resize) {
            resize_table_columns();
            return false;
        }
        return QWidget::eventFilter(obj, event);
    }

    void mainwindow_searchpatient::on_BTN_SearchRecord_clicked() {
        // 先删除表格内容
        TB_model->clear();
        TB_model->setHorizontalHeaderLabels({"测试ID", "起始时间", "持续时间"});

        const std::string _accountOrId = ui->Edit_Patient_AccountOrID->text().toStdString();


        if (_accountOrId.empty()) {
            QMessageBox::warning(this, "错误", "请填入信息");
            return;
        }
        const bool isId = std::ranges::all_of(_accountOrId, ::isdigit);

        // todo: 还得改 返回id和name两个信息
        const bool isValued = pd->searchPatient(_accountOrId, p_name, p_id);

        if (isId && !isValued) {
            QMessageBox::warning(this, "错误", "ID信息有误");
            return;
        }
        if (!isId && !isValued) {
            QMessageBox::warning(this, "错误", "姓名信息有误或存在重复姓名用户\n请再确认姓名信息或尝试ID查询");
            return;
        }

        std::cout<<"name: "<<p_name<<std::endl;
        std::cout<<"id: "<<p_id<<std::endl;

        // setANDget_info
        QVector<QStringList> infos;
        epd->setANDget_info(p_name, p_id, infos);

        // todo: 执行表格重绘
        for (const auto& row : infos) {
            QList<QStandardItem*> items;
            for (const auto& col : row) {
                items << new QStandardItem(col);
            }
            TB_model->appendRow(items);
        }
    }

    void mainwindow_searchpatient::on_TBView_PatientRecord_doubleClicked(const QModelIndex &index) {
        const auto _row = index.row();
        const auto _col = index.column();
        std::cout<<"Row="<<_row<<" Col="<<_col<<std::endl;

        const std::string timestamp = TB_model->index(_row, 1).data().toString().toStdString();
        std::cout << timestamp << std::endl;

        ui_manager->show_mainwindow_playback_data(p_name, p_id, timestamp);

        this->close();

    }

    void mainwindow_searchpatient::resize_table_columns() {
        const int totalWidth = ui->TBView_PatientRecord->viewport()->width();
        QList<double> ratios {0.3, 0.4, 0.3};

        auto* header = ui->TBView_PatientRecord->horizontalHeader();
        for (int i = 0; i < ratios.size(); ++i) {
            header->resizeSection(i, static_cast<int>(totalWidth * ratios[i]));
        }
    }

    void mainwindow_searchpatient::set_accountOrId(const QString& accountOrId) const {
        ui->Edit_Patient_AccountOrID->setText(accountOrId);
    }
    void mainwindow_searchpatient::check_Info() {
        if (!ui->Edit_Patient_AccountOrID->text().isEmpty()) {
            on_BTN_SearchRecord_clicked();
            return;
        }
        TB_model->clear();
    }

} // MyApp::UI::mainwindow_searchpatient
