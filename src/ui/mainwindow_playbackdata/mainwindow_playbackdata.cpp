//
// Created by Administrator on 26-1-7.
//

// You may need to build the project (run Qt uic code generator) to get "ui_mainwindow_playbackdata.h" resolved

#include "mainwindow_playbackdata.h"
#include "ui_mainwindow_playbackdata.h"
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace MyApp::UI::mainwindow_playbackdata {
    mainwindow_playbackdata::mainwindow_playbackdata(
        QWidget *parent,
        UiManager *ui_manager,
        const std::string &p_name,
        const std::string &p_id,
        const std::string &p_timestamp) :
    QWidget(parent),
    ui(new Ui::mainwindow_playbackdata),
    ui_manager(ui_manager),
    p_name(p_name),
    p_id(p_id),
    p_timestamp(p_timestamp)
    {
        ui->setupUi(this);
        // 初始化lambda语句
        p_dir = [this]() {
            return this->ui_manager->baseDir_AppData.toStdString()
            + "/" + "each_P_data"
            + "/" + this->p_name
            + "_" + this->p_id
            + "/" + this->p_timestamp;
        };

        std::cout << "from playback_data: " << p_name << "_" << p_id << std::endl;

        rbtn_group_plotMode = new QButtonGroup(this);
        rbtn_group_plotMode->setExclusive(true);
        rbtn_group_plotMode->addButton(ui->RBTN_Mode_0, 0);
        rbtn_group_plotMode->addButton(ui->RBTN_Mode_1, 1);
        rbtn_group_plotMode->addButton(ui->RBTN_Mode_2, 2);
        rbtn_group_plotMode->addButton(ui->RBTN_Mode_3, 3);
        rbtn_group_plotMode->addButton(ui->RBTN_Mode_4, 4);
        connect(rbtn_group_plotMode, &QButtonGroup::buttonToggled, this, &mainwindow_playbackdata::rbtn_group_plotMode_toggled);
        ui->RBTN_Mode_1->setChecked(true);

        ckbox_group_plotMode = new QButtonGroup(this);
        ckbox_group_plotMode->setExclusive(false);

        pd = new PatientDatabase(ui_manager->baseDir_AppData.toStdString());
        epd = new EachPatientDatabase(ui_manager->baseDir_AppData.toStdString());

        TB_model = new QStandardItemModel(this);
        ui->TBView_PatientRecord->setModel(TB_model);
        TB_header = ui->TBView_PatientRecord->horizontalHeader();
        TB_header->setSectionResizeMode(QHeaderView::Interactive);
        ui->TBView_PatientRecord->viewport()->installEventFilter(this);
        ui->TBView_PatientRecord->installEventFilter(this);

        ui->Edit_Patient_AccountOrID->setText(p_id.c_str());
        ui->Edit_Patient_Timestamp->setText(p_timestamp.c_str());

        on_BTN_DoSearch_clicked();
        std::cout<<"abc cba"<<std::endl;
    }

    mainwindow_playbackdata::~mainwindow_playbackdata() {
        delete ui;
    }

    bool mainwindow_playbackdata::event(QEvent *event) {
#if defined(Q_OS_WIN)
        if (event->type() == QEvent::WindowStateChange) {
            if (isFullScreen()) {
                for (const QWindow *w : QGuiApplication::allWindows()) {
                    if (!w)
                        continue;
                    if (w->surfaceType() == QWindow::OpenGLSurface) {
                        std::cout<<"opengl"<<std::endl;
                        const auto h = reinterpret_cast<HWND>(w->winId());
                        const LONG_PTR style = GetWindowLongPtr(h, GWL_STYLE);
                        SetWindowLongPtr(h, GWL_STYLE, style | WS_BORDER);
                    }
                }
            }
        }
#endif
        return QWidget::event(event);
    }

    bool mainwindow_playbackdata::eventFilter(QObject *obj, QEvent *event) {
        if (obj == ui->TBView_PatientRecord->viewport() && event->type() == QEvent::Resize) {
            resize_table_columns();
            return false;
        }
        return QWidget::eventFilter(obj, event);
    }


    void mainwindow_playbackdata::on_BTN_DoSearch_clicked() {
        QVector<QStringList> infos;

        const std::string NameOrId = ui->Edit_Patient_AccountOrID->text().toStdString();
        p_timestamp = ui->Edit_Patient_Timestamp->text().toStdString();

        // 检查id或者name是否有效
        if (NameOrId.empty()) {
            ui->Label_Status->setText("错误：请填入有效信息");
            return;
        }
        const bool isId = std::ranges::all_of(NameOrId, ::isdigit);
        const bool isValued = pd->searchPatient(NameOrId, p_name, p_id);
        if (isId && !isValued) {
            ui->Label_Status->setText(QStringLiteral("错误：ID信息有误"));
            return;
        }
        if (!isId && !isValued) {
            ui->Label_Status->setText(QStringLiteral("错误：姓名信息有误或存在重复"));
            return;
        }

        epd->setANDget_info(p_name, p_id, infos);

        // 确认timestamp是否存在
        if (p_timestamp.empty()) {
            is_doubleClicked_with_timestamp = false;

            TB_model->clear();
            TB_model->setHorizontalHeaderLabels({"测试ID", "起始时间", "持续时间"});
            for (const auto& row : infos) {
                QList<QStandardItem*> items;
                for (const auto& col : row) {
                    items << new QStandardItem(col);
                }
                TB_model->appendRow(items);
            }
            return;
        } else if (!std::ranges::all_of(p_timestamp, ::isdigit) && !p_timestamp.empty()) {
            ui->Label_Status->setText(QStringLiteral("输入日期不合法"));
            return;
        } else if (std::ranges::all_of(p_timestamp, ::isdigit) && !p_timestamp.empty()) {
            bool found = std::ranges::any_of(
                infos,
                [&](const QStringList &row) {
                    return row.size() > 1 && row.at(1) == QString::fromStdString(p_timestamp);
                }
            );
            if (!found) {
                ui->Label_Status->setText(QStringLiteral("无该日期数据"));
                return;
            } else {
                is_doubleClicked_with_timestamp = true;
                // 设置表格 + 绘图
                // 读取数据 (stage.csv 先设置表格的内容，触发一次doubleClicked表格实现初始绘图的操作)
                // 先清除
                TB_model->clear();
                TB_model->setHorizontalHeaderLabels({"STAGE", "Ex1", "Ex2"});
                // 尝试读取（可能不存在）
                //todo: 新建一个函数
                std::cout << "要执行绘图操作" <<std::endl;
                // check_before_draw();

            }
        }

    }
    void mainwindow_playbackdata::on_BTN_Exit_clicked() {
        const auto btn = QMessageBox::question(
            this,
            tr("确认退出"),
             tr("是否确定要退出应用？未保存的数据将丢失！"),
             QMessageBox::Yes | QMessageBox::No,
             QMessageBox::No);
        std::cout<< btn << std::endl;
        if (btn == QMessageBox::Yes) {
            this->close();
        }
    }
    void mainwindow_playbackdata::on_BTN_Plot_clicked() {
        std::cout << "BTN_Plot clicked." << std::endl;
    }

    void mainwindow_playbackdata::rbtn_group_plotMode_toggled(QAbstractButton *button, const bool checked) {
        if (!checked) return;  // rbtn 反转触发两次 -> 其中一个0->1; 另一个1->0 要捕获的是0->1的那个

        const auto id = rbtn_group_plotMode->id(button);
        std::cout<<"RBTN_ID: "<<id<<std::endl;

    }

    void mainwindow_playbackdata::setInfo(
        const std::string &p_name,
        const std::string &p_id,
        const std::string &p_timestamp) {
        this->p_name = p_name;
        this->p_id = p_id;
        this->p_timestamp = p_timestamp;
        ui->Edit_Patient_AccountOrID->setText(p_id.c_str());
        ui->Edit_Patient_Timestamp->setText(p_timestamp.c_str());
    }

    void mainwindow_playbackdata::check_before_draw() {
        std::cout << "this: " << std::endl;
        std::cout << "p_name: " << p_name << std::endl;
        std::cout << "p_id: " << p_id << std::endl;
        std::cout << "p_timestamp: " << p_timestamp << std::endl;
        std::cout << "p_dir:" << p_dir() << std::endl;
        std::cout << "*epd: " << std::endl;
        std::cout << "name:" << epd->name << std::endl;
        std::cout << "id:" << epd->id << std::endl;
    }

    void mainwindow_playbackdata::initPlotPlayBack() {
        m_playback_engine = new PlaybackEngine(this);

    }

    void mainwindow_playbackdata::on_TBView_PatientRecord_doubleClicked(const QModelIndex &index) {
        const auto _row = index.row();
        const auto _col = index.column();
        std::cout<<"Row="<<_row<<" Col="<<_col<<std::endl;
        // 先是双击时有timestamp，即绘制的stage了，执行绘制代码
        if (is_doubleClicked_with_timestamp) {
        } else {
            is_doubleClicked_with_timestamp = true;
            ui->Edit_Patient_Timestamp->setText(TB_model->index(_row, 1).data().toString());
            on_BTN_DoSearch_clicked();
        }
    }

    void mainwindow_playbackdata::resize_table_columns() {
        const int totalWidth = ui->TBView_PatientRecord->viewport()->width();
        QList<double> ratios {0.3, 0.4, 0.3};

        auto* header = ui->TBView_PatientRecord->horizontalHeader();
        for (int i = 0; i < ratios.size(); ++i) {
            header->resizeSection(i, static_cast<int>(totalWidth * ratios[i]));
        }
    }





} // MyApp::UI::mainwindow_playbackdata
