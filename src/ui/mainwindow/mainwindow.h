//
// Created by Administrator on 26-1-7.
//

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <iostream>
#include <QWidget>
#include <QThread>
#include <QElapsedTimer>
#include <QTimer>
#include "../manager_of_ui.h"

#include "../../tools/diy_serial/diy_serial.h"
#include "../../tools/diy_oscilloscope-plot/diy_oscilloscope-plot.h"
#include "../../tools/diy_bl/diy_bl.h"
#include "../../tools/diy_start-test/diy_start-test.h"
#include "../../tools/diy_ui_toast-notification/diy_ui_toast-notification.h"


#include "bl_ui/mainwindow_bl.h"

namespace MyApp::UI{ class UiManager; }

class StartTest;
class PatientDatabase;
class EachPatientDatabase;
struct DataFrame;

namespace MyApp::UI::mainwindow_serial {
    class mainwindow_serial;
}
namespace MyApp::UI::mainwindow_bl {
    class mainwindow_bl;
}
namespace MyApp::UI::mainwindow_bottom_menu {
    class mainwindow_bottom_menu;
}

namespace MyApp::UI::mainwindow {
    QT_BEGIN_NAMESPACE
    namespace Ui { class mainwindow; }
    QT_END_NAMESPACE

    class mainwindow : public QWidget {
        Q_OBJECT

        friend class mainwindow_bottom_menu::mainwindow_bottom_menu;

    signals:
        void startSerial(const QString& portName, qint32 baudrate, QSerialPort::DataBits dataBits, QSerialPort::StopBits stopBits, QSerialPort::Parity parity);
        void writeSerial(const QString& data);
        void stopSerial();

    public:
        explicit mainwindow(QWidget *parent = nullptr, UiManager *ui_manager = nullptr);
        ~mainwindow() override;

        QThread* serial_thread;

        bool isSerialConnected = false;
        bool isBLConnected = false;
        bool isPatientConnected = false;


        StartTest *st;
        bool isTesting = false;

        SerialManager *serialMgr;


        BluetoothManager *blMgr;
        QList<QBluetoothDeviceInfo> BLscannedDevices;

        // using BleDeviceInfo = DiyBlWinRt::BleDeviceInfo;
        // DiyBlWinRt::BluetoothManager *blMgr;
        //
        // QList<BleDeviceInfo> BLscannedDevices;


        QPointer<mainwindow_bottom_menu::mainwindow_bottom_menu> bottom_menu = nullptr;

    protected:
        bool event(QEvent *event) override;
        void closeEvent(QCloseEvent *event) override;
        void resizeEvent(QResizeEvent *event) override;

    private slots:
        void on_BTN_SerialConnect_clicked();
        void on_BTN_BLConnect_clicked();
        void on_BTN_Patient_StartTest_clicked();
        void on_BTN_Patient_AddANDUpdate_clicked();
        void on_BTN_Patient_SearchRecord_clicked();
        void on_BTN_Test_clicked();
        void on_BTN_EXIT_clicked();

        // void on_RBTN_BL_clicked();
        // void on_RBTN_Serial_clicked();

        void onBLDevicesFound(const QList<QBluetoothDeviceInfo> &devices);
        // void onBLDevicesFound(const QList<BleDeviceInfo> &devices);
        void onBLStatus(const QString &msg);
        void set_isBLConnected(bool _info);
        void onSerialConnected();
        void onSerialDisconnected();

        void test_esp32_data_draw(QList<QVector<double>> &newData);

        // void handleSerialData(const QByteArray& data);

    private:
        Ui::mainwindow *ui;
        UiManager *ui_manager;

        enum class MonitorState {
            Waiting,
            Ready,
            Recording,
            Warning
        };
        MonitorState monitorState = MonitorState::Waiting;
        QTimer monitorUiTimer;
        QElapsedTimer monitorRecordingElapsed;
        QElapsedTimer monitorBatchElapsed;
        QElapsedTimer monitorLastDataElapsed;
        double monitorSmoothedRate = 0.0;
        bool monitorHardWarning = false;

        void initializeMonitorUi();
        void setMonitorState(MonitorState state, const QString &detail = {});
        void updateMonitorDeviceState();
        void updateMonitorSignalData(const QList<QVector<double>> &channels);
        void updateMonitorStorage(quint64 rows, const QString &filePath);
        static QString formatMonitorDuration(qint64 seconds);

        QButtonGroup *rbtnGroup_connection;

        // QPointer<mainwindow_serial::mainwindow_serial> window_serialSet = nullptr;
        // QPointer<mainwindow_bl::mainwindow_bl> window_blSet = nullptr;

        // 数据库
        PatientDatabase *pd;  // TODO: 没有固定的p_name和p_id使用同一个即可 后面更改 优化内存使用
        EachPatientDatabase *epd;

        // 开始测试类

        // 主界面绘图
        OscilloscopeEngine *m_engine;
        void initPlots();
        void deinitPlots();
        QVector<OscilloscopePlot*> m_plots;

        void print_serial_data(const QString &data);
        void save_test_epd(std::string& p_name, std::string& p_id, std::string& start_time, std::string& duration_time);

    private slots:
        void test_generate_data();
    private:
        QTimer test_timer;
        int test_writeIndex = 0;
        float test_phase = 0.0f;

    };
} // MyApp::UI::mainwindow

#endif //MAINWINDOW_H
