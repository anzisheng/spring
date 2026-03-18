# QMedia 替换为 FMOD 合并设计

## 1. 背景与结论

当前主工程是 `D:\workspace\FY25\QGLWater-Src-2026-01-16_SL40 - Clock\Src-2026-01-16`，参考源工程是 `D:\workspace\FY25\QGLWater683\QGLWater`。本次目标不是继续保留 `QMediaPlayer + QAudioOutput`，而是把播放链路统一到 FMOD，避免一套工程里同时维护 Qt Multimedia 和 FMOD 两种音频实现。

本次排查后的关键结论有两个：

1. 当前主工程里真正仍在使用 `QMediaPlayer/QAudioOutput` 的代码只剩 `MusicWidget.h` 和 `MusicWidget.cpp`，工程文件里的 `QT += multimedia` 也是为这一处服务。
2. 参考源工程当前工作树 `main@fcd0462` 里并没有“完全切到 FMOD”的结果，真正可作为来源的是它的历史提交 `9e40b18`，提交标题为 `QMedia play 替换`。这个提交引入了 `Refactor/Audio/AudioPlayer.*` 和 `Refactor/Audio/FmodAudioPlayer.*`，并把 `MusicWidget` 从直接依赖 `QMediaPlayer` 改成依赖 FMOD 后端。

因此，这次特性的正确合并单位不是“源工程当前目录”，而是“源工程历史提交 `9e40b18` 里的音频抽象层改造”。

## 2. 推荐合并策略

推荐方案是“手工摘取音频抽象层，保留当前主工程现状”，不建议直接 cherry-pick 整个 `9e40b18`。

推荐原因：

- `9e40b18` 除了音频替换，还同时改动了 `MusicGrapthicsScene`、`MusicActionController`、`MusicActionRepository`、`mainwindow.cpp` 等重构文件。
- 当前主工程已经有自己的一套稳定实现：
  - `MusicGraphicsView + QGraphicsScene`
  - 右侧留白 `-30` 的时间线宽度修正
  - `saveCutSolidLines/loadCutSolidLines` 的时间值存储逻辑
  - `m_spWidget` 空指针保护
  - Qt 6.8.3 下的 `.pro` 输出目录与 post-link 拷贝链路
- 如果整提交合入，很容易把当前主工程已经验证过的 Qt6/UI/交互修正一起覆盖掉。

不推荐的两种方案：

- 直接 cherry-pick `9e40b18`
  - 风险最高，容易把不相关的场景重构一起带入。
- 保持现状，继续让 FMOD 只负责波形和 DSP，播放仍用 QMedia
  - 不能达成“统一多媒体库”的目标，后续问题仍会在两套实现之间来回排查。

## 3. 目标架构

目标结构应当是：

- `MusicWidget`
  - 只面向 `AudioPlayer` 工作
  - 不再直接引用 `QMediaPlayer/QAudioOutput`
- `AudioPlayer`
  - 提供和现有 `MusicWidget` 兼容的状态枚举、位置、时长、播放、暂停、停止、跳转能力
  - 负责把 FMOD 后端包装成接近 Qt Multimedia 的信号模型
- `FmodAudioPlayer`
  - 直接操作 `FMOD::System / FMOD::Sound / FMOD::Channel`
  - 负责加载、播放、暂停、停止、跳转、倍速
- `MusicWidget::FMODinit()`
  - 继续作为 FMOD `System` 的初始化入口
  - 同一个 `pSystem` 继续同时服务给：
    - `Track` 波形解析路径
    - `AudioPlayer/FmodAudioPlayer` 播放路径

这样做以后，工程里的音频职责会收敛为一条链路：播放、跳转、结束状态、倍速全部走 FMOD；Qt Multimedia 可以从业务代码里退出。

## 4. 本次 merge 的边界

建议只处理以下文件：

- 新增
  - `Refactor/Audio/AudioPlayer.h`
  - `Refactor/Audio/AudioPlayer.cpp`
  - `Refactor/Audio/FmodAudioPlayer.h`
  - `Refactor/Audio/FmodAudioPlayer.cpp`
- 修改
  - `MusicWidget.h`
  - `MusicWidget.cpp`
  - `QGLWater.pro`

明确不要跟这次特性一起合入的文件：

- `MusicGrapthicsScene.*`
- `MusicGraphicsView.cpp`
- `mainwindow.cpp`
- `lineitem.cpp`
- `Refactor/Controllers/*`
- `Refactor/Repository/*`
- `Refactor/Services/*`
- 任何只属于 `9e40b18` 场景重构的文件

这些文件并不是“QMedia 替换为 FMOD”这件事的必要条件，混进来只会抬高冲突成本。

## 5. 已知冲突点

### 5.1 `MusicWidget` 的场景类型已经不同

`9e40b18` 的 `MusicWidget` 使用 `MusicGrapthicsScene* sceneBase`，当前主工程使用的是 `QGraphicsScene* sceneBase`，并把交互逻辑留在 `MusicGraphicsView` 上。因此只能迁入音频播放相关代码，不能整段替换构造函数和场景连接代码。

### 5.2 cutline 存储逻辑必须保留当前实现

当前主工程的 `saveCutSolidLines/loadCutSolidLines` 已经改成按“时间值”存储，和 `9e40b18` 中按比例值存储不同。这里必须保留当前主工程实现，否则旧文件兼容性和现有时间换算会回退。

### 5.3 `.pro` 里的 FMOD 路径必须保留当前实现

当前主工程已经把 FMOD 库路径切到：

- `ext/FMOD/inc`
- `ext/FMOD/lib/x86`
- `ext/FMOD/lib/x64`

并补齐了 `cfgtemp`、`dark.qss`、DLL 拷贝和 `windeployqt`。这些都是 Qt 6.8.3 迁移后的稳定结果，不能回退到 `9e40b18` 里那套 `../vendors/fmod11020` 布局。

### 5.4 需要保留当前主工程的 UI 修正

以下行为属于当前主工程的有效修正，不应因本次 merge 丢失：

- `sceneBase->setBackgroundBrush(QColor(238, 242, 247))`
- 时间线和编曲区域使用 `(m_EditView - 30)` 的宽度修正
- `m_spWidget != nullptr` 的保护判断
- 中文字面量统一逐步改成 `tr()`

## 6. 验收标准

这次 merge 完成后，至少要满足以下条件：

1. 业务代码中不再直接出现 `QMediaPlayer`、`QAudioOutput`。
2. `MusicWidget` 的播放、暂停、停止、拖动跳转、空格续播、自然播完回调全部仍可用。
3. `Track` 波形绘制和现有 FMOD DSP 初始化不回退。
4. `QGLWater.pro` 继续保持当前 Qt 6.8.3 的输出目录和 post-link 逻辑。
5. `QT += multimedia` 能删除，且删除后构建仍通过。

## 7. 推荐执行顺序

推荐按下面顺序实施：

1. 先把 `Refactor/Audio` 四个文件迁入当前主工程。
2. 再把 `MusicWidget` 从 `QMediaPlayer` 改成 `AudioPlayer`。
3. 最后再清理 `QT += multimedia` 和遗留注释。

这个顺序的好处是：每一步都可以单独编译和回归，不会把“引入音频抽象层”和“替换业务调用点”混在一起排错。
