//
// Created by Administrator on 26-1-15.
//

#ifndef DIY_SERIAL_H
#define DIY_SERIAL_H



#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QMutex>
#include <QQueue>

namespace MyApp::UI{ class UiManager; }

class SerialManager : public QObject
{
    Q_OBJECT
public:
    explicit SerialManager(QObject* parent = nullptr);
    ~SerialManager();

    bool open(
        const QString& portName,
        qint32 baudrate = 3000000,
        QSerialPort::DataBits dataBits = QSerialPort::Data8,
        QSerialPort::StopBits stopBits = QSerialPort::OneStop,
        QSerialPort::Parity parity = QSerialPort::NoParity
        );

    void close();
    void write(const QString& data);
    QStringList serialGetPorts();

public slots:
    void setTesting(bool isTesting);

signals:
    void dataReceived(const QByteArray& data);
    void connectionSucceed();
    void connectionLost(const QString& msg);

private slots:
    void onReadyRead();
    void onError(QSerialPort::SerialPortError error);

private:
    QSerialPort* serial = nullptr;
    bool m_isTesting = false;
    QByteArray buffer;  // 是不是可以注释掉了
};


#endif //DIY_SERIAL_H
