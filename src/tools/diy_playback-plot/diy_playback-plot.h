//
// Created by Administrator on 26-1-20.
//

#ifndef DIY_PLAYBACK_PLOT_H
#define DIY_PLAYBACK_PLOT_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QTimer>
#include <QRandomGenerator>
#include <vector>

#include <gl/GL.h>

class PlaybackPlot : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit PlaybackPlot(QWidget *parent = nullptr);
    ~PlaybackPlot() override;

    void updateData();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

public:
    // 找文件
    bool findSourceSignalPath(uint32_t id, const std::string &name, int timestamp, uint8_t stage);
    std::string source_signal_path;
    //读文件
    bool loadSourceSignalPath(uint32_t start, uint32_t length);
    // todo: 开变量->写入某一片缓存(保留MAX_LENGTH)

};

class PlaybackEngine : QObject {
    Q_OBJECT
public:
    explicit PlaybackEngine(QObject* parent = nullptr);
    ~PlaybackEngine() override;

    void addPlot(PlaybackPlot* plot);
    void removePlot(PlaybackEngine* plot);
    void hidePlot(PlaybackPlot* plot);

    void clearPlot();
    void stop();

    QVector<PlaybackPlot*> m_plots;

private slots:
    void onTick();

private:
    QTimer m_timer;
    QVector<QVector<float>> plot_data_array;
    QVector<QVector<float>> readF32FileRange(
        int start_row,
        const QString& bin_file,
        int signal_length,
        int N_COLS
        );
};

/* 流程：
 * 传入绘图信号(id, name, timestamp, stage):
 * 找文件findSourceSignalPath(.f32)
 *
 */
#endif //DIY_PLAYBACK_PLOT_H
