//
// Created by Administrator on 26-7-14.
//

#include "diy_audio.h"

#include <QUrl>

AlarmAudioManager::AlarmAudioManager(QObject *parent) : QObject(parent) {
    registerSound(AlarmType::GeneralWarning,
                  QUrl(QStringLiteral("qrc:/audio/general_warning.wav")));
    registerSound(AlarmType::DeviceDisconnected,
                  QUrl(QStringLiteral("qrc:/audio/device_disconnected.wav")));
    registerSound(AlarmType::TestCompleted,
                  QUrl(QStringLiteral("qrc:/audio/test_completed.wav")));
}

bool AlarmAudioManager::play(const AlarmType type, const bool infiniteLoop) {
    SoundEntry *entry = entryFor(type);
    if (!entry || !entry->effect || entry->effect->status() == QSoundEffect::Error)
        return false;

    // QSoundEffect 的一个实例只负责一种声音，避免同一警报被重复注入。
    if (entry->state != PlaybackState::Stopped || entry->effect->isPlaying())
        return false;

    entry->effect->setLoopCount(infiniteLoop ? QSoundEffect::Infinite : 1);
    setState(type, infiniteLoop ? PlaybackState::Looping : PlaybackState::PlayingOnce);
    entry->effect->play();
    return true;
}

void AlarmAudioManager::stop(const AlarmType type) {
    SoundEntry *entry = entryFor(type);
    if (!entry || !entry->effect)
        return;

    entry->effect->stop();
    setState(type, PlaybackState::Stopped);
}

void AlarmAudioManager::stopAll() {
    for (std::size_t index = 0; index < SoundCount; ++index)
        stop(static_cast<AlarmType>(index));
}

bool AlarmAudioManager::isPlaying(const AlarmType type) const {
    return state(type) != PlaybackState::Stopped;
}

AlarmAudioManager::PlaybackState AlarmAudioManager::state(const AlarmType type) const {
    const SoundEntry *entry = entryFor(type);
    return entry ? entry->state : PlaybackState::Stopped;
}

std::size_t AlarmAudioManager::indexOf(const AlarmType type) {
    return static_cast<std::size_t>(type);
}

void AlarmAudioManager::registerSound(const AlarmType type, const QUrl &source, const qreal volume) {
    SoundEntry *entry = entryFor(type);
    if (!entry)
        return;

    entry->effect = new QSoundEffect(this);
    entry->effect->setSource(source);
    entry->effect->setVolume(volume);

    connect(entry->effect, &QSoundEffect::playingChanged, this, [this, type] {
        const SoundEntry *changedEntry = entryFor(type);
        if (changedEntry && changedEntry->effect && !changedEntry->effect->isPlaying())
            setState(type, PlaybackState::Stopped);
    });

    connect(entry->effect, &QSoundEffect::statusChanged, this, [this, type] {
        const SoundEntry *changedEntry = entryFor(type);
        if (changedEntry && changedEntry->effect
            && changedEntry->effect->status() == QSoundEffect::Error) {
            setState(type, PlaybackState::Stopped);
            emit soundLoadFailed(type);
        }
    });
}

AlarmAudioManager::SoundEntry *AlarmAudioManager::entryFor(const AlarmType type) {
    const std::size_t index = indexOf(type);
    return index < SoundCount ? &sounds[index] : nullptr;
}

const AlarmAudioManager::SoundEntry *AlarmAudioManager::entryFor(const AlarmType type) const {
    const std::size_t index = indexOf(type);
    return index < SoundCount ? &sounds[index] : nullptr;
}

void AlarmAudioManager::setState(const AlarmType type, const PlaybackState newState) {
    SoundEntry *entry = entryFor(type);
    if (!entry || entry->state == newState)
        return;

    entry->state = newState;
    emit soundStateChanged(type, newState);
}
