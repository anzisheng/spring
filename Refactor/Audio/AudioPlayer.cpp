#include "Refactor/Audio/AudioPlayer.h"

AudioPlayer::AudioPlayer(QObject *parent)
    : QObject(parent)
    , m_position(0)
    , m_duration(0)
    , m_state(StoppedState)
    , m_status(NoMedia)
    , m_loaded(false)
{
    m_pollTimer.setInterval(50);
    m_pollTimer.setSingleShot(false);
    connect(&m_pollTimer, &QTimer::timeout, this, &AudioPlayer::pollPlayback);
}

void AudioPlayer::setSystem(FMOD::System *system)
{
    m_backend.setSystem(system);
}

void AudioPlayer::setSource(const QString &filePath)
{
    m_source = filePath;
    m_loaded = false;

    if (filePath.isEmpty())
    {
        updateDurationInternal(0);
        updatePositionInternal(0);
        setMediaStatusInternal(NoMedia);
        return;
    }

    setMediaStatusInternal(LoadingMedia);
    if (!m_backend.load(filePath))
    {
        setMediaStatusInternal(InvalidMedia);
        emit errorOccurred(tr("加载音频文件失败"));
        return;
    }

    m_loaded = true;
    updateDurationInternal(m_backend.duration());
    updatePositionInternal(0);
    setMediaStatusInternal(LoadedMedia);
}

void AudioPlayer::play()
{
    if (!ensureLoaded())
    {
        return;
    }

    if (!m_backend.play())
    {
        emit errorOccurred(tr("启动音频播放失败"));
        return;
    }

    setMediaStatusInternal(LoadedMedia);
    setPlaybackStateInternal(PlayingState);
    if (!m_pollTimer.isActive())
    {
        m_pollTimer.start();
    }
}

void AudioPlayer::pause()
{
    if (!m_loaded)
    {
        return;
    }

    if (!m_backend.pause())
    {
        emit errorOccurred(tr("暂停音频播放失败"));
        return;
    }

    setPlaybackStateInternal(PausedState);
    if (!m_pollTimer.isActive())
    {
        m_pollTimer.start();
    }
}

void AudioPlayer::stop()
{
    if (!m_loaded)
    {
        updatePositionInternal(0);
        setPlaybackStateInternal(StoppedState);
        return;
    }

    if (!m_backend.stop())
    {
        emit errorOccurred(tr("停止音频播放失败"));
        return;
    }

    updatePositionInternal(0);
    setPlaybackStateInternal(StoppedState);
    setMediaStatusInternal(LoadedMedia);
    m_pollTimer.stop();
}

void AudioPlayer::setPosition(qint64 positionMs)
{
    if (!m_loaded)
    {
        updatePositionInternal(positionMs);
        return;
    }

    if (!m_backend.setPosition(positionMs))
    {
        emit errorOccurred(tr("跳转音频位置失败"));
        return;
    }

    updatePositionInternal(positionMs);
}

qint64 AudioPlayer::position() const
{
    return m_position;
}

qint64 AudioPlayer::duration() const
{
    return m_duration;
}

void AudioPlayer::setPlaybackRate(qreal rate)
{
    if (!m_backend.setPlaybackRate(rate))
    {
        emit errorOccurred(tr("设置播放速度失败"));
    }
}

AudioPlayer::PlaybackState AudioPlayer::playbackState() const
{
    return m_state;
}

AudioPlayer::MediaStatus AudioPlayer::mediaStatus() const
{
    return m_status;
}

void AudioPlayer::pollPlayback()
{
    if (!m_loaded)
    {
        return;
    }

    m_backend.update();

    updatePositionInternal(m_backend.position());

    if (m_backend.duration() > 0)
    {
        updateDurationInternal(m_backend.duration());
    }

    const bool playing = m_backend.isPlaying();
    const bool paused = m_backend.isPaused();
    if (m_state == PlayingState && !playing && !paused)
    {
        setMediaStatusInternal(EndOfMedia);
        setPlaybackStateInternal(StoppedState);
        m_pollTimer.stop();
    }
}

void AudioPlayer::setPlaybackStateInternal(PlaybackState state)
{
    if (m_state == state)
    {
        return;
    }

    m_state = state;
    emit playbackStateChanged(state);
}

void AudioPlayer::setMediaStatusInternal(MediaStatus status)
{
    if (m_status == status)
    {
        return;
    }

    m_status = status;
    emit mediaStatusChanged(status);
}

void AudioPlayer::updateDurationInternal(qint64 durationMs)
{
    if (durationMs < 0)
    {
        durationMs = 0;
    }
    if (m_duration == durationMs)
    {
        return;
    }
    m_duration = durationMs;
    emit durationChanged(durationMs);
}

void AudioPlayer::updatePositionInternal(qint64 positionMs)
{
    if (positionMs < 0)
    {
        positionMs = 0;
    }
    if (m_position == positionMs)
    {
        return;
    }
    m_position = positionMs;
    emit positionChanged(positionMs);
}

bool AudioPlayer::ensureLoaded()
{
    if (m_loaded)
    {
        return true;
    }

    if (m_source.isEmpty())
    {
        emit errorOccurred(tr("音频源为空"));
        setMediaStatusInternal(NoMedia);
        return false;
    }

    if (!m_backend.load(m_source))
    {
        setMediaStatusInternal(InvalidMedia);
        emit errorOccurred(tr("加载音频文件失败"));
        return false;
    }

    m_loaded = true;
    updateDurationInternal(m_backend.duration());
    updatePositionInternal(0);
    setMediaStatusInternal(LoadedMedia);
    return true;
}
