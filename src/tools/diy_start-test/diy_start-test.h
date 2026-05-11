//
// Created by Administrator on 26-1-17.
//

#ifndef DIY_START_TEST_H
#define DIY_START_TEST_H
#pragma once

#include <QWidget>
#include <QDir>
#include <QTcpSocket>
#include <QMutex>
#include "../../ui/manager_of_ui.h"

// namespace MyApp::UI{ class UiManager; }

struct DataFrame {
    float data[8];
};
Q_DECLARE_METATYPE(DataFrame)

class BLFrameParser;

namespace MyApp::UI{ class UiManager; }

class StartTest : public QObject{
    Q_OBJECT
public:
    explicit StartTest(QWidget *parent = nullptr, MyApp::UI::UiManager *ui_manager = nullptr);
    ~StartTest();

    BLFrameParser *blfp;

    bool btn_pressed(const std::string &_name, const std::string &_id, bool start);
    void test_interruption();

    void parserData(const QByteArray &data);
    void saveData(QVector<DataFrame> batchSaveData);
    void transmitData(const QByteArray &data);

    void executeCMD(const QByteArray &cmd);

signals:
    void finished_test_epd(std::string& p_name, std::string& p_id, std::string& start_time, std::string& duration_time);

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

    void appendInfo(const QByteArray &info);

    QTimer parserTimer;
signals:
    void dataFrameParsed(const QByteArray& payload);
    void commandFrameParsed(const QByteArray& payload);

public:
    std::mutex m_mutex;
    QByteArray m_totalBuffer;
    QByteArray m_processBuffer;
    QByteArray freshData;
    QList<QVector<double>> m_channelData;

    void parse();


private:


    static constexpr int DATA_FRAME_LEN = 30;
    static constexpr uint8_t DATA_HEADER1 = 0xEE;
    static constexpr uint8_t DATA_HEADER2 = 0xBB;


public:
signals:
    void test_esp32_data_signal(QList<QVector<double>> &m_channelData);
    void dataSave(QVector<DataFrame> batchSaveData);
    void data_error(int error_count);
    // void test_esp32_data_signal(float *result_array);
    // void dataSave(float *result_array);
};

#endif //DIY_START_TEST_H
