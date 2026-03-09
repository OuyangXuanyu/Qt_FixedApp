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

private:
    // 找文件
    bool findSourceSignalPath(uint32_t id, const std::string &name, int timestamp, uint8_t stage);
    std::string source_signal_path;
    //读文件
    bool loadSourceSignalPath(uint32_t start, uint32_t length);
    // todo: 开变量->写入某一片缓存(保留MAX_LENGTH)
    // 底层绘制私有接口
    void drawPrivate();

public:
    void drawPublic();

};

/* 流程：
 * 传入绘图信号(id, name, timestamp, stage):
 * 找文件findSourceSignalPath(.f32)
 *
 */
#endif //DIY_PLAYBACK_PLOT_H
