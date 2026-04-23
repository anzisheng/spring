#include "WaterCurtainWidget.h"
#include <QPainter>
#include <QElapsedTimer>
#include <QRandomGenerator>
#include <QtMath>
#include <QDebug>
#include <QKeyEvent>

WaterCurtainWidget::WaterCurtainWidget(QWidget* parent)
    : QWidget(parent)
    , m_particleCount(10000)
    , m_lastTime(0.0f)
    , m_isFalling(false)
    , m_curtainWidth(0)
    , m_curtainHeight(0)
{
    setMinimumSize(800, 600);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setFocusPolicy(Qt::StrongFocus);

    m_updateTimer.setInterval(16);
    connect(&m_updateTimer, &QTimer::timeout, this, &WaterCurtainWidget::updateParticles);
    m_updateTimer.start();
}

WaterCurtainWidget::~WaterCurtainWidget()
{
}

bool WaterCurtainWidget::loadCurtainTexture(const QString& filepath)
{
    if (m_curtainImage.load(filepath)) {
        m_curtainImage = m_curtainImage.convertToFormat(QImage::Format_Grayscale8);
        m_curtainWidth = m_curtainImage.width();
        m_curtainHeight = m_curtainImage.height();
        qDebug() << "Loaded BMP:" << m_curtainWidth << "x" << m_curtainHeight;
        initParticles();
        return true;
    }
    qDebug() << "No BMP loaded, using default heart pattern";
    initParticles();
    return false;
}

void WaterCurtainWidget::setParticleCount(int count)
{
    m_particleCount = count;
    initParticles();
}

bool WaterCurtainWidget::shouldSpawnAt(float x, float z) const
{
    if (m_curtainImage.isNull()) {
        float heartX = x / 2.5f;
        float heartZ = z / 2.0f;
        float heartFormula = pow(heartX * heartX + heartZ * heartZ - 1, 3) - heartX * heartX * heartZ * heartZ * heartZ;
        return heartFormula < 0;
    }

    float u = (x + 5.0f) / 10.0f;
    float v = (z + 4.0f) / 8.0f;

    if (u < 0 || u > 1 || v < 0 || v > 1) return false;

    int ix = qBound(0, static_cast<int>(u * m_curtainWidth), m_curtainWidth - 1);
    int iy = qBound(0, static_cast<int>(v * m_curtainHeight), m_curtainHeight - 1);

    int gray = qGray(m_curtainImage.pixel(ix, iy));
    float density = gray / 255.0f;

    return density > 0.2f;
}

void WaterCurtainWidget::initParticles()
{
    m_particles.clear();

    float xStart = -5.0f;
    float xEnd = 5.0f;
    float zStart = -4.0f;
    float zEnd = 4.0f;

    int gridX = 140;
    int gridZ = 112;

    float stepX = (xEnd - xStart) / gridX;
    float stepZ = (zEnd - zStart) / gridZ;

    for (int ix = 0; ix < gridX && m_particles.size() < m_particleCount; ++ix) {
        for (int iz = 0; iz < gridZ && m_particles.size() < m_particleCount; ++iz) {
            float x = xStart + ix * stepX + stepX / 2;
            float z = zStart + iz * stepZ + stepZ / 2;

            if (shouldSpawnAt(x, z)) {
                Particle p;
                p.x = x;
                p.z = z;
                p.y = 0.0f;
                p.vy = 0.0f;
                p.active = true;
                m_particles.append(p);
            }
        }
    }

    m_particleCount = m_particles.size();
    qDebug() << "Initialized" << m_particleCount << "particles";
}

void WaterCurtainWidget::startFalling()
{
    for (int i = 0; i < m_particles.size(); ++i) {
        m_particles[i].active = true;
        m_particles[i].y = 4.5f;
        m_particles[i].vy = -3.0f - QRandomGenerator::global()->generateDouble() * 4.0f;
    }
    m_isFalling = true;
    m_lastTime = QElapsedTimer().nsecsElapsed() / 1e9f;
    qDebug() << "Start falling!";
}

void WaterCurtainWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Space) {
        if (m_isFalling) {
            for (int i = 0; i < m_particles.size(); ++i) {
                m_particles[i].active = true;
                m_particles[i].y = 4.5f;
                m_particles[i].vy = -3.0f - QRandomGenerator::global()->generateDouble() * 4.0f;
            }
            m_lastTime = QElapsedTimer().nsecsElapsed() / 1e9f;
            qDebug() << "Reset falling!";
        }
        else {
            startFalling();
        }
    }
}

void WaterCurtainWidget::updateParticles()
{
    if (!m_isFalling) {
        return;
    }

    float now = QElapsedTimer().nsecsElapsed() / 1e9f;
    float deltaTime = now - m_lastTime;
    m_lastTime = now;

    if (deltaTime > 0.033f) deltaTime = 0.033f;
    if (deltaTime < 0.001f) deltaTime = 0.016f;

    float gravity = 45.0f;

    int activeCount = 0;

    for (int i = 0; i < m_particles.size(); ++i) {
        Particle& p = m_particles[i];

        if (!p.active) continue;

        p.vy += gravity * deltaTime;
        p.y += p.vy * deltaTime;

        // 超出底部则消失
        if (p.y < -5.0f) {
            p.active = false;
        }
        else {
            activeCount++;
        }
    }

    if (activeCount == 0 && m_isFalling) {
        m_isFalling = false;
        qDebug() << "All particles have fallen!";
    }

    update();
}

void WaterCurtainWidget::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), QColor(5, 5, 15));

    int w = width();
    int h = height();

    float marginX = w * 0.18f;
    float marginY = h * 0.18f;
    float drawW = w - 2 * marginX;
    float drawH = h - 2 * marginY;

    float scaleX = drawW / 10.0f;
    float scaleZ = drawH / 8.0f;

    int activeCount = 0;

    for (const auto& p : m_particles) {
        if (!p.active) continue;
        activeCount++;

        // 屏幕X坐标
        float screenX = marginX + (p.x + 5.0f) * scaleX;

        // 基础Y坐标（根据Z）
        float baseY = marginY + (4.0f - p.z) * scaleZ;

        float screenY;
        if (m_isFalling) {
            // 基础Y位置（根据Z）
            float baseY = marginY + (4.0f - p.z) * scaleZ;
            // 下落偏移：Y值从4.5降到-5，偏移量从0增加到最大值
            float t = (4.5f - p.y) / 9.5f;  // p.y=4.5时t=0，p.y=-5时t=1
            t = qBound(0.0f, t, 1.0f);
            float yOffset = t * drawH * 0.5f;  // 最大偏移屏幕高度的50%
            screenY = baseY + yOffset;
        }
        else {
            screenY = marginY + (4.0f - p.z) * scaleZ;
        }

        if (screenX < -20 || screenX > w + 20 || screenY < -20 || screenY > h + 20) continue;

        float density = 0.5f;
        if (!m_curtainImage.isNull()) {
            float u = (p.x + 5.0f) / 10.0f;
            float v = (p.z + 4.0f) / 8.0f;
            if (u >= 0 && u <= 1 && v >= 0 && v <= 1) {
                int ix = qBound(0, static_cast<int>(u * m_curtainWidth), m_curtainWidth - 1);
                int iy = qBound(0, static_cast<int>(v * m_curtainHeight), m_curtainHeight - 1);
                density = qGray(m_curtainImage.pixel(ix, iy)) / 255.0f;
            }
        }

        density = qBound(0.15f, density, 1.0f);

        float size = 2.5f + density * 4.5f;

        int r = 40 + (int)(density * 70);
        int g = 90 + (int)(density * 90);
        int b = 170 + (int)(density * 85);

        int alpha = 200;
        if (m_isFalling) {
            // 下落越远越透明
            float heightFactor = (p.y + 5.0f) / 9.5f;
            heightFactor = qBound(0.2f, heightFactor, 1.0f);
            alpha = 100 + (int)(heightFactor * 100);
        }
        alpha = qBound(80, alpha, 240);

        painter.setBrush(QColor(r, g, b, alpha));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(QPointF(screenX, screenY), size, size);
    }

    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 11));
    painter.drawText(10, 25, "Water Curtain - BMP Shape Falling Effect");
    painter.drawText(10, 48, QString("Particles: %1 / %2").arg(activeCount).arg(m_particles.size()));

    if (!m_curtainImage.isNull()) {
        painter.drawText(10, 71, QString("BMP: %1 x %2").arg(m_curtainWidth).arg(m_curtainHeight));
    }
    else {
        painter.drawText(10, 71, "No BMP - Using heart shape");
    }

    if (m_isFalling) {
        painter.setPen(QColor(0, 255, 100));
        painter.drawText(10, 130, "▼ FALLING - Press SPACE to reset");
        if (activeCount > 0) {
            int percent = (int)((float)activeCount / m_particles.size() * 100);
            painter.drawText(10, 153, QString("Remaining: %1%").arg(percent));
        }
    }
    else {
        if (activeCount == 0 && m_particles.size() > 0) {
            painter.setPen(QColor(255, 200, 0));
            painter.drawText(10, 130, "● ALL PARTICLES FELL - Press SPACE to reset");
        }
        else {
            painter.setPen(QColor(255, 200, 0));
            painter.drawText(10, 130, "● STATIC - Press SPACE to start falling");
        }
    }
}