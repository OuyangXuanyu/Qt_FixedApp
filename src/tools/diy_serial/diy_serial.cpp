//
// Created by Administrator on 26-1-15.
//

#include "diy_serial.h"

#include <iostream>

SerialManager::SerialManager(QObject* parent) : QObject(parent)
{
    serial = new QSerialPort(this);
}

SerialManager::~SerialManager() {
    this->close();
}

bool SerialManager::open(
    const QString& portName,
    const qint32 baudrate,
    const QSerialPort::DataBits dataBits,
    const QSerialPort::StopBits stopBits,
    const QSerialPort::Parity parity
    )
{
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
    return true;
}

void SerialManager::close()
{
    if (serial && serial->isOpen())
        serial->close();
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
    serial->write(data.toStdString().c_str());
    // serial->write(data.toStdString().data());
}

void SerialManager::onReadyRead()
{
    buffer.append(serial->readAll());

    while (buffer.contains('\n')) {
        auto line = buffer.left(buffer.indexOf('\n')).trimmed();
        buffer.remove(0, buffer.indexOf('\n') + 1);

        // todo: 确实是否需要转换函数
        emit dataReceived(line);
        // emit dataReceived(QString::fromUtf8(line));
    }
}

void SerialManager::onError(const QSerialPort::SerialPortError error)
{
    if (error == QSerialPort::ResourceError) {
        emit connectionLost("串口异常断开");
        close();
    }
}
