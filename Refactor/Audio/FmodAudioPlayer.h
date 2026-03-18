#ifndef FMODAUDIOPLAYER_H
#define FMODAUDIOPLAYER_H

#include <QString>

namespace FMOD
{
    class Channel;
    class Sound;
    class System;
}

class FmodAudioPlayer
{
public:
    FmodAudioPlayer();
    ~FmodAudioPlayer();

    void setSystem(FMOD::System *system);

    bool load(const QString &filePath);
    bool play();
    bool pause();
    bool stop();
    bool setPosition(qint64 positionMs);
    bool setPlaybackRate(qreal rate);

    qint64 position() const;
    qint64 duration() const;

    bool isPlaying() const;
    bool isPaused() const;
    bool isLoaded() const;

    void update();
    void release();

private:
    bool applyPlaybackRate();

private:
    FMOD::System *m_system;
    FMOD::Sound *m_sound;
    FMOD::Channel *m_channel;

    qint64 m_duration;
    qint64 m_pendingPosition;
    bool m_hasPendingPosition;
    bool m_loaded;
    bool m_paused;

    qreal m_rate;
    float m_baseFrequency;
};

#endif // FMODAUDIOPLAYER_H
