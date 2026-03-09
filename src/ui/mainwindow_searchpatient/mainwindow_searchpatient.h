//
// Created by Administrator on 26-1-7.
//

#ifndef MAINWINDOW_SEARCHPATIENT_H
#define MAINWINDOW_SEARCHPATIENT_H

#include <QWidget>
#include <QHeaderView>
#include <QStandardItemModel>
#include "../manager_of_ui.h"
#include "../../tools/diy_database/patient_database.h"

namespace MyApp::UI{ class UiManager; }

namespace MyApp::UI::mainwindow_searchpatient {
    QT_BEGIN_NAMESPACE
    namespace Ui { class mainwindow_searchpatient; }
    QT_END_NAMESPACE

    class mainwindow_searchpatient : public QWidget {
        Q_OBJECT

    public:
        explicit mainwindow_searchpatient(QWidget *parent = nullptr, UiManager *ui_manager = nullptr);
        ~mainwindow_searchpatient() override;

        void set_accountOrId(const QString& accountOrId) const;
        void check_Info();

    protected:
        bool eventFilter(QObject *obj, QEvent *event) override;

    private slots:
        void on_BTN_SearchRecord_clicked();

        void on_TBView_PatientRecord_doubleClicked(const QModelIndex &index);

    private:
        void resize_table_columns();

        Ui::mainwindow_searchpatient *ui;
        UiManager *ui_manager;

        QStandardItemModel *TB_model;
        QHeaderView *TB_header;

        PatientDatabase *pd = nullptr;  // 用于检查用户是否存在
        EachPatientDatabase *epd = nullptr;  // 用于返回数据表

        std::string p_name = "";
        std::string p_id = "";
    };
} // MyApp::UI::mainwindow_searchpatient

#endif //MAINWINDOW_SEARCHPATIENT_H
