//
// Created by Administrator on 26-1-16.
//

#include "diy_bl.h"

// 蓝牙扫描类
BluetoothScanner::BluetoothScanner(QObject *parent) : QObject(parent) {
    agent = new QBluetoothDeviceDiscoveryAgent(this);
    agent->setLowEnergyDiscoveryTimeout(5000);

    connect(agent, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered, this, &BluetoothScanner::onDeviceDiscovered);
    connect(agent, &QBluetoothDeviceDiscoveryAgent::finished, this, &BluetoothScanner::onScanFinished);
    connect(agent, &QBluetoothDeviceDiscoveryAgent::errorOccurred, this, &BluetoothScanner::onScanError);
}
BluetoothScanner::~BluetoothScanner() {

}

void BluetoothScanner::startScan() {
    devices.clear();
    agent->start(QBluetoothDeviceDiscoveryAgent::LowEnergyMethod);
}
void BluetoothScanner::stopScan() {
    agent->stop();
}

void BluetoothScanner::onDeviceDiscovered(const QBluetoothDeviceInfo &info) {
    if (!info.name().isEmpty() && info.name().toLower() != "unknown") {
        devices.append(info);
    }
}
void BluetoothScanner::onScanFinished() {
    emit deviceFound(devices);
}
void BluetoothScanner::onScanError(QBluetoothDeviceDiscoveryAgent::Error)
{
    emit scanError(agent->errorString());
}

// 蓝牙管理类
BluetoothManager::BluetoothManager(QObject *parent) :  QObject(parent) {
    blScr = new BluetoothScanner(this);
}
BluetoothManager::~BluetoothManager() {

}

void BluetoothManager::connectToDevice(const QBluetoothDeviceInfo &device)
{
    controller = QLowEnergyController::createCentral(device, this);

    connect(controller, &QLowEnergyController::connected,
            this, &BluetoothManager::onConnected);  // 物理层面的连接成功

    connect(controller, &QLowEnergyController::disconnected,
            this, &BluetoothManager::onDisconnected);

    connect(controller, &QLowEnergyController::serviceDiscovered,
            this, &BluetoothManager::onServiceDiscovered);  // 在&QLowEnergyController::connected中调用

    connect(controller, &QLowEnergyController::discoveryFinished,
            this, &BluetoothManager::onServiceScanDone);  // 此时应当只有Service UUID列表

    emit statusChanged("正在连接蓝牙设备...");
    controller->connectToDevice();
}

void BluetoothManager::onConnected()
{
    emit statusChanged("已连接，扫描服务...");
    controller->discoverServices();
}
void BluetoothManager::onDisconnected()
{
    emit statusChanged("蓝牙已断开");
    emit isConnecting(false);
}
void BluetoothManager::onServiceDiscovered(const QBluetoothUuid &uuid)
{
    Q_UNUSED(uuid);
}
void BluetoothManager::onServiceScanDone()
{
    // Service UUID 判定
    for (const auto &uuid : controller->services()) {
        std::cout << "[Service] " << uuid.toString().toStdString() << std::endl;

        // 写固定配置的UUID
        // 但是先写一下ESP32的UUID处理逻辑（以0000作为起始）
        // if (isStandardService(uuid)) {
        //     continue;
        // }

        service = controller->createServiceObject(uuid, this);

        if (!service)
            continue;
        connect(service, &QLowEnergyService::stateChanged,
            this, &BluetoothManager::onServiceStateChanged);

        connect(service, &QLowEnergyService::characteristicChanged,
            this, &BluetoothManager::onCharacteristicChanged);

        service->discoverDetails();
        return;
    }

    emit statusChanged("未找到自定义服务");
}

void BluetoothManager::onServiceStateChanged(QLowEnergyService::ServiceState state) {
    if (state != QLowEnergyService::RemoteServiceDiscovered)
        return;

    std::cout << "=== Service Ready ===" << std::endl;

    for (const auto &c : service->characteristics()) {
        std::cout << "Char: "
                  << c.uuid().toString().toStdString()
                  << " Props=" << c.properties()
                  << std::endl;

        if ((c.properties() & QLowEnergyCharacteristic::Notify) ||
            (c.properties() & QLowEnergyCharacteristic::Indicate)) {

            if (!rxChar.isValid()) rxChar = c;
        }

        if ((c.properties() & QLowEnergyCharacteristic::Write) ||
            (c.properties() & QLowEnergyCharacteristic::WriteNoResponse)) {

            if (!txChar.isValid()) txChar = c;
        }
    }

    if (!(rxChar.isValid() && txChar.isValid())) {
        emit statusChanged("未找到合适特征");
    }

    // 启用 Notify
    const auto cccd = rxChar.descriptor(
        QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration);

    if (cccd.isValid())
        service->writeDescriptor(cccd, QByteArray::fromHex("0100"));

    emit statusChanged("蓝牙连接成功（自动匹配）");
    emit isConnecting(true);
}

bool BluetoothManager::isStandardService(const QBluetoothUuid &uuid) {
    const QString s = uuid.toString().toLower();
    return s.startsWith("{000018");
}

void BluetoothManager::write(const QByteArray &data)
{
    if (!service || !txChar.isValid()) return;
    service->writeCharacteristic(txChar, data);
}

void BluetoothManager::onCharacteristicChanged(
    const QLowEnergyCharacteristic &, const QByteArray &value)
{
    emit dataReceived(value);
    // std::cout << QString::fromUtf8(value).toStdString() << std::endl;
}

void BluetoothManager::disconnectDevice()
{
    if (controller) controller->disconnectFromDevice();
}