//
// Created by Administrator on 26-1-15.
//

#ifndef DIY_OSCILLOSCOPE_PLOT_H
#define DIY_OSCILLOSCOPE_PLOT_H

#include <cmath>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QTimer>
#include <QRandomGenerator>
#include <vector>

#include <gl/GL.h>

class OscilloscopePlot : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit OscilloscopePlot(QWidget *parent = nullptr);
    ~OscilloscopePlot() override;

    void insertData(float e_data);
    void tick();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    static constexpr int SAMPLE_COUNT = 1000;

    QTimer m_timer;  // 图像刷新率（默认60Hz）
    // // Version 1
    // std::vector<float> m_data;

    // Version 2
    float m_data[SAMPLE_COUNT] = {0.0f};
    int   m_writeIndex = 0;   // 当前写入位置

    float m_phase = 0.0f;

    void generateTestSignal();

};

class OscilloscopeEngine : public QObject
{
    Q_OBJECT
public:
    explicit OscilloscopeEngine(QObject* parent = nullptr);
    ~OscilloscopeEngine() override;
    void addPlot(OscilloscopePlot* plot);
    void removePlot(OscilloscopePlot* plot);
    void clearPlots();
    void start();
    void stop();
    QVector<OscilloscopePlot*> m_plots;

private slots:
    void onTick();

private:
    QTimer m_timer;
};

#endif //DIY_OSCILLOSCOPE_PLOT_H
