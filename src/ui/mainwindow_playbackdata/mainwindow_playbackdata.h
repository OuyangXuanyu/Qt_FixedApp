//
// Created by Administrator on 26-1-7.
//

#ifndef MAINWINDOW_PLAYBACKDATA_H
#define MAINWINDOW_PLAYBACKDATA_H

#include <QWidget>
#include <QHeaderView>
#include <QStandardItemModel>
#include "../manager_of_ui.h"

#include "../../tools/diy_playback-plot/diy_playback-plot.h"

namespace MyApp::UI{ class UiManager; }

namespace MyApp::UI::mainwindow_playbackdata {
    QT_BEGIN_NAMESPACE
    namespace Ui { class mainwindow_playbackdata; }
    QT_END_NAMESPACE

    class mainwindow_playbackdata : public QWidget {
    Q_OBJECT

    public:
        explicit mainwindow_playbackdata(QWidget *parent = nullptr, UiManager *ui_manager = nullptr, const std::string &p_name = {}, const std::string &p_id = {}, const std::string &p_timestamp = {});
        ~mainwindow_playbackdata() override;

        void setInfo(
            const std::string &p_name,
            const std::string &p_id,
            const std::string &p_timestamp);
        void check_before_draw();

    protected:
        bool event(QEvent *event) override;
        bool eventFilter(QObject *obj, QEvent *event) override;

    private slots:
        void on_BTN_DoSearch_clicked();
        void on_BTN_Plot_clicked();
        void on_BTN_Exit_clicked();

        void rbtn_group_plotMode_toggled(QAbstractButton *button, bool checked);

        void on_TBView_PatientRecord_doubleClicked(const QModelIndex &index);

    private:
        // Basic ex
        Ui::mainwindow_playbackdata *ui;
        UiManager *ui_manager;

        // Database ex
        PatientDatabase *pd;
        EachPatientDatabase *epd;

        // TableView ex
        QStandardItemModel *TB_model;
        QHeaderView *TB_header;

        // Patient Infos
        std::string p_name{};
        std::string p_id{};
        std::string p_timestamp{};

        std::function<std::string()> p_dir;

        // Plot Operator
        QButtonGroup *rbtn_group_plotMode;
        QButtonGroup *ckbox_group_plotMode;

        QVector<int> configPlotModeList{
            0b00000000,
            0b11001010,
            0b11110010,
            0b00001110,
            0b10011000
        };

        // TBView resize col
        void resize_table_columns();

        // Plot init
        PlaybackEngine *m_engine;
        QVector<PlaybackPlot*> m_plots;
        void initPlotPlayBack();
        void deinitPlotPlayback();

        // bool check
        bool is_doubleClicked_with_timestamp = false;

    };
} // MyApp::UI::mainwindow_playbackdata

#endif //MAINWINDOW_PLAYBACKDATA_H
