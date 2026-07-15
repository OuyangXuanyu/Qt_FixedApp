//
// Created by Administrator on 26-7-14.
//

#ifndef DIY_AUDIO_H
#define DIY_AUDIO_H

#include <array>
#include <cstddef>

#include <QObject>
#include <QSoundEffect>

// 警报声音管理类
class AlarmAudioManager : public QObject {
    Q_OBJECT

public:
    enum class AlarmType {
        GeneralWarning,
        DeviceDisconnected,
        TestCompleted,
        Count
    };
    Q_ENUM(AlarmType)

    enum class PlaybackState {
        Stopped,
        PlayingOnce,
        Looping
    };
    Q_ENUM(PlaybackState)

    explicit AlarmAudioManager(QObject *parent = nullptr);
    ~AlarmAudioManager() override = default;

    // 同一种声音正在播放时不会重复启动，不同种类的声音可以同时播放。
    bool play(AlarmType type, bool infiniteLoop);
    void stop(AlarmType type);
    void stopAll();

    [[nodiscard]] bool isPlaying(AlarmType type) const;
    [[nodiscard]] PlaybackState state(AlarmType type) const;

signals:
    void soundStateChanged(AlarmType type, PlaybackState state);
    void soundLoadFailed(AlarmType type);

private:
    struct SoundEntry {
        QSoundEffect *effect = nullptr;
        PlaybackState state = PlaybackState::Stopped;
    };

    static constexpr std::size_t SoundCount = static_cast<std::size_t>(AlarmType::Count);
    std::array<SoundEntry, SoundCount> sounds{};

    static std::size_t indexOf(AlarmType type);
    void registerSound(AlarmType type, const QUrl &source, qreal volume = 0.8);
    SoundEntry *entryFor(AlarmType type);
    const SoundEntry *entryFor(AlarmType type) const;
    void setState(AlarmType type, PlaybackState state);
};

#endif //DIY_AUDIO_H
