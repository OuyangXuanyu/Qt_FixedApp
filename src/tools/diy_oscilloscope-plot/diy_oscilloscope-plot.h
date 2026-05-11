//
// Created by Administrator on 26-1-15.
//

#ifndef DIY_OSCILLOSCOPE_PLOT_H
#define DIY_OSCILLOSCOPE_PLOT_H

#include <cmath>
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>


#include <QTimer>
#include <QRandomGenerator>

#include <vector>
#include <mutex>

#include <gl/GL.h>

struct Vertex {
    float x;
    float y;
};

class OscilloscopePlot : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit OscilloscopePlot(QWidget *parent = nullptr);
    ~OscilloscopePlot() override;

    void insertData(float e_data);
    void insertDataBatch(const float* newData, int size);
    void tick();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

private:
    QOpenGLShaderProgram m_shader;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_vbo;

    std::vector<Vertex> m_vertices;

    std::mutex m_dataMutex;
    int   m_writeIndex = 0;   // 当前写入位置

    static constexpr int X_PLOT_COUNT = 2000;
    static constexpr int X_SAVE_COUNT = 2000;

    QTimer m_timer;  // 图像刷新率（默认60Hz）
    // // Version 1
    // std::vector<float> m_data;

    // 数据互斥锁

    // Version 2
    float m_data[X_SAVE_COUNT] = {0.0f};

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
