//
// Created by Administrator on 26-1-17.
//

#ifndef DIY_START_TEST_H
#define DIY_START_TEST_H
#pragma once

#include <QWidget>
#include <QDir>
#include <QTcpSocket>
#include "../../ui/manager_of_ui.h"

// namespace MyApp::UI{ class UiManager; }

class BLFrameParser;

namespace MyApp::UI{ class UiManager; }

class StartTest : public QObject{
    Q_OBJECT
public:
    explicit StartTest(QWidget *parent = nullptr, MyApp::UI::UiManager *ui_manager = nullptr);
    ~StartTest();

    BLFrameParser *blfp;

    bool btn_pressed(const std::string &_name, const std::string &_id);
    void test_interruption();

    void parserData(const QByteArray &data);
    void saveData(const QByteArray &data);
    void transmitData(const QByteArray &data);

    void executeCMD(const QByteArray &cmd);

private:
    bool openNewCsvFile();
    void closeCurrentCsvFile();

    QWidget *parent;
    MyApp::UI::UiManager *ui_manager;

    std::string p_name;
    std::string p_id;

    std::string start_time;
    qlonglong start_time_s = 0;
    std::string end_time;
    std::string duration_time;
    int test_stage;

    std::function<std::string()> save_dir_path;

    static constexpr int CHANNEL_COUNT = 7;  // todo
    static constexpr int SAMPLE_RATE_HZ = 1000;
    static constexpr int MAX_DURATION_SEC = 20 * 60; // 20 min
    static constexpr quint64 MAX_ROWS_PER_FILE = SAMPLE_RATE_HZ * MAX_DURATION_SEC; // 1,200,000
    quint64 currentRowCount = 0;

    QFile *csvFile{nullptr};
    QTextStream *csvStream{nullptr};

    QVector<float> txBuffer;
    // QTcpSocket *tcpSocket{nullptr};
};

class BLFrameParser : public QObject {
    Q_OBJECT
public:
    explicit BLFrameParser(QObject *parent = nullptr);

    void setDataFrame (uchar h1, uchar h2, uchar t1, uchar t2, int len);
    void setCommandFrame(uchar h1, uchar h2, uchar t1, uchar t2, int len);

    void appendInfo(const QByteArray &info);

signals:
    void dataFrameParsed(const QByteArray& payload);
    void commandFrameParsed(const QByteArray& payload);

private:
    void parse();

private:
    QByteArray m_totalBuffer;

    struct FrameDef {
        uchar head1, head2;
        uchar tail1, tail2;
        int   totalLen;      // 包含头尾的总长度
    } m_dataDef, m_cmdDef;

    static constexpr int DATA_FRAME_LEN = 30;
    static constexpr int CMD_FRAME_LEN  = 6;

public:
signals:
    void test_esp32_data_signal(double vol, int index);
};

#endif //DIY_START_TEST_H
