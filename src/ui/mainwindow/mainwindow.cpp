//
// Created by Administrator on 26-1-7.
//

// You may need to build the project (run Qt uic code generator) to get "ui_mainwindow.h" resolved
//dev

#include "mainwindow.h"
#include "ui_mainwindow.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

#include "./bottom_menu/ui_mainwindow_bottom_menu.h"

#include "../../tools/diy_database/patient_database.h"
#include "../../tools/diy_start-test/diy_start-test.h"


qint64 getChronoMicrosecondsSinceEpoch();
void checkMyCore();

namespace MyApp::UI::mainwindow {
    mainwindow::mainwindow(QWidget *parent, UiManager *ui_manager) : QWidget(parent), ui(new Ui::mainwindow), ui_manager(std::move(ui_manager)) {
        ui->setupUi(this);

        checkMyCore();

        // 初始化开始测试类
        st = new StartTest(this, ui_manager);

        // 初始化绘图区域
        initPlots();

        bottom_menu = new mainwindow_bottom_menu::mainwindow_bottom_menu(this);
        bottom_menu->setParent(this);
        bottom_menu->raise();
        bottom_menu->updatePosition();


        // 连接方式RBTN
        // rbtnGroup_connection = new QButtonGroup(this);
        // rbtnGroup_connection->addButton(ui->RBTN_BL, 0);
        // rbtnGroup_connection->addButton(ui->RBTN_Serial, 1);

        // 初始化串口Manager
        serial_thread = new QThread(this);
        serialMgr = new SerialManager(nullptr);

        serialMgr->moveToThread(serial_thread);

        connect(this, &mainwindow::startSerial, serialMgr, &SerialManager::open);
        connect(this, &mainwindow::writeSerial, serialMgr, &SerialManager::write);
        connect(this, &mainwindow::stopSerial, serialMgr, &SerialManager::close);

        connect(serialMgr, &SerialManager::connectionLost, this, &mainwindow::onSerialDisconnected);
        connect(serialMgr, &SerialManager::connectionSucceed, this, &mainwindow::onSerialConnected);

        connect(serialMgr, &SerialManager::dataReceived, st->blfp, &BLFrameParser::appendInfo);
        connect(serial_thread, &QThread::finished, serialMgr, &QObject::deleteLater);

        serial_thread->start();


        // 初始化蓝牙Manager
        blMgr = new BluetoothManager(this);

        // 初始化数据库
        pd = new PatientDatabase(ui_manager->baseDir_AppData.toStdString());
        epd = new EachPatientDatabase(ui_manager->baseDir_AppData.toStdString());


        // 同时初始化 蓝牙(+串口)的信号
        connect(blMgr->blScr, &BluetoothScanner::deviceFound, this, &mainwindow::onBLDevicesFound);
        connect(blMgr, &BluetoothManager::statusChanged, this, &mainwindow::onBLStatus);
        connect(blMgr, &BluetoothManager::isConnecting, this, &mainwindow::set_isBLConnected);
        // // 测试用蓝牙通信代码
        // connect(blMgr, &BluetoothManager::dataReceived,
        //     this, [&](const QByteArray &data)
        //     {
        //         // TODO: 这里接受的内容到专门的数据流处理 -> 放到start-test类处理？
        //         qDebug() << data;
        //     }
        // );

        // 上传的数据解析接口
        connect(blMgr, &BluetoothManager::dataReceived, st->blfp, &BLFrameParser::appendInfo, Qt::QueuedConnection);


        // 解析完数据绘图
        connect(st->blfp, &BLFrameParser::test_esp32_data_signal, this, &mainwindow::test_esp32_data_draw);
        // 测试完数据写epd
        connect(st, &StartTest::finished_test_epd, this, &mainwindow::save_test_epd);

        // 错误数据
        connect(st->blfp, &BLFrameParser::data_error, this, [&](const int error_count)
        {
            // TODO: 这里接受的内容到专门的数据流处理 -> 放到start-test类处理？
            qDebug() << error_count;
        }, Qt::QueuedConnection);
        // ui->BTN_Patient_StartTest->setEnabled(false);
        bottom_menu->raise();
        bottom_menu->show(); // 确保它是显示状态
    }
    mainwindow::~mainwindow() {
        if (serial_thread->isRunning()) {
            serial_thread->quit();
            serial_thread->wait();
        }
        std::cout << "~mainwindow()" << std::endl;
        delete ui;

    }
    bool mainwindow::event(QEvent *event) {
        /*
         *处理OpenGLWidget在全屏下出现闪屏效果的BUG
         */
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
                        // break;
                    }
                }
            }
        }
#endif
        return QWidget::event(event);
    }

    void mainwindow::closeEvent(QCloseEvent *event) {
        if (isTesting) {
            QMessageBox::warning(this, "警告", "测试中不允许退出软件！", QMessageBox::StandardButton::Ok);
            event->ignore();
        }
        const auto btn = QMessageBox::question(
            this,
            tr("确认退出"),
             tr("是否确定要退出应用？未保存的数据将丢失！"),
             QMessageBox::Yes | QMessageBox::No,
             QMessageBox::No);
        if (btn != QMessageBox::Yes) {
            event->ignore();          // ② 用户取消，忽略事件
            return;
        }

        // 安全退出蓝牙 or 串口
        // TODO
        deinitPlots();
        ui_manager->logStream<<ui_manager->write_logFile(0, "exit the app").c_str()<<Qt::endl;

        event->accept();
    }

    void mainwindow::resizeEvent(QResizeEvent *event) {
        QWidget::resizeEvent(event); // 必须先调用基类处理

        if (bottom_menu) {
            bottom_menu->updatePosition();
            qDebug() << "1";
        }
    }

    // 串口相关按钮实现
    void mainwindow::on_BTN_SerialConnect_clicked() {
        // if (!isSerialConnected) {
        //     emit startSerial(
        //         ui->CBBox_Serial->currentText(),
        //         3000000,
        //         QSerialPort::Data8,
        //         QSerialPort::OneStop,
        //         QSerialPort::NoParity
        //         );
        // }
        // else {
        //     emit stopSerial();
        //     // isSerialConnected = false;
        //     // ui->BTN_SerialConnect->setText(QStringLiteral("打开串口"));
        //     // ui->RBTN_Serial->setEnabled(true);
        //     // ui->RBTN_BL->setEnabled(true);
        // }

    }

    // 串口取数据（注意返回格式）
    void mainwindow::print_serial_data(const QString &data) {
        std::cout<<"Serial print: "<<data.toStdString()<<std::endl;
    }

    // 蓝牙相关按钮实现
    void mainwindow::on_BTN_BLConnect_clicked(){
        // const int index = ui->CBBox_BL->currentIndex();
        // if (index < 0 || index >= BLscannedDevices.size()) {
        //     std::cout<<"[ERROR] 未选择有效设备"<<std::endl;
        //     return;
        // }
        //
        // const auto &device = BLscannedDevices.at(index);
        // ui->Label_AppName->setText("🔌 正在连接...");
        // std::cout<<"[INFO] 正在连接..."<<std::endl;
        //
        // blMgr->connectToDevice(device);
    }
    void mainwindow::onBLDevicesFound(const QList<QBluetoothDeviceInfo> &devices) {
        BLscannedDevices = devices;
        for (const auto &dev : devices) {
            // QString text = QString("%1 (%2)")
            //     .arg(dev.name())
            //     .arg(dev.address().toString());
            QString text = QString("%1").arg(dev.name());
            bottom_menu->window_blSet->ui->CBBox_Bl->addItem(text);
        }

    }
    void mainwindow::onBLStatus(const QString &msg)
    {
        std::cout<<"onBLStatus: "<< msg.toStdString()<<std::endl;
    }
    void mainwindow::set_isBLConnected(const bool _info) {
        isBLConnected = _info;
        bottom_menu->window_blSet->ui->BTN_BlConnect->setText(_info ? "断开蓝牙" : "连接蓝牙");
    }

    void mainwindow::onSerialConnected() {
        isSerialConnected = true;
        bottom_menu->window_serialSet->ui->BTN_SerialConnect->setText(QStringLiteral("断开串口"));
        bottom_menu->ui->RBTN_Serial->setEnabled(false);
        bottom_menu->ui->RBTN_BL->setEnabled(false);
    }
    void mainwindow::onSerialDisconnected() {
        isSerialConnected = false;
        bottom_menu->window_serialSet->ui->BTN_SerialConnect->setText(QStringLiteral("打开串口"));
        bottom_menu->ui->RBTN_Serial->setEnabled(true);
        bottom_menu->ui->RBTN_BL->setEnabled(true);
    }

    // 开始测试实现唯一入口
    void mainwindow::on_BTN_Patient_StartTest_clicked(){
        if (!isTesting) {
            // step1. 验证id/name是否为有效信息
            const std::string _accountOrId = bottom_menu->ui->Edit_Patient_AccountOrID->text().toStdString();
            std::string _name, _id;
            if (_accountOrId.empty()) {
                QMessageBox::warning(this, "错误", "请填入信息");
                return;
            }
            const bool isId = std::ranges::all_of(_accountOrId, ::isdigit);
            const bool isValued = pd->searchPatient(_accountOrId, _name, _id);
            if (isId && !isValued) {
                QMessageBox::warning(this, "错误", "ID信息有误");
                return;
            }
            if (!isId && !isValued) {
                QMessageBox::warning(this, "错误", "姓名信息有误或存在重复姓名用户\n请再确认姓名信息或尝试ID查询");
                return;
            }

            // step2. 验证蓝牙是否连接成功
            if (!isBLConnected && !isSerialConnected) {
                QMessageBox::warning(this, "错误", "还未连接任何设备，请先确认连接");
                return;
            }
            // step3. 重绘UI
            // todo:
            // step4. 执行start_test类的btn_pressed函数
            if (!st->btn_pressed(_name, _id, true)) {
                return;
            }
            // step5. 绘图引擎start
            m_engine->start();
            // step6. 蓝牙发送"START"指令 / 串口
            if (isBLConnected) blMgr->write(QByteArrayLiteral("START"));
            if (isSerialConnected) serialMgr->write(QStringLiteral("AA169000A6FF"));
            // step7. 修改各个按键的状态
            bottom_menu->ui->BTN_Patient_AddANDUpdate->setEnabled(false);
            bottom_menu->ui->BTN_Patient_StartTest->setText(QStringLiteral("停止测试"));
            isTesting = true;
            QMetaObject::invokeMethod(serialMgr, "setTesting", Qt::QueuedConnection, Q_ARG(bool, isTesting));
            qint64 micros = getChronoMicrosecondsSinceEpoch();
            qDebug() << "Chrono timestamp (μs):" << micros;

        } else {
            isTesting = false;
            QMetaObject::invokeMethod(serialMgr, "setTesting", Qt::QueuedConnection, Q_ARG(bool, isTesting));
            // Step1. 断开signal
            qint64 micros = getChronoMicrosecondsSinceEpoch();
            qDebug() << "Chrono timestamp (μs):" << micros;
            // Step2. st执行断开
            st->btn_pressed("", "", false);
            // Step3.
            bottom_menu->ui->BTN_Patient_StartTest->setText(QStringLiteral("开始测试"));
        }


    }

    // 调用新的窗口
    void mainwindow::on_BTN_Patient_AddANDUpdate_clicked(){
        ui_manager->show_mainwindow_addpatient(bottom_menu->ui->Edit_Patient_AccountOrID->text());
    }
    void mainwindow::on_BTN_Patient_SearchRecord_clicked(){
        ui_manager->show_mainwindow_searchpatient(bottom_menu->ui->Edit_Patient_AccountOrID->text());
    }

    // 测试按钮
    void mainwindow::on_BTN_Test_clicked() {
        // std::cout << "btn was pressed" << std::endl;
        //
        // // // 测试：发送START到蓝牙端
        // // if (isBLConnected) blMgr->write(QByteArrayLiteral("START"));
        // test_timer.setInterval(25);
        // connect(&test_timer, &QTimer::timeout, this, &mainwindow::test_generate_data);
        // test_timer.start();
        // // m_engine->start();
        if (isSerialConnected) {
            // serialMgr->write("73");
            serialMgr->write(QStringLiteral("AA169000A6FF"));
            // bottom_menu->ui->StatusBar_Label->setText(QStringLiteral("发送启动指令成功"));  // todo: danger:
            // ui->BTN_Patient_StartTest->setEnabled(true);
        } else {
            // bottom_menu->ui->StatusBar_Label->setText(QStringLiteral("串口未开启，发送启动指令失败"));  // todo: danger:
        }
    }

    // 退出按钮
    void mainwindow::on_BTN_EXIT_clicked() {
        std::cout << 1231133123 << std::endl;
        this->close();
    }

    // 初始化Plot区域
    void mainwindow::initPlots() {
        m_engine = new OscilloscopeEngine(this);
        m_plots.clear();

        auto attachPlot = [&](QWidget *placeholder) {
            const auto layout = new QVBoxLayout(placeholder);
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);

            const auto plot = new OscilloscopePlot(placeholder);

            // 明确告诉 Qt：这是一个“非 Native”的 OpenGL Widget
            plot->setAttribute(Qt::WA_NativeWindow, false);
            plot->setAttribute(Qt::WA_PaintOnScreen, false);
            // 禁止部分更新，减少 swap-chain 重组
            plot->setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);

            layout->addWidget(plot);

            // DANGER:
            m_plots.push_back(plot);
            m_engine->addPlot(plot);
        };

        attachPlot(ui->Plot_A);
        attachPlot(ui->Plot_B);
        attachPlot(ui->Plot_C);
        attachPlot(ui->Plot_D);
        attachPlot(ui->Plot_E);
        attachPlot(ui->Plot_F);
        attachPlot(ui->Plot_G);
        attachPlot(ui->Plot_H);
        attachPlot(ui->Plot_I);
    }

    void mainwindow::deinitPlots() {
        if (!m_engine && m_plots.empty()) return;

        if (m_engine) {
            m_engine->stop();
        }
        if (m_engine) {
            for (auto* plot : m_plots) {
                m_engine->removePlot(plot);  // 假设引擎有这个方法
            }
        }


        // for (auto* plot : m_plots) {
        //     if (plot) {
        //         // 先从布局中移除，避免野指针
        //         if (plot->parentWidget()) {
        //             auto* layout = plot->parentWidget()->layout();
        //             if (layout) {
        //                 layout->removeWidget(plot);
        //             }
        //             plot->setParent(nullptr);
        //         }
        //
        //         // 删除 plot 调用 ~OscilloscopePlot
        //         delete plot;
        //     }
        // }

        m_plots.clear();

        delete m_engine;
        m_engine = nullptr;
    }

    void mainwindow::test_generate_data() {
        int i = 1;
        for (OscilloscopePlot *plot : m_engine->m_plots) {
            plot->insertData(3.3f * std::sin(2.0f * M_PI * test_writeIndex / 200.0f * (i*=2)));
        }
        // test_phase += 0.1;
        test_writeIndex++;
    }

    // void mainwindow::test_esp32_data_draw(const float *result_array) {
    void mainwindow::test_esp32_data_draw(QList<QVector<double>> &newData) {
        if (newData.isEmpty() || newData[0].isEmpty()) return;

        int channelCount = newData.size(); // 应该是 7
        int pointCount = newData[0].size(); // 应该是 10

        // static int judge_count = 0;
        // 遍历每一个通道
        for (int j = 0; j < channelCount; ++j) {
            if (j >= m_engine->m_plots.size()) break; // 防止越界

            // 直接获取该通道的 QVector 地址并推送到对应的 Plot
            // 如果你的 insertDataBatch 接收 float*，需要注意 double 到 float 的转换
            // 如果 insertDataBatch 已经支持 double，直接传 data() 即可

            const QVector<double> &oneChannel = newData.at(j);

            // 如果你的绘图引擎 insertDataBatch 必须接收 float*
            // 且 newData 是 double，则仍需一个临时转换：
            static QVector<float> tempBuffer;
            tempBuffer.resize(pointCount);
            for(int i = 0; i < pointCount; ++i) {
                tempBuffer[i] = static_cast<float>(oneChannel[i]);
            }

            m_engine->m_plots.at(j)->insertDataBatch(tempBuffer.data(), pointCount);
        }

    }

    void mainwindow::save_test_epd(std::string& p_name, std::string& p_id, std::string& start_time, std::string& duration_time) {
        auto *t_epd = new EachPatientDatabase(ui_manager->baseDir_AppData.toStdString());
        bool is_success = t_epd->createNewTable(std::stoi(p_id));
        if (!is_success) {
            QMessageBox::warning(this, "错误", "创建表失败");
        }
        is_success = t_epd->add_testlist(p_name, p_id, start_time, duration_time);
        std::cout << "is_success_abc: " << is_success << std::endl;
    }



} // MyApp::UI::mainwindow

qint64 getChronoMicrosecondsSinceEpoch() {
    const auto now = std::chrono::system_clock::now();
    const auto duration = now.time_since_epoch();
    const auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration);
    return micros.count();
}

void checkMyCore() {
    // 获取当前代码正在哪个逻辑核心上执行
    const DWORD coreNum = GetCurrentProcessorNumber();
    qDebug() << "UI Thread is currently running on Logical Core:" << coreNum;
}