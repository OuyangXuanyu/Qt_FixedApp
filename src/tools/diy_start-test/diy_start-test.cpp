//
// Created by Administrator on 26-1-17.
//

#include "diy_start-test.h"

#include <algorithm>
#include <limits>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

qint64 getChronoMicrosecondsSinceEpoch();

StartTest::StartTest(QWidget *parent, MyApp::UI::UiManager *ui_manager) : parent(parent), ui_manager(ui_manager){
    blfp = new BLFrameParser();  // 数据解析（串口+蓝牙统一使用）
    save_dir_path = [this]() {
        return this->ui_manager->baseDir_AppData.toStdString()
        + "/" + "each_P_data"
        + "/" + this->p_name
        + "_" + this->p_id
        + "/" + this->start_time;
    };
    connect(blfp, &BLFrameParser::dataSave, this, &StartTest::saveData);
    // tcpSocket = new QTcpSocket(this);
    // tcpSocket->connectToHost("127.0.0.1", 5678);
}

StartTest::~StartTest() = default;

bool StartTest::btn_pressed(const std::string &_name, const std::string &_id, const bool start) {
    if (start == true) {
        p_name = _name;
        p_id = _id;

        // 时间戳确定
        const QDateTime dt = QDateTime::currentDateTime();
        start_time = dt
                .toString("yyyyMMddHHmmss")
                .toStdString();
        start_time_s = dt.toSecsSinceEpoch();

        const QString target_dir = QString::fromStdString(save_dir_path());

        if (!QDir().mkpath(target_dir)) {
            QMessageBox::warning(nullptr, "错误", "创建路径失败");
            return false;
        }
        test_stage = 0;
        totalSavedRows = 0;

        if (!openNewCsvFile()) {
            return false;
        }

        // blfp->parserTimer.start();

        return true;
    } else {
        closeCurrentCsvFile();
        const QDateTime dt = QDateTime::currentDateTime();
        end_time = dt
            .toString("yyyyMMddHHmmss")
            .toStdString();
        duration_time = std::to_string((dt.toSecsSinceEpoch() - start_time_s));
        emit finished_test_epd(p_name, p_id, start_time, duration_time);

        // blfp->parserTimer.stop();

        return true;
    }
}

void StartTest::test_interruption() {

}

// 数据操作
void StartTest::parserData(const QByteArray &data){

    ++currentRowCount;
    // 到达 20 分钟，切换文件
    if (currentRowCount >= MAX_ROWS_PER_FILE) {
        openNewCsvFile();
    }
}

void StartTest::saveData(QVector<DataFrame> batchSaveData){
    if (!csvStream) return;
    for (const auto&[data] : batchSaveData) {
        if (currentRowCount >= MAX_ROWS_PER_FILE && !openNewCsvFile())
            return;
        // 3. 这里的 frame.data 就是你之前的 result_array
        (*csvStream)
            << data[0] << ","
            << data[1] << ","
            << data[2] << ","
            << data[3] << ","
            << data[4] << ","
            << data[5] << ","
            << data[6] << ","
            << data[7] << ","
            << data[8] << "\n";
        ++currentRowCount;
        ++totalSavedRows;
    }
    if (csvStream->status() != QTextStream::Ok) {
        emit storageError(QStringLiteral("写入信号文件失败：%1").arg(currentFilePath));
        return;
    }
    emit storageProgress(totalSavedRows, currentFilePath);
}

void StartTest::transmitData(const QByteArray &data){

}

void StartTest::executeCMD(const QByteArray &cmd){

}

bool StartTest::openNewCsvFile()
{
    closeCurrentCsvFile();

    const QString target_dir = QString::fromStdString(save_dir_path());
    const QString filePath = target_dir + QString("/TestZhao_%1.csv").arg(test_stage);
    currentFilePath = filePath;

    std::cout << filePath.toStdString() << std::endl;

    csvFile = new QFile(filePath);
    if (!csvFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        emit storageError(QStringLiteral("无法创建信号文件：%1").arg(filePath));
        QMessageBox::warning(parent, "错误", "CSV 文件创建失败");
        return false;
    }

    csvStream = new QTextStream(csvFile);

    // CSV header（顺序固定）
    *csvStream << "CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9\n";

    currentRowCount = 0;
    ++test_stage;

    emit storageProgress(totalSavedRows, currentFilePath);

    return true;
}

void StartTest::closeCurrentCsvFile()
{
    if (csvStream) {
        csvStream->flush();
        delete csvStream;
        csvStream = nullptr;
    }

    if (csvFile) {
        csvFile->close();
        delete csvFile;
        csvFile = nullptr;
    }
}

BLFrameParser::BLFrameParser(QObject *parent) : QObject(parent){
    parserTimer.setInterval(1);
    connect(&parserTimer, &QTimer::timeout, this, &BLFrameParser::parse);
    m_totalBuffer.reserve(2048 * 2048);
    m_processBuffer.reserve(2048 * 2048);
    freshData.reserve(2048 * 2048);
    for(int i=0; i<9; ++i) m_channelData.append(QVector<double>());
}

void BLFrameParser::appendInfo(const QByteArray &info) {
    // if (info.size() != 21)
    //     return;
    //
    // constexpr double VREF = 3.3;
    // constexpr double ADC_MAX = 16777215.0;  // 2^24 - 1
    //
    // for (int i = 0; i < 7; ++i)
    // {
    //     const int index = i * 3;
    //
    //     // 必须转 unsigned char，防止符号扩展
    //     const uint32_t x0 = static_cast<unsigned char>(info[index]);
    //     const uint32_t x1 = static_cast<unsigned char>(info[index + 1]);
    //     const uint32_t x2 = static_cast<unsigned char>(info[index + 2]);
    //     const uint32_t value = (x0 << 16) | (x1 << 8) | x2;
    //     const double voltage = value / ADC_MAX * VREF;
    //     emit test_esp32_data_signal(voltage, i);
    // }

    // 最终形式
    // 看上来数据的格式
    // std::cout << info << std::endl;
    // std::cout << info.toHex(' ').toStdString() << "  len=" << info.size() << std::endl;
    // std::cout << "len=" << info.size() << std::endl;

    m_mutex.lock();
    m_totalBuffer.append(info);
    m_mutex.unlock();
    parse();
}

void BLFrameParser::parse()
// todo: 还有余量?
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        freshData.swap(m_totalBuffer);
    }

    static double CH5_odd_judge = 0;
    static double CH5_even_judge = 0;
    static double CH6_odd_judge = 0;
    static double CH6_even_judge = 0;
    static int CH5_count_judge = 0;
    static int CH6_count_judge = 0;

    static bool CH5_judge = false;
    static bool CH6_judge = false;

    m_processBuffer.append(freshData);
    freshData.clear();

    while (m_processBuffer.contains("\r\n")) {
        const int index = m_processBuffer.indexOf("\r\n");
        QByteArray line = m_processBuffer.left(index).trimmed(); // 提取一行，如 "CH0:1.6500"
        m_processBuffer.remove(0, index + 2); // 从缓冲区移除已处理的数据

        // 解析字符串
        QString strLine = QString::fromUtf8(line);
        if (strLine.contains(":")) {
            QStringList parts = strLine.split(":");
            if (parts.size() == 2) {
                const int chIdx = parts[0].mid(2).toInt(); // 提取 "CH0" 中的 "0"
                const double val = parts[1].toDouble();    // 提取 "1.6500"

                if (chIdx < 5) m_channelData[chIdx].append(val);
                else {
                    if (CH5_count_judge < 100 && chIdx == 5) {
                        if (CH5_count_judge % 2 == 0) CH5_even_judge += val;
                        else CH5_odd_judge += val;
                        CH5_count_judge++;
                        m_channelData[chIdx].append(0);
                        m_channelData[chIdx].append(0);
                        m_channelData[chIdx+1].append(0);
                        m_channelData[chIdx+1].append(0);
                    } else if (CH6_count_judge < 100 && chIdx == 6) {
                        if (CH6_count_judge % 2 == 0) CH6_even_judge += val;
                        else CH6_odd_judge += val;
                        CH6_count_judge++;
                        m_channelData[chIdx+1].append(0);
                        m_channelData[chIdx+1].append(0);
                        m_channelData[chIdx+2].append(0);
                        m_channelData[chIdx+2].append(0);
                    } else if (CH5_count_judge >= 100 && chIdx == 5) {
                        if (CH5_count_judge % 2 == 0) {
                            if (CH5_judge == true) {
                                m_channelData[chIdx].append(val);
                                m_channelData[chIdx].append(val);
                            }
                            else {
                                m_channelData[chIdx + 1].append(val);
                                m_channelData[chIdx + 1].append(val);
                            }
                        } else if (CH5_count_judge % 2 == 1) {
                            if (CH5_judge == true) {
                                m_channelData[chIdx + 1].append(val);
                                m_channelData[chIdx + 1].append(val);
                            }
                            else {
                                m_channelData[chIdx].append(val);
                                m_channelData[chIdx].append(val);
                            }
                        }
                    } else if (CH6_count_judge >= 100 && chIdx == 6) {
                        if (CH6_count_judge % 2 == 0) {
                            if (CH6_judge == true) {
                                m_channelData[chIdx+1].append(val);
                                m_channelData[chIdx+1].append(val);
                            }
                            else {
                                m_channelData[chIdx+2].append(val);
                                m_channelData[chIdx+2].append(val);
                            }
                        } else if (CH5_count_judge % 2 == 1) {
                            if (CH6_judge == true) {
                                m_channelData[chIdx+2].append(val);
                                m_channelData[chIdx+2].append(val);
                            }
                            else {
                                m_channelData[chIdx+1].append(val);
                                m_channelData[chIdx+1].append(val);
                            }
                        }
                    }
                }
            }
        }
        if (CH5_count_judge == 100) {
            std::cout << "CH5达到100了" << std::endl;
            CH5_judge = CH5_odd_judge<CH5_even_judge;
        }
        if (CH6_count_judge == 100) {
            std::cout << "CH6达到100了" << std::endl;
            CH6_judge = CH6_odd_judge<CH6_even_judge;
        }

        // 前 8 个存储通道都集齐一批数据后，才同时绘图和落盘。
        bool ready = true;
        for (int channel = 0; channel < SIGNAL_CHANNEL_COUNT; ++channel)
            ready = ready && m_channelData[channel].size() >= 10;
        if (ready) {
            emit test_esp32_data_signal(m_channelData); // 发送信号给绘图模块

            constexpr int savedChannelCount = SIGNAL_CHANNEL_COUNT;
            qsizetype frameCount = std::numeric_limits<qsizetype>::max();
            for (int channel = 0; channel < savedChannelCount; ++channel)
                frameCount = std::min(frameCount, m_channelData[channel].size());
            if (frameCount > 0 && frameCount != std::numeric_limits<qsizetype>::max()) {
                QVector<DataFrame> frames;
                frames.reserve(frameCount);
                for (qsizetype sample = 0; sample < frameCount; ++sample) {
                    DataFrame frame{};
                    for (int channel = 0; channel < savedChannelCount; ++channel)
                        frame.data[channel] = static_cast<float>(m_channelData[channel][sample]);
                    frames.append(frame);
                }
                emit dataSave(frames);
            }

            // 清空缓冲区供下次收集
            for(int i=0; i<9; ++i) m_channelData[i].clear();
        }
    }



}

// qint64 getChronoMicrosecondsSinceEpoch() {
//     auto now = std::chrono::system_clock::now();
//     auto duration = now.time_since_epoch();
//     auto micros = std::chrono::duration_cast<std::chrono::microseconds>(duration);
//     return micros.count();
// }





