//
// Created by Administrator on 26-7-15.
//

#ifndef DIY_SIGNAL_DATA_H
#define DIY_SIGNAL_DATA_H

#include <QMetaType>

inline constexpr int SIGNAL_CHANNEL_COUNT = 9;
inline constexpr int SIGNAL_SAMPLE_RATE_HZ = 200;

// 统一的九通道软件帧，供解析、保存、绘图和实时分析共同使用。
struct DataFrame {
    float data[SIGNAL_CHANNEL_COUNT];
};
Q_DECLARE_METATYPE(DataFrame)

#endif //DIY_SIGNAL_DATA_H
