//
// Created by Administrator on 26-1-15.
//



#include "diy_serial.h"

#include <iostream>

#include "../../ui/manager_of_ui.h"

#ifdef Q_OS_WIN
#include <windows.h>
#endif

SerialManager::SerialManager(QObject* parent) : QObject(parent)
{
    serial = new QSerialPort(this);
    serial->setReadBufferSize(2048 * 2048);
}

SerialManager::~SerialManager() {
    this->close();
    delete serial;

}

void SerialManager::setTesting(const bool isTesting) {
    m_isTesting = isTesting;
}

bool SerialManager::open(
    const QString& portName,
    const qint32 baudrate,
    const QSerialPort::DataBits dataBits,
    const QSerialPort::StopBits stopBits,
    const QSerialPort::Parity parity
    )
{
    {
#ifdef Q_OS_WIN
    // 换核

        // 获取当前执行线程的句柄
        HANDLE hThread = GetCurrentThread();

        // 绑定到核心 10 (掩码 1 << 10)
        DWORD_PTR mask = (1 << 10);
        DWORD_PTR previousMask = SetThreadAffinityMask(hThread, mask);
        if (previousMask != 0) {
            qDebug() << "Serial Thread successfully pinned to Logical Core 10";
        } else {
            qDebug() << "Failed to set Affinity Mask. Error:" << GetLastError();
        }
        const DWORD coreNum = GetCurrentProcessorNumber();
        qDebug() << "UI Thread is currently running on Logical Core:" << coreNum;

        // 设置高优先级，这对于 3Mbps 采样非常关键
        SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
#endif
    }
    serial->setPortName(portName);
    serial->setBaudRate(baudrate);
    serial->setDataBits(dataBits);
    serial->setStopBits(stopBits);
    serial->setParity(parity);

    serial->setFlowControl(QSerialPort::NoFlowControl);

    if (!serial->open(QIODevice::ReadWrite)) {
        std::cout << "failed to open serial port" << std::endl;
        return false;
    }

    serial->setDataTerminalReady(false);
    serial->setRequestToSend(false);

    connect(serial, &QSerialPort::readyRead, this, &SerialManager::onReadyRead);
    connect(serial, &QSerialPort::errorOccurred, this, &SerialManager::onError);

    std::cout << "success to open serial port" << std::endl;
    emit connectionSucceed();
    return true;
}

void SerialManager::close()
{
    if (serial && serial->isOpen()) {
        serial->close();
        emit connectionLost("");
    }
    // delete serial;
}

QStringList SerialManager::serialGetPorts()
{
    QStringList ports;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo& info : infos) {
        std::cout << info.portName().toStdString() << std::endl;
        ports << info.portName();   // Windows: COM3
        // info.systemLocation();   // Linux / Mac: /dev/ttyUSB0
    }
    return ports;
}

void SerialManager::write(const QString& data) {
    const QByteArray hexData = QByteArray::fromHex(data.toLatin1());
    serial->write(hexData);
}

void SerialManager::onReadyRead()
{
    // static QByteArray newData{};

    if (m_isTesting) {
        QByteArray newData{};
        while (serial->bytesAvailable() > 0) {
            newData.append(serial->readAll());
        }
        emit dataReceived(newData);
    } else {
        serial->readAll();
    }


}

void SerialManager::onError(const QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        emit connectionLost("串口异常断开");
        close();
    }
}
