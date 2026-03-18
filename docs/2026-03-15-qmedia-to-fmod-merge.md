# QMedia 替换为 FMOD 合并实施计划

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 将当前仓库 `MusicWidget` 对 `QMediaPlayer/QAudioOutput` 的直接依赖替换为 FMOD 后端，统一播放、跳转、暂停和结束状态处理路径。

**Architecture:** 从 `D:\workspace\FY25\QGLWater683\QGLWater` 的历史提交 `9e40b18` 提取 `AudioPlayer + FmodAudioPlayer` 适配层，保留当前仓库已经完成的 Qt 6.8.3、UI、时间线、cutline 存储和 `.pro` 发布逻辑，只迁移音频播放抽象与 `MusicWidget` 的调用点。

**Tech Stack:** Qt 6.8.3, qmake, MSVC2022, Qt Widgets, FMOD Core API, C++

---

### Task 1: 落地 FMOD 音频适配层

**Files:**
- Create: `Refactor/Audio/AudioPlayer.h`
- Create: `Refactor/Audio/AudioPlayer.cpp`
- Create: `Refactor/Audio/FmodAudioPlayer.h`
- Create: `Refactor/Audio/FmodAudioPlayer.cpp`
- Modify: `QGLWater.pro`
- Reference: 源提交 `9e40b18`

**Step 1: 从源提交摘取四个音频文件**

保留以下接口形态，不要自行改成第二套风格：

```cpp
class AudioPlayer : public QObject
{
    Q_OBJECT
public:
    enum PlaybackState { StoppedState, PlayingState, PausedState };
    enum MediaStatus { NoMedia, LoadingMedia, LoadedMedia, EndOfMedia, InvalidMedia };

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
};
```

```cpp
class FmodAudioPlayer
{
public:
    void setSystem(FMOD::System *system);
    bool load(const QString &filePath);
    bool play();
    bool pause();
    bool stop();
    bool setPosition(qint64 positionMs);
    bool setPlaybackRate(qreal rate);
    qint64 position() const;
    qint64 duration() const;
    void update();
    void release();
};
```

**Step 2: 适配当前仓库的 FMOD 头文件与字符串规范**

- `FmodAudioPlayer.cpp` 使用当前仓库可编译的 FMOD 头文件路径。
- 优先依赖 `QGLWater.pro` 已有的 `INCLUDEPATH += $$FMOD_INC_DIR`，新文件中直接使用：

```cpp
#include <fmod.hpp>
```

- 所有对外错误信息统一使用 `tr()`，例如：

```cpp
emit errorOccurred(tr("Failed to load audio file"));
```

如果需要新加中文提示，也必须使用 `tr("...")`。

**Step 3: 把新文件注册到 qmake 工程**

只在当前 `.pro` 基础上增量追加，不要回退它现有的输出目录和 post-link 逻辑：

```qmake
SOURCES += \
    Refactor/Audio/AudioPlayer.cpp \
    Refactor/Audio/FmodAudioPlayer.cpp

HEADERS += \
    Refactor/Audio/AudioPlayer.h \
    Refactor/Audio/FmodAudioPlayer.h
```

**Step 4: 先做一次静态扫描确认挂接到位**

Run:

```bash
rg -n "AudioPlayer|FmodAudioPlayer|Refactor/Audio" Refactor/Audio QGLWater.pro
```

Expected:

- 四个新文件都能被扫描到
- `QGLWater.pro` 里出现新的 `SOURCES` / `HEADERS` 条目

**Step 5: 提交这一阶段**

```bash
git add Refactor/Audio/AudioPlayer.h
git add Refactor/Audio/AudioPlayer.cpp
git add Refactor/Audio/FmodAudioPlayer.h
git add Refactor/Audio/FmodAudioPlayer.cpp
git add QGLWater.pro
git commit -m "feat: add FMOD audio player abstraction"
```

### Task 2: 替换 `MusicWidget` 的播放链路

**Files:**
- Modify: `MusicWidget.h`
- Modify: `MusicWidget.cpp`

**Step 1: 替换头文件依赖和成员定义**

把 `MusicWidget.h` 里的 Qt Multimedia 依赖：

```cpp
#include <QAudioOutput>
#include <QMediaPlayer>
```

替换为：

```cpp
#include "Refactor/Audio/AudioPlayer.h"
```

把成员：

```cpp
QMediaPlayer *mediaPlayer;
QAudioOutput *audioOutput;
```

替换为：

```cpp
AudioPlayer *m_audioPlayer;
```

同时补入下面这些接口：

```cpp
void applyMediaPositionFromLine(qint64 position, bool autoPlay);
void onMediaStatusChanged(AudioPlayer::PlaybackState state);
void onMediaEndStatusChanged(AudioPlayer::MediaStatus status);
void onAudioDurationChanged(qint64 duration);
```

**Step 2: 在构造函数中接入 FMOD 播放包装层**

保留当前仓库已有的这些代码不动：

- `sceneBase = new QGraphicsScene(m_MusicView);`
- `sceneBase->setBackgroundBrush(QColor(238, 242, 247));`
- `connect(m_MusicView, ...);`
- `qRegisterMetaType<float*>("float*");`
- `FMODinit();`

把 Qt Multimedia 初始化：

```cpp
audioOutput = new QAudioOutput(this);
mediaPlayer = new QMediaPlayer(this);
mediaPlayer->setAudioOutput(audioOutput);
```

替换为：

```cpp
m_audioPlayer = new AudioPlayer(this);
m_audioPlayer->setSystem(pSystem);
connect(m_audioPlayer, &AudioPlayer::durationChanged, this, &MusicWidget::onAudioDurationChanged);
connect(m_audioPlayer, &AudioPlayer::positionChanged, this, &MusicWidget::onPositionChanged);
connect(m_audioPlayer, &AudioPlayer::playbackStateChanged, this, &MusicWidget::onMediaStatusChanged);
connect(m_audioPlayer, &AudioPlayer::mediaStatusChanged, this, &MusicWidget::onMediaEndStatusChanged);
```

**Step 3: 按功能点替换所有播放调用**

按下面映射替换：

- `mediaPlayer->setSource(QUrl::fromLocalFile(music_path))` -> `m_audioPlayer->setSource(music_path)`
- `mediaPlayer->play()` -> `m_audioPlayer->play()`
- `mediaPlayer->pause()` -> `m_audioPlayer->pause()`
- `mediaPlayer->stop()` -> `m_audioPlayer->stop()`
- `mediaPlayer->setPosition(...)` -> `m_audioPlayer->setPosition(...)`
- `mediaPlayer->position()` -> `m_audioPlayer->position()`
- `mediaPlayer->setPlaybackRate(...)` -> `m_audioPlayer->setPlaybackRate(...)`

必须重写的函数范围：

- `playtrack()`
- `stoptrack()`
- `updatePositionplay()`
- `slotsetmediaposition()`
- `slotmousesetmediaposition()`
- `onMediaStatusChanged()`
- `onMediaEndStatusChanged()`
- `pausetrack()`
- `playFromPosition()`

建议把当前位置换算逻辑统一收口到：

```cpp
void MusicWidget::applyMediaPositionFromLine(qint64 position, bool autoPlay)
```

避免两套跳转逻辑继续分叉。

**Step 4: 明确保留当前仓库逻辑，拒绝整段覆盖**

这一步必须人工检查，以下内容不能被源提交覆盖：

- `saveCutSolidLines/loadCutSolidLines` 当前的“按时间值存储”实现
- 当前仓库的 `m_spWidget != nullptr` 判空
- 当前仓库的 `tr("音乐：%1\n时长：%2 秒")`
- 当前仓库的 `(m_EditView - 30)` 时间宽度修正
- 当前仓库的 `QGraphicsScene` 与 `MusicGraphicsView` 配合方式

明确不要引入：

- `MusicGrapthicsScene`
- `MusicActionController`
- `MusicActionRepository`

**Step 5: 扫描 `MusicWidget`，确认 Qt Multimedia 已清空**

Run:

```bash
rg -n "QMediaPlayer|QAudioOutput|mediaPlayer|audioOutput" MusicWidget.h MusicWidget.cpp
```

Expected:

- 没有匹配结果

**Step 6: 提交这一阶段**

```bash
git add MusicWidget.h
git add MusicWidget.cpp
git commit -m "feat: migrate MusicWidget playback to FMOD"
```

### Task 3: 清理工程级 Qt Multimedia 依赖

**Files:**
- Modify: `QGLWater.pro`
- Verify: `MusicWidget.h`
- Verify: `MusicWidget.cpp`

**Step 1: 先全仓确认 `QMedia` 只剩历史注释或文档**

Run:

```bash
rg -n "QMediaPlayer|QAudioOutput|QMedia|QAudio" . --glob '!docs/**' --glob '!ext/**' --glob '!build/**' --glob '!debug/**' --glob '!release/**' --glob '!vendors/**'
```

Expected:

- 除文档或旧注释外，不再出现编译路径中的 `QMediaPlayer/QAudioOutput`

**Step 2: 删除 `.pro` 中的 Qt Multimedia 模块声明**

从 `QGLWater.pro` 删除：

```qmake
QT += multimedia
```

保留下面这些当前仓库已有配置：

- `ARCH_NAME` 的 x86/x64 判断
- `FMOD_INC_DIR`
- `FMOD_LIB_DIR`
- `QMAKE_POST_LINK`
- `windeployqt --compiler-runtime`

**Step 3: 再做一次工程扫描**

Run:

```bash
rg -n "multimedia|AudioPlayer|FmodAudioPlayer" QGLWater.pro
```

Expected:

- `AudioPlayer` / `FmodAudioPlayer` 条目存在
- 不再出现 `QT += multimedia`

**Step 4: 提交这一阶段**

```bash
git add QGLWater.pro
git commit -m "chore: remove Qt multimedia dependency"
```

### Task 4: 构建与手工回归

**Files:**
- Verify: `QGLWater.pro`
- Verify: `MusicWidget.h`
- Verify: `MusicWidget.cpp`
- Verify: `Refactor/Audio/AudioPlayer.cpp`
- Verify: `Refactor/Audio/FmodAudioPlayer.cpp`

**Step 1: 在 Qt 6.8.3 + MSVC2022 环境重新生成 Makefile**

Run:

```bash
qmake QGLWater.pro
```

Expected:

- Makefile 重新生成成功
- 没有 `QMediaPlayer/QAudioOutput` 相关找不到头文件的问题

**Step 2: 做一次 Release 构建**

Run:

```bash
jom -f Makefile.Release
```

Expected:

- 构建成功
- `build/x64/QGLWater.exe` 或对应 Qt Creator Release 输出产物生成

**Step 3: 做最小手工回归**

检查顺序：

1. 打开一个带音乐的编曲页面
2. 点击播放，确认能正常出声
3. 点击暂停，再恢复播放，确认从暂停位置继续
4. 拖动参考线或点击定位，确认 `m_audioPlayer->setPosition()` 生效
5. 让音乐自然播完，确认 `EndOfMedia` 状态触发并把 `bisplayinit` 复位
6. 检查波形和频谱仍能正常绘制

Expected:

- 没有 `QMedia` 相关运行时依赖缺失
- 没有 FMOD seek/pause 状态错乱
- 时间线和音乐位置仍能对齐

**Step 4: 做一次最终扫描**

Run:

```bash
rg -n "QMediaPlayer|QAudioOutput|mediaPlayer|audioOutput" . --glob '!docs/**' --glob '!ext/**' --glob '!build/**' --glob '!debug/**' --glob '!release/**' --glob '!vendors/**'
```

Expected:

- 无匹配结果

**Step 5: 提交验证后的最终版本**

```bash
git add MusicWidget.h
git add MusicWidget.cpp
git add QGLWater.pro
git add Refactor/Audio/AudioPlayer.h
git add Refactor/Audio/AudioPlayer.cpp
git add Refactor/Audio/FmodAudioPlayer.h
git add Refactor/Audio/FmodAudioPlayer.cpp
git commit -m "feat: unify audio playback on FMOD"
```
