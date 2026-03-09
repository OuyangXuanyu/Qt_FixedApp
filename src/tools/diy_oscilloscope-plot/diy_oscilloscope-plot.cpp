//
// Created by Administrator on 26-1-15.
//

#include "diy_oscilloscope-plot.h"

OscilloscopePlot::OscilloscopePlot(QWidget *parent)
    : QOpenGLWidget(parent)
{
    // m_data.resize(SAMPLE_COUNT);

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
    doneCurrent();
    delete this;
}

void OscilloscopePlot::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
}

void OscilloscopePlot::resizeGL(int, int)
{

}

void OscilloscopePlot::paintGL()
{
    // Version 1
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

    // Version 2
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(0.0f, 1.0f, 0.0f);
    // 第一段：writeIndex → 999
    glBegin(GL_LINE_STRIP);
    for (int i = m_writeIndex; i < SAMPLE_COUNT; ++i) {
        const float x = float(i) / (SAMPLE_COUNT - 1) * 2.0f - 1.0f;
        const float y = m_data[i] / 3.3f;
        glVertex2f(x, y);
    }
    glEnd();
    // 第二段：0 → writeIndex-1
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i < m_writeIndex; ++i) {
        const float x = float(i) / (SAMPLE_COUNT - 1) * 2.0f - 1.0f;
        const float y = m_data[i] / 3.3f;
        glVertex2f(x, y);
    }
    glEnd();
}

void OscilloscopePlot::generateTestSignal()
{
    // // Version 1
    // for (int i = 0; i < SAMPLE_COUNT; ++i) {
    //     m_data[i] = 3.3f * std::sin(2.0f * M_PI * i / 100.0f + m_phase);
    // }
    // m_phase += 0.1f;

    // Version 2
    m_data[m_writeIndex] =
        3.3f * std::sin(2.0f * M_PI * m_writeIndex / 100.0f + m_phase) + (QRandomGenerator::global()->generateDouble()*0.5f - 0.25f);

    m_writeIndex++;
    if (m_writeIndex >= SAMPLE_COUNT)
        m_writeIndex = 0;

    m_phase += 0.1f;
}

void OscilloscopePlot::insertData(const float e_data) {
    m_data[m_writeIndex] = e_data;
    m_writeIndex++;
    if (m_writeIndex >= SAMPLE_COUNT)
        m_writeIndex = 0;
}

void OscilloscopePlot::tick() {
    update();
}


OscilloscopeEngine::OscilloscopeEngine(QObject* parent) : QObject(parent)
{
    m_timer.setInterval(16); // ~60 Hz
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
    for (auto* plot : m_plots)
        plot->tick();
}
