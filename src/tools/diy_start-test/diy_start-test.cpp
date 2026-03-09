//
// Created by Administrator on 26-1-17.
//

#include "diy_start-test.h"

StartTest::StartTest(QWidget *parent, MyApp::UI::UiManager *ui_manager) : parent(parent), ui_manager(ui_manager){
    blfp = new BLFrameParser();  // 数据解析（串口+蓝牙统一使用）
    save_dir_path = [this]() {
        return this->ui_manager->baseDir_AppData.toStdString()
        + "/" + "each_P_data"
        + "/" + this->p_name
        + "_" + this->p_id
        + "/" + this->start_time;
    };
    // tcpSocket = new QTcpSocket(this);
    // tcpSocket->connectToHost("127.0.0.1", 5678);
}

StartTest::~StartTest() = default;

bool StartTest::btn_pressed(const std::string &_name, const std::string &_id) {
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

    if (!openNewCsvFile()) {
        return false;
    }

    return true;
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

void StartTest::saveData(const QByteArray &data){
    if (!csvStream) return;
    const auto ch = reinterpret_cast<const float*>(data.constData());
    (*csvStream)
        << ch[0] << ","  // ICG
        << ch[1] << ","  // PPG
        << ch[2] << ","  // ECG
        << ch[3] << ","  // AAA
        << ch[4] << ","  // BBB
        << ch[5] << ","  // CCC
        << ch[6] << "\n"; // DDD

}

void StartTest::transmitData(const QByteArray &data){

}

void StartTest::executeCMD(const QByteArray &cmd){

}

bool StartTest::openNewCsvFile()
{
    closeCurrentCsvFile();

    const QString target_dir = QString::fromStdString(save_dir_path());
    const QString filePath = target_dir + QString("/SourceData_%1.csv").arg(test_stage);

    std::cout << filePath.toStdString() << std::endl;

    csvFile = new QFile(filePath);
    if (!csvFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(parent, "错误", "CSV 文件创建失败");
        return false;
    }

    csvStream = new QTextStream(csvFile);

    // CSV header（顺序固定）
    *csvStream << "ICG,PPG,ECG,AAA,BBB,CCC,DDD\n";

    currentRowCount = 0;
    ++test_stage;

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
    setDataFrame(0xAA, 0xBB, 0xCC, 0xDD, 30);
    setCommandFrame(0xAB, 0xBC, 0xCD, 0xDE, 6);
}

void BLFrameParser::setDataFrame(const uchar h1, const uchar h2, const uchar t1, const uchar t2, const int len) {
    m_dataDef = {h1, h2, t1, t2, len};
}

void BLFrameParser::setCommandFrame(const uchar h1, const uchar h2, const uchar t1, const uchar t2, const int len) {
    m_cmdDef = {h1, h2, t1, t2, len};
}

void BLFrameParser::appendInfo(const QByteArray &info) {
    if (info.isEmpty())
        return;
    if (info.size() != 21)
        return;

    constexpr double VREF = 3.3;
    constexpr double ADC_MAX = 16777215.0;  // 2^24 - 1

    for (int i = 0; i < 7; ++i)
    {
        const int index = i * 3;

        // 必须转 unsigned char，防止符号扩展
        const uint32_t x0 = static_cast<unsigned char>(info[index]);
        const uint32_t x1 = static_cast<unsigned char>(info[index + 1]);
        const uint32_t x2 = static_cast<unsigned char>(info[index + 2]);
        const uint32_t value = (x0 << 16) | (x1 << 8) | x2;
        const double voltage = value / ADC_MAX * VREF;
        emit test_esp32_data_signal(voltage, i);
    }

    // 最终形式
    // 看上来数据的格式
    // std::cout << info << std::endl;
    // std::cout << info.toHex(' ').toStdString() << "  len=" << info.size() << std::endl;

    // m_totalBuffer.append(info);
    // parse();
}

void BLFrameParser::parse()
{
    while (m_totalBuffer.size() > 0) {
        bool found = false;
        // 先尝试解析数据包
        if (m_totalBuffer.size() >= m_dataDef.totalLen) {
            for (int i = 0; i <= m_totalBuffer.size() - m_dataDef.totalLen; ++i)
            {
                const auto p = reinterpret_cast<const uchar*>(m_totalBuffer.constData() + i);
                if (p[0] == m_dataDef.head1 && p[1] == m_dataDef.head2 &&
                    p[m_dataDef.totalLen-2] == m_dataDef.tail1 && p[m_dataDef.totalLen-1] == m_dataDef.tail2)
                {
                    // todo: connect dafaFrameParsed信号，进行plot和save操作
                    /*
                     *void Engine::insertDataByIndex(int index, float value)
                     *{
                     *    if (index < 0 || index >= m_plots.size()) {
                     *        qWarning() << "Invalid plot index:" << index;
                     *        return;
                     *    }
                     *    OscilloscopePlot* plot = m_plots.at(index);  // at() 有边界检查
                     *    if (!plot) {
                     *        qWarning() << "Null plot pointer at index:" << index;
                     *        return;
                     *    }
                     *    plot->insertData(value);
                     *}
                     */

                    emit dataFrameParsed(m_totalBuffer.mid(i + 2, m_dataDef.totalLen - 4)); // 去掉头尾
                    m_totalBuffer.remove(i, m_dataDef.totalLen); // 整条删除
                    found = true;
                    break;
                }
            }
        }
        // 再尝试解析指令包
        if (!found && m_totalBuffer.size() >= m_cmdDef.totalLen)
        {
            for (int i = 0; i <= m_totalBuffer.size() - m_cmdDef.totalLen; ++i)
            {
                const auto p = reinterpret_cast<const uchar*>(m_totalBuffer.constData() + i);
                if (p[0] == m_cmdDef.head1 && p[1] == m_cmdDef.head2 &&
                    p[m_cmdDef.totalLen-2] == m_cmdDef.tail1 && p[m_cmdDef.totalLen-1] == m_cmdDef.tail2)
                {
                    emit commandFrameParsed(m_totalBuffer.mid(i + 2, m_cmdDef.totalLen-4));
                    m_totalBuffer.remove(i, m_cmdDef.totalLen);
                    found = true;
                    break;
                }
            }
        }

        if (!found) break;   // 再也找不到完整帧就退出，等待下次拼包
    }
}





