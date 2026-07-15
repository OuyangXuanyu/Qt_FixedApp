//
// Created by Administrator on 26-7-15.
//

#ifndef DIY_SIGNAL_ANALYSIS_H
#define DIY_SIGNAL_ANALYSIS_H

#include <atomic>
#include <memory>
#include <vector>

#include <QObject>
#include <QReadWriteLock>
#include <QThread>
#include <QTimer>

#include "../diy_signal-data/diy_signal-data.h"

// 保存最近一段时间的九通道软件数据帧。
// 写入和快照复制分别使用写锁、读锁；算法计算发生在锁外，不阻塞实时写入。
class SignalHistoryBuffer final {
public:
    explicit SignalHistoryBuffer(int sampleRateHz = 200, int historySeconds = 10);

    void configure(int sampleRateHz, int historySeconds);
    void clear();
    void append(const QVector<DataFrame> &frames);

    [[nodiscard]] std::vector<DataFrame> snapshot() const;
    [[nodiscard]] std::size_t frameCount() const;
    [[nodiscard]] std::size_t capacity() const;
    [[nodiscard]] int sampleRateHz() const;
    [[nodiscard]] int historySeconds() const;

private:
    mutable QReadWriteLock lock;
    std::vector<DataFrame> ringBuffer;
    std::size_t writeIndex = 0;
    std::size_t validFrameCount = 0;
    int configuredSampleRateHz = 200;
    int configuredHistorySeconds = 10;
};

// 算法执行对象。它运行在独立线程中，后续实时分析算法可以继续放在 runAnalysis() 中。
class SignalAnalysisWorker : public QObject {
    Q_OBJECT

public:
    explicit SignalAnalysisWorker(std::shared_ptr<SignalHistoryBuffer> history,
                                  QObject *parent = nullptr);

public slots:
    void start();
    void stop();
    void setAnalysisInterval(int intervalMs);

private slots:
    void runAnalysis();

signals:
    void ch1VarianceCalculated(double variance, qsizetype sampleCount);

private:
    std::shared_ptr<SignalHistoryBuffer> history;
    QTimer *analysisTimer = nullptr;
    int analysisIntervalMs = 5000;
};

// 主界面使用的实时信号分析入口。
class SignalAnalysisManager : public QObject {
    Q_OBJECT

public:
    explicit SignalAnalysisManager(QObject *parent = nullptr);
    ~SignalAnalysisManager() override;

    void configureHistory(int sampleRateHz, int historySeconds);
    void setAnalysisInterval(int intervalMs);
    void start();
    void stop();
    void clear();

    [[nodiscard]] std::vector<DataFrame> latestSnapshot() const;

public slots:
    void appendFrames(const QVector<DataFrame> &frames);

signals:
    void ch1VarianceCalculated(double variance, qsizetype sampleCount);

private:
    std::shared_ptr<SignalHistoryBuffer> history;
    QThread analysisThread;
    SignalAnalysisWorker *worker = nullptr;
    std::atomic_bool acceptingFrames = false;
};

#endif //DIY_SIGNAL_ANALYSIS_H
