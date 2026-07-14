//
// Created by Administrator on 26-1-15.
//

#include "diy_oscilloscope-plot.h"

#include <iostream>
#include <QDateTime>

// 顶点着色器：只负责接收坐标并传递给 GPU
static const char *vertexShaderSource = R"(
    attribute highp vec2 posAttr;
    void main() {
        gl_Position = vec4(posAttr, 0.0, 1.0);
    }
)";

// 片段着色器：决定线条的颜色 (纯绿色)
static const char *fragmentShaderSource = R"(
    void main() {
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    }
)";

OscilloscopePlot::OscilloscopePlot(QWidget *parent)
    : QOpenGLWidget(parent), m_vbo(QOpenGLBuffer::VertexBuffer)
{
    m_vertices.resize(X_PLOT_COUNT);
    for (int i = 0; i < X_PLOT_COUNT; ++i) {
        // X 坐标初始化后永远不变 (-1.0 到 1.0)
        m_vertices[i].x = static_cast<float>(i) / (X_PLOT_COUNT - 1) * 2.0f - 1.0f;
        m_vertices[i].y = 0.0f;
    }

    connect(
        &m_timer,
        &QTimer::timeout,
        this,
        [this] {
            update();   // 触发 paintGL()
        }
    );

    // m_timer.start(25); // ~50 FPS
}

OscilloscopePlot::~OscilloscopePlot() {
    makeCurrent();
    if (context()) {
        context()->makeCurrent(context()->surface());
        glFinish();
    }
    m_vao.destroy();
    m_vbo.destroy();
    doneCurrent();
    // delete this;
}

void OscilloscopePlot::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);

    // 1. 编译并链接着色器
    m_shader.addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSource);
    m_shader.addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSource);
    m_shader.link();

    // 2. 创建并绑定 VAO 和 VBO
    m_vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao); // 自动绑定和解绑

    m_vbo.create();
    m_vbo.bind();

    // 给 GPU 开辟显存，并传入初始数据
    // 使用 QOpenGLBuffer::DynamicDraw 告诉显卡：这份数据会被频繁修改！
    m_vbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);
    m_vbo.allocate(m_vertices.data(), m_vertices.size() * sizeof(Vertex));

    // 3. 告诉着色器如何解析这份数据 (解析 posAttr 变量)
    m_shader.bind();
    int posAttr = m_shader.attributeLocation("posAttr");
    m_shader.setAttributeBuffer(posAttr, GL_FLOAT, 0, 2, sizeof(Vertex));
    m_shader.enableAttributeArray(posAttr);
    m_shader.release();

}

void OscilloscopePlot::resizeGL(int, int)
{

}

void OscilloscopePlot::paintGL()
{
    /* Version 1
    // glClear(GL_COLOR_BUFFER_BIT);
    // glBegin(GL_LINE_STRIP);
    //
    // glColor3f(0.0f, 1.0f, 0.0f);
    //
    // for (int i = 0; i < SAMPLE_COUNT; ++i) {
    //     float x = (float)i / 999.0f * 2.0f - 1.0f;      // 0–1000 → -1~1
    //     float y = m_data[i] / 3.3f;                     // -3.3~3.3 → -1~1
    //     glVertex2f(x, y);
    // }
    // glEnd();
    */

    /* // Version 2
    // glClear(GL_COLOR_BUFFER_BIT);
    // glColor3f(0.0f, 1.0f, 0.0f);
    //
    //
    // float displayCopy[X_PLOT_COUNT];
    // int currentIndex = 0;
    //
    // {
    //     std::lock_guard<std::mutex> lock(m_dataMutex);
    //     memcpy(displayCopy, m_data, sizeof(float) * X_PLOT_COUNT);
    //     currentIndex = m_writeIndex;
    //
    // }
    //
    //
    // // 第一段：writeIndex → 999
    // glBegin(GL_LINE_STRIP);
    // for (int i = currentIndex; i < X_PLOT_COUNT; ++i) {
    //     const float x = static_cast<float>(i) / (X_PLOT_COUNT - 1) * 2.0f - 1.0f;
    //     // const float y = m_data[i] / 3.3f;
    //     const float y = m_data[i] / 0.1f;
    //     glVertex2f(x, y);
    // }
    // glEnd();
    // // 第二段：0 → writeIndex-1
    // glBegin(GL_LINE_STRIP);
    // for (int i = 0; i < currentIndex; ++i) {
    //     const float x = static_cast<float>(i) / (X_PLOT_COUNT - 1) * 2.0f - 1.0f;
    //     const float y = m_data[i] / 0.1f;
    //     glVertex2f(x, y);
    // }
    // glEnd();
    */

    // Version 3
    glClear(GL_COLOR_BUFFER_BIT);

    {
        std::lock_guard<std::mutex> lock(m_dataMutex);
        m_vbo.bind();
        // 直接覆盖 GPU 中的内存 (比 glBegin 快几百倍)
        m_vbo.write(0, m_vertices.data(), m_vertices.size() * sizeof(Vertex));
    }

    m_shader.bind();
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    if (m_writeIndex < X_PLOT_COUNT) {
        glDrawArrays(GL_LINE_STRIP, m_writeIndex, X_PLOT_COUNT - m_writeIndex);
    }
    // 第二段：从开头画到 writeIndex-1
    if (m_writeIndex > 0) {
        glDrawArrays(GL_LINE_STRIP, 0, m_writeIndex);
    }

    m_shader.release();

}

void OscilloscopePlot::generateTestSignal()
{
    // // Version 1
    // for (int i = 0; i < SAMPLE_COUNT; ++i) {
    //     m_data[i] = 3.3f * std::sin(2.0f * M_PI * i / 100.0f + m_phase);
    // }
    // m_phase += 0.1f;

    // // Version 2
    // m_data[m_writeIndex] =
    //     3.3f * std::sin(2.0f * M_PI * m_writeIndex / 100.0f + m_phase) + (QRandomGenerator::global()->generateDouble()*0.5f - 0.25f);
    //
    // m_writeIndex++;
    // if (m_writeIndex >= SAMPLE_COUNT)
    //     m_writeIndex = 0;
    //
    // m_phase += 0.1f;
}

void OscilloscopePlot::insertData(const float e_data) {
    // std::lock_guard<std::mutex> lock(m_dataMutex); // 写入时上锁
    m_data[m_writeIndex] = e_data;
    m_writeIndex++;
    if (m_writeIndex >= X_SAVE_COUNT)
        m_writeIndex = 0;

}

void OscilloscopePlot::insertDataBatch(const float *newData, int size) {
    // if (size <= 0) return;
    //
    // std::lock_guard<std::mutex> lock(m_dataMutex);
    //
    // // 如果一次性传来的数据超过了缓冲区总长，只取最后一段
    // if (size > X_SAVE_COUNT) {
    //     newData += (size - X_SAVE_COUNT);
    //     size = X_SAVE_COUNT;
    // }
    //
    // // 分两段拷贝（处理环形缓冲区越界情况）
    // int spaceAtEnd = X_SAVE_COUNT - m_writeIndex;
    // if (size <= spaceAtEnd) {
    //     memcpy(m_data + m_writeIndex, newData, size * sizeof(float));
    //     m_writeIndex = (m_writeIndex + size) % X_SAVE_COUNT;
    // } else {
    //     // 第一部分：写到缓冲区末尾
    //     memcpy(m_data + m_writeIndex, newData, spaceAtEnd * sizeof(float));
    //     // 第二部分：绕回到缓冲区开头
    //     memcpy(m_data, newData + spaceAtEnd, (size - spaceAtEnd) * sizeof(float));
    //     m_writeIndex = size - spaceAtEnd;
    // }

    if (size <= 0) return;

    std::lock_guard<std::mutex> lock(m_dataMutex);

    if (size > X_PLOT_COUNT) {
        newData += (size - X_PLOT_COUNT);
        size = X_PLOT_COUNT;
    }

    // 环形更新 Y 坐标
    for (int i = 0; i < size; ++i) {
        m_vertices[m_writeIndex].y = newData[i]; // 缩放因子
        m_writeIndex = (m_writeIndex + 1) % X_PLOT_COUNT;
    }

}


void OscilloscopePlot::tick() {
    update();
}


OscilloscopeEngine::OscilloscopeEngine(QObject* parent) : QObject(parent)
{
    m_timer.setInterval(15); // ~60 Hz
    connect(&m_timer, &QTimer::timeout, this, &OscilloscopeEngine::onTick);
}

OscilloscopeEngine::~OscilloscopeEngine() {
    clearPlots();
    stop();
}

void OscilloscopeEngine::addPlot(OscilloscopePlot* plot)
{
    m_plots.push_back(plot);
}

void OscilloscopeEngine::removePlot(OscilloscopePlot* plot) {
    if (const int idx = m_plots.indexOf(plot); idx >= 0) {
        m_plots.removeAt(idx);
    }
}

void OscilloscopeEngine::clearPlots() {
    m_plots.clear();
}

void OscilloscopeEngine::start()
{
    m_timer.start();

}

void OscilloscopeEngine::stop()
{
    m_timer.stop();
}

void OscilloscopeEngine::onTick()
{
    // qDebug() << QDateTime::currentMSecsSinceEpoch();

    for (auto* plot : m_plots)
        plot->update();
}
