//
// Created by Administrator on 26-1-20.
//

#include "diy_playback-plot.h"

PlaybackPlot::PlaybackPlot(QWidget *parent) : QOpenGLWidget(parent) {
    source_signal_path = "";
}

PlaybackPlot::~PlaybackPlot() {
    delete this;
}

void PlaybackPlot::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.80f, 0.80f, 0.80f, 1.0f);
}

void PlaybackPlot::resizeGL(int, int)
{

}

void PlaybackPlot::paintGL() {
    glClear(GL_COLOR_BUFFER_BIT);
    // 画笔颜色
    glColor3f(0.0f, 1.0f, 0.0f);

}

bool PlaybackPlot::findSourceSignalPath(const uint32_t id, const std::string &name, const int timestamp, const uint8_t stage) {
    source_signal_path;  // 组装路径
    if (source_signal_path.isExist()) {
        return true;
    }
    return false;
}

bool PlaybackPlot::loadSourceSignalPath(const uint32_t start, const uint32_t length) {
    try {
        // 某个变量尝试写入内容(.f32)
    }
    catch (const std::exception &e) {
        return false
    }
}