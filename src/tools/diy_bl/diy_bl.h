//
// Created by Administrator on 26-1-16.
//

#ifndef DIY_BL_H
#define DIY_BL_H

#include <iostream>

#include <QObject>
#include <QStringLiteral>

#include <QBluetoothDeviceInfo>
#include <QBluetoothDeviceDiscoveryAgent>

#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QBluetoothDeviceInfo>

// 蓝牙扫描类
class BluetoothScanner : public QObject {
    Q_OBJECT
public:
    explicit BluetoothScanner(QObject *parent = nullptr);
    ~BluetoothScanner();

    void startScan();
    void stopScan();

signals:
        void deviceFound(const QList<QBluetoothDeviceInfo> &devices);
        void scanError(const QString &error);

private slots:
    void onDeviceDiscovered(const QBluetoothDeviceInfo &info);
    void onScanFinished();
    void onScanError(QBluetoothDeviceDiscoveryAgent::Error error);

private:
    QBluetoothDeviceDiscoveryAgent *agent;
    QList<QBluetoothDeviceInfo> devices;
};

// 蓝牙管理类
class BluetoothManager : public QObject{
    Q_OBJECT
public:
    explicit BluetoothManager(QObject *parent = nullptr);
    ~BluetoothManager() override;

    BluetoothScanner *blScr;

    void connectToDevice(const QBluetoothDeviceInfo &device);
    void disconnectDevice();
    void write(const QByteArray &data);

signals:
    void statusChanged(const QString &msg);
    void log(const QString &msg);
    void dataReceived(const QByteArray &data);
    void isConnecting(bool _info);

private slots:
    void onConnected();
    void onDisconnected();

    void onServiceDiscovered(const QBluetoothUuid &uuid);
    void onServiceScanDone();
    void onServiceStateChanged(QLowEnergyService::ServiceState state);

    void onCharacteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);

    static bool isStandardService(const QBluetoothUuid &uuid);
private:
    QLowEnergyController *controller = nullptr;
    QLowEnergyService *service = nullptr;

    QLowEnergyCharacteristic rxChar;
    QLowEnergyCharacteristic txChar;

};


#endif //DIY_BL_H
