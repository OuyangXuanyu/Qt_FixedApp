//
// Created by Administrator on 26-1-15.
//

#ifndef DIY_SERIAL_H
#define DIY_SERIAL_H

#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>

class SerialManager : public QObject
{
    Q_OBJECT
public:
    explicit SerialManager(QObject* parent = nullptr);
    ~SerialManager();

    bool open(
        const QString& portName,
        qint32 baudrate = QSerialPort::Baud115200,
        QSerialPort::DataBits dataBits = QSerialPort::Data8,
        QSerialPort::StopBits stopBits = QSerialPort::OneStop,
        QSerialPort::Parity parity = QSerialPort::NoParity
        );
    void close();
    void write(const QString& data);
    QStringList serialGetPorts();

signals:
    void dataReceived(const QByteArray& data);
    // void dataReceived(const QString& data);
    void connectionLost(const QString& msg);

private slots:
    void onReadyRead();
    void onError(QSerialPort::SerialPortError error);

private:
    QSerialPort* serial = nullptr;
    QByteArray buffer;
};


#endif //DIY_SERIAL_H
