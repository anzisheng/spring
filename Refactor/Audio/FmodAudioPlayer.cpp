#include "Refactor/Audio/FmodAudioPlayer.h"

#include <QByteArray>
#include <fmod.hpp>

FmodAudioPlayer::FmodAudioPlayer()
    : m_system(nullptr)
    , m_sound(nullptr)
    , m_channel(nullptr)
    , m_duration(0)
    , m_pendingPosition(0)
    , m_hasPendingPosition(false)
    , m_loaded(false)
    , m_paused(false)
    , m_rate(1.0)
    , m_baseFrequency(0.0f)
{
}

FmodAudioPlayer::~FmodAudioPlayer()
{
    release();
}

void FmodAudioPlayer::setSystem(FMOD::System *system)
{
    if (m_system == system)
    {
        return;
    }

    release();
    m_system = system;
}

bool FmodAudioPlayer::load(const QString &filePath)
{
    if (!m_system || filePath.isEmpty())
    {
        return false;
    }

    release();

    const QByteArray pathBytes = filePath.toUtf8();
    FMOD_RESULT result = m_system->createSound(pathBytes.constData(), FMOD_DEFAULT, nullptr, &m_sound);
    if (result != FMOD_OK)
    {
        m_sound = nullptr;
        return false;
    }

    unsigned int length = 0;
    result = m_sound->getLength(&length, FMOD_TIMEUNIT_MS);
    if (result == FMOD_OK)
    {
        m_duration = static_cast<qint64>(length);
    }
    else
    {
        m_duration = 0;
    }

    m_loaded = true;
    m_paused = false;
    m_baseFrequency = 0.0f;
    m_hasPendingPosition = false;
    m_pendingPosition = 0;
    return true;
}

bool FmodAudioPlayer::play()
{
    if (!m_system || !m_loaded || !m_sound)
    {
        return false;
    }

    if (m_channel && m_paused)
    {
        if (m_channel->setPaused(false) != FMOD_OK)
        {
            return false;
        }
        m_paused = false;
        return true;
    }

    bool playing = false;
    if (m_channel && m_channel->isPlaying(&playing) != FMOD_OK)
    {
        playing = false;
    }

    if (!m_channel || !playing)
    {
        m_channel = nullptr;
        FMOD_RESULT result = m_system->playSound(m_sound, nullptr, true, &m_channel);
        if (result != FMOD_OK)
        {
            return false;
        }

        if (m_channel->getFrequency(&m_baseFrequency) != FMOD_OK)
        {
            m_baseFrequency = 0.0f;
        }
        if (!applyPlaybackRate())
        {
            return false;
        }

        if (m_hasPendingPosition)
        {
            m_channel->setPosition(static_cast<unsigned int>(m_pendingPosition), FMOD_TIMEUNIT_MS);
        }
    }

    if (m_channel->setPaused(false) != FMOD_OK)
    {
        return false;
    }

    m_paused = false;
    return true;
}

bool FmodAudioPlayer::pause()
{
    if (!m_channel)
    {
        return false;
    }

    if (m_channel->setPaused(true) != FMOD_OK)
    {
        return false;
    }

    m_paused = true;
    return true;
}

bool FmodAudioPlayer::stop()
{
    if (m_channel)
    {
        m_channel->stop();
        m_channel = nullptr;
    }

    m_paused = false;
    m_pendingPosition = 0;
    m_hasPendingPosition = true;
    return true;
}

bool FmodAudioPlayer::setPosition(qint64 positionMs)
{
    if (positionMs < 0)
    {
        positionMs = 0;
    }

    m_pendingPosition = positionMs;
    m_hasPendingPosition = true;

    if (!m_channel)
    {
        return true;
    }

    return m_channel->setPosition(static_cast<unsigned int>(positionMs), FMOD_TIMEUNIT_MS) == FMOD_OK;
}

bool FmodAudioPlayer::setPlaybackRate(qreal rate)
{
    if (rate <= 0.0)
    {
        return false;
    }

    m_rate = rate;
    return applyPlaybackRate();
}

qint64 FmodAudioPlayer::position() const
{
    if (m_channel)
    {
        unsigned int position = 0;
        if (m_channel->getPosition(&position, FMOD_TIMEUNIT_MS) == FMOD_OK)
        {
            return static_cast<qint64>(position);
        }
    }

    if (m_hasPendingPosition)
    {
        return m_pendingPosition;
    }

    return 0;
}

qint64 FmodAudioPlayer::duration() const
{
    return m_duration;
}

bool FmodAudioPlayer::isPlaying() const
{
    if (!m_channel)
    {
        return false;
    }

    bool playing = false;
    if (m_channel->isPlaying(&playing) != FMOD_OK)
    {
        return false;
    }
    return playing;
}

bool FmodAudioPlayer::isPaused() const
{
    if (!m_channel)
    {
        return false;
    }

    bool paused = false;
    if (m_channel->getPaused(&paused) != FMOD_OK)
    {
        return false;
    }
    return paused;
}

bool FmodAudioPlayer::isLoaded() const
{
    return m_loaded;
}

void FmodAudioPlayer::update()
{
    if (!m_system)
    {
        return;
    }

    m_system->update();
}

void FmodAudioPlayer::release()
{
    if (m_channel)
    {
        m_channel->stop();
        m_channel = nullptr;
    }

    if (m_sound)
    {
        m_sound->release();
        m_sound = nullptr;
    }

    m_loaded = false;
    m_duration = 0;
    m_baseFrequency = 0.0f;
    m_paused = false;
    m_pendingPosition = 0;
    m_hasPendingPosition = false;
}

bool FmodAudioPlayer::applyPlaybackRate()
{
    if (!m_channel)
    {
        return true;
    }

    if (m_baseFrequency <= 0.0f && m_channel->getFrequency(&m_baseFrequency) != FMOD_OK)
    {
        return false;
    }

    return m_channel->setFrequency(m_baseFrequency * static_cast<float>(m_rate)) == FMOD_OK;
}
