//
// Created by Administrator on 26-7-15.
//

#include "diy_signal-analysis.h"

#include <algorithm>
#include <cmath>
#include <iostream>

#include <QMetaObject>
#include <QReadLocker>
#include <QWriteLocker>

SignalHistoryBuffer::SignalHistoryBuffer(const int sampleRateHz, const int historySeconds) {
    configure(sampleRateHz, historySeconds);
}

void SignalHistoryBuffer::configure(const int sampleRateHz, const int historySeconds) {
    const int validSampleRate = std::max(1, sampleRateHz);
    const int validHistorySeconds = std::max(1, historySeconds);
    const std::size_t newCapacity = static_cast<std::size_t>(validSampleRate)
                                    * static_cast<std::size_t>(validHistorySeconds);

    QWriteLocker locker(&lock);
    configuredSampleRateHz = validSampleRate;
    configuredHistorySeconds = validHistorySeconds;
    ringBuffer.assign(newCapacity, DataFrame{});
    writeIndex = 0;
    validFrameCount = 0;
}

void SignalHistoryBuffer::clear() {
    QWriteLocker locker(&lock);
    writeIndex = 0;
    validFrameCount = 0;
}

void SignalHistoryBuffer::append(const QVector<DataFrame> &frames) {
    if (frames.isEmpty())
        return;

    QWriteLocker locker(&lock);
    const std::size_t bufferCapacity = ringBuffer.size();
    if (bufferCapacity == 0)
        return;

    qsizetype firstFrame = 0;
    if (static_cast<std::size_t>(frames.size()) >= bufferCapacity) {
        firstFrame = frames.size() - static_cast<qsizetype>(bufferCapacity);
        writeIndex = 0;
        validFrameCount = 0;
    }

    for (qsizetype index = firstFrame; index < frames.size(); ++index) {
        ringBuffer[writeIndex] = frames[index];
        writeIndex = (writeIndex + 1) % bufferCapacity;
        validFrameCount = std::min(validFrameCount + 1, bufferCapacity);
    }
}

std::vector<DataFrame> SignalHistoryBuffer::snapshot() const {
    QReadLocker locker(&lock);
    std::vector<DataFrame> result;
    result.reserve(validFrameCount);

    if (validFrameCount == 0 || ringBuffer.empty())
        return result;

    const std::size_t oldestIndex = (writeIndex + ringBuffer.size() - validFrameCount)
                                    % ringBuffer.size();
    for (std::size_t offset = 0; offset < validFrameCount; ++offset)
        result.push_back(ringBuffer[(oldestIndex + offset) % ringBuffer.size()]);

    return result;
}

std::size_t SignalHistoryBuffer::frameCount() const {
    QReadLocker locker(&lock);
    return validFrameCount;
}

std::size_t SignalHistoryBuffer::capacity() const {
    QReadLocker locker(&lock);
    return ringBuffer.size();
}

int SignalHistoryBuffer::sampleRateHz() const {
    QReadLocker locker(&lock);
    return configuredSampleRateHz;
}

int SignalHistoryBuffer::historySeconds() const {
    QReadLocker locker(&lock);
    return configuredHistorySeconds;
}

SignalAnalysisWorker::SignalAnalysisWorker(std::shared_ptr<SignalHistoryBuffer> history,
                                           QObject *parent)
    : QObject(parent), history(std::move(history)) {
}

void SignalAnalysisWorker::start() {
    if (!analysisTimer) {
        analysisTimer = new QTimer(this);
        connect(analysisTimer, &QTimer::timeout, this, &SignalAnalysisWorker::runAnalysis);
    }

    analysisTimer->setInterval(analysisIntervalMs);
    analysisTimer->start();
}

void SignalAnalysisWorker::stop() {
    if (analysisTimer)
        analysisTimer->stop();
}

void SignalAnalysisWorker::setAnalysisInterval(const int intervalMs) {
    analysisIntervalMs = std::max(1, intervalMs);
    if (analysisTimer)
        analysisTimer->setInterval(analysisIntervalMs);
}

void SignalAnalysisWorker::runAnalysis() {
    constexpr int ExampleChannel = 1; // 软件 CH1

    const std::vector<DataFrame> frames = history->snapshot();
    const std::size_t requiredFrameCount = history->capacity();
    if (frames.size() < requiredFrameCount) {
        std::cout << "[SignalAnalysis] Waiting for a complete "
                  << history->historySeconds() << " s window: "
                  << frames.size() << "/" << requiredFrameCount
                  << " frames" << std::endl;
        return;
    }

    // Welford 算法：数值稳定地计算最近 10 秒 CH1 的总体方差。
    double mean = 0.0;
    double squaredDeviationSum = 0.0;
    qsizetype validSampleCount = 0;
    for (const DataFrame &frame : frames) {
        const double value = frame.data[ExampleChannel];
        if (!std::isfinite(value))
            continue;

        ++validSampleCount;
        const double delta = value - mean;
        mean += delta / static_cast<double>(validSampleCount);
        squaredDeviationSum += delta * (value - mean);
    }

    if (validSampleCount == 0)
        return;

    const double variance = squaredDeviationSum / static_cast<double>(validSampleCount);
    std::cout << "[SignalAnalysis] CH1 variance over the latest "
              << history->historySeconds() << " s = " << variance
              << " (samples=" << validSampleCount << ")" << std::endl;
    emit ch1VarianceCalculated(variance, validSampleCount);
}

SignalAnalysisManager::SignalAnalysisManager(QObject *parent)
    : QObject(parent), history(std::make_shared<SignalHistoryBuffer>()) {
    worker = new SignalAnalysisWorker(history);
    worker->moveToThread(&analysisThread);

    connect(&analysisThread, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &SignalAnalysisWorker::ch1VarianceCalculated,
            this, &SignalAnalysisManager::ch1VarianceCalculated);

    analysisThread.setObjectName(QStringLiteral("SignalAnalysisThread"));
    analysisThread.start();
}

SignalAnalysisManager::~SignalAnalysisManager() {
    acceptingFrames.store(false, std::memory_order_release);
    if (analysisThread.isRunning()) {
        QMetaObject::invokeMethod(worker, &SignalAnalysisWorker::stop,
                                  Qt::BlockingQueuedConnection);
        analysisThread.quit();
        analysisThread.wait();
    }
}

void SignalAnalysisManager::configureHistory(const int sampleRateHz, const int historySeconds) {
    history->configure(sampleRateHz, historySeconds);
}

void SignalAnalysisManager::setAnalysisInterval(const int intervalMs) {
    QMetaObject::invokeMethod(worker, "setAnalysisInterval", Qt::QueuedConnection,
                              Q_ARG(int, intervalMs));
}

void SignalAnalysisManager::start() {
    acceptingFrames.store(true, std::memory_order_release);
    QMetaObject::invokeMethod(worker, &SignalAnalysisWorker::start, Qt::QueuedConnection);
}

void SignalAnalysisManager::stop() {
    acceptingFrames.store(false, std::memory_order_release);
    QMetaObject::invokeMethod(worker, &SignalAnalysisWorker::stop, Qt::QueuedConnection);
}

void SignalAnalysisManager::clear() {
    history->clear();
}

std::vector<DataFrame> SignalAnalysisManager::latestSnapshot() const {
    return history->snapshot();
}

void SignalAnalysisManager::appendFrames(const QVector<DataFrame> &frames) {
    if (acceptingFrames.load(std::memory_order_acquire))
        history->append(frames);
}
