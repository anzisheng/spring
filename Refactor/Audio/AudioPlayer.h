#ifndef AUDIOPLAYER_H
#define AUDIOPLAYER_H

#include <QObject>
#include <QString>
#include <QTimer>

#include "Refactor/Audio/FmodAudioPlayer.h"

class AudioPlayer : public QObject
{
    Q_OBJECT
public:
    enum PlaybackState
    {
        StoppedState,
        PlayingState,
        PausedState
    };
    Q_ENUM(PlaybackState)

    enum MediaStatus
    {
        NoMedia,
        LoadingMedia,
        LoadedMedia,
        EndOfMedia,
        InvalidMedia
    };
    Q_ENUM(MediaStatus)

    explicit AudioPlayer(QObject *parent = nullptr);

    void setSystem(FMOD::System *system);
    void setSource(const QString &filePath);
    void play();
    void pause();
    void stop();
    void setPosition(qint64 positionMs);
    qint64 position() const;
    qint64 duration() const;
    void setPlaybackRate(qreal rate);

    PlaybackState playbackState() const;
    MediaStatus mediaStatus() const;

signals:
    void durationChanged(qint64 durationMs);
    void positionChanged(qint64 positionMs);
    void playbackStateChanged(AudioPlayer::PlaybackState state);
    void mediaStatusChanged(AudioPlayer::MediaStatus status);
    void errorOccurred(const QString &message);

private slots:
    void pollPlayback();

private:
    void setPlaybackStateInternal(PlaybackState state);
    void setMediaStatusInternal(MediaStatus status);
    void updateDurationInternal(qint64 durationMs);
    void updatePositionInternal(qint64 positionMs);
    bool ensureLoaded();

private:
    FmodAudioPlayer m_backend;
    QTimer m_pollTimer;
    QString m_source;
    qint64 m_position;
    qint64 m_duration;
    PlaybackState m_state;
    MediaStatus m_status;
    bool m_loaded;
};

#endif // AUDIOPLAYER_H
