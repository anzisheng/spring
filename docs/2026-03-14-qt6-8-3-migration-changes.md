# QGLWater Qt 6.8.3 迁移修改记录

## 文档目的

本文档记录本次将 `QGLWater` 从旧版 Qt 环境迁移到 `Qt 6.8.3 + MSVC2022 64bit` 时，已经落地的关键修改位置，方便后续继续迁移、排查回归和做代码审阅。

## 本次迁移的目标环境

- Qt: `6.8.3`
- 编译器: `MSVC2022 64bit`
- 当前 qmake 实际编译标准: `-std=c++17`
- 工程文件: `QGLWater.pro`
- 源码文件编码约定: `*.cpp` / `*.h` 使用 `UTF-8 BOM`

## 已落实的迁移修改

### 0. 项目自有 `cpp/h` 已统一为 `UTF-8 BOM`

目的：
- 统一源码编码，避免 Qt6/MSVC 下因为旧中文注释或历史编码混杂导致的编译告警。

处理范围：
- 项目自有源码文件
- 排除目录：
  - `build/`
  - `debug/`
  - `release/`
  - `GeneratedFiles/`
  - `vendors/`
  - `ext/`
  - `Win32/`

处理结果：
- 已统一处理 `101` 个项目自有 `*.cpp` / `*.h`
- 当前剩余“无 BOM”的项目自有源码文件数量为 `0`
- 其中原先含旧中文编码痕迹的文件，已按 `GB18030 -> UTF-8 BOM` 转换

### 1. qcustomplot 切换到仓库内第三方目录

目的：
- 不再修改旧第三方源码副本。
- 工程统一引用仓库内的 `vendors/qcustomplot`。

修改位置：
- `QGLWater.pro`
  - `vendors/qcustomplot/qcustomplot.cpp`
  - `vendors/qcustomplot/qcustomplot.h`
  - `INCLUDEPATH += $$PWD/vendors/qcustomplot`
  - `DEPENDPATH += $$PWD/vendors/qcustomplot`
- `spectrumwidget.ui`
  - custom widget header 改为 `qcustomplot.h`

对应位置：
- `QGLWater.pro:53`
- `QGLWater.pro:99`
- `QGLWater.pro:148`
- `QGLWater.pro:149`
- `spectrumwidget.ui:37`

### 2. 去除 Qt5 的 `QTextCodec` 依赖

目的：
- 适配 Qt6 中已经移除或不建议继续依赖的文本编码接口。

修改位置：
- `main.cpp`
  - 移除 `QTextCodec` 初始化路径。
- `QFileHelper.cpp`
  - 使用 `QStringDecoder(QStringConverter::System)` 读取本地文本。
- `mainwindow.cpp`
  - 保留的 `QTextCodec` 只剩下注释，不再参与编译逻辑。

对应位置：
- `QFileHelper.cpp:105`

### 3. 主程序启动细节兼容 Qt6

目的：
- 修复 Qt6/MSVC 下的直接编译错误。

修改位置：
- `main.cpp`
  - 日志输出去掉 `endl` 的旧写法，避免 `endl` 未声明错误。
  - `QTranslator::load()` 返回值显式接住，避免 `[[nodiscard]]` 警告升级为问题。

对应位置：
- `main.cpp:39`
- `main.cpp:43`
- `main.cpp:158`
- `main.cpp:159`

### 4. 音频播放从旧多媒体写法迁到 Qt6 可用接口

目的：
- 适配 Qt6 多媒体模块接口变化。

修改位置：
- `MusicWidget.h`
  - 使用 `QAudioOutput`
  - 使用 `QMediaPlayer`
- `MusicWidget.cpp`
  - 构造时创建 `QAudioOutput`
  - `mediaPlayer->setAudioOutput(audioOutput)`

说明：
- 这部分迁移承接了原先去掉 `QMediaPlaylist` 的改造，当前代码已基于 `QMediaPlayer + QAudioOutput` 运行。

对应位置：
- `MusicWidget.h:8`
- `MusicWidget.h:133`
- `MusicWidget.cpp:32`

### 5. `QTimeLine` 旧曲线接口迁移到 Qt6

目的：
- 修复 Qt6 中 `setCurveShape(QTimeLine::LinearCurve)` 不再可用的问题。

修改位置：
- `MusicWidget.cpp`
  - 引入 `QEasingCurve`
  - 使用 `m_timeline->setEasingCurve(QEasingCurve::Linear)`

对应位置：
- `MusicWidget.cpp:5`
- `MusicWidget.cpp:275`

### 6. `qStableSort` / `qSort` 迁移到标准库排序

目的：
- Qt6 中旧排序辅助函数已经不可继续依赖。

修改位置：
- `MusicGraphicsView.cpp`
  - `qStableSort(...)` 改为 `std::stable_sort(...)`
- `mainwindow.cpp`
  - 活动代码中的 `qSort(route_id_list.begin(), route_id_list.end())`
    统一改为 `std::sort(...)`

对应位置：
- `MusicGraphicsView.cpp:19`
- `MusicGraphicsView.cpp:587`
- `mainwindow.cpp:1183`
- `mainwindow.cpp:2178`
- `mainwindow.cpp:2234`
- `mainwindow.cpp:2520`
- `mainwindow.cpp:3208`

### 7. `QString` / 枚举接口的 Qt6 兼容调整

目的：
- 修复 Qt6 下字符串拼接和分割接口变化导致的编译错误。

修改位置：
- `mainwindow.cpp`
  - `QString + float` 改为 `tr(...).arg(...).arg(QString::number(...))`
  - `QString::SkipEmptyParts` 改为 `Qt::SkipEmptyParts`

对应位置：
- `mainwindow.cpp:667`
- `mainwindow.cpp:5238`

### 8. 桌面尺寸接口迁移到 `QScreen`

目的：
- 去掉 Qt5 时代 `QDesktopWidget` / `QApplication::desktop()` 依赖。

修改位置：
- `mainwindow.cpp`
- `drawtabledialog.cpp`
- `CutSolidVerticalLine.cpp`

当前用法：
- `QGuiApplication::primaryScreen()`

对应位置：
- `mainwindow.cpp:168`
- `drawtabledialog.cpp:16`
- `CutSolidVerticalLine.cpp:47`

### 9. 移除 `mainwindow.cpp` 中未使用的 QtConcurrent 头

目的：
- 避免无实际使用却触发 `QtConcurrent` 模板展开错误。

背景：
- 构建时曾在 `QtConcurrent/qtconcurrentreducekernel.h(120)` 报错。
- `mainwindow.cpp` 实际并未使用 `QFuture` / `QtConcurrent`。

修改位置：
- `mainwindow.cpp`
  - 删除未使用的 `#include <QFuture>`
  - 删除未使用的 `#include <QtConcurrent/QtConcurrentRun>`
  - 删除未使用的 `#include <QtConcurrent>`

对应位置：
- `mainwindow.cpp:77-80` 附近

### 10. 把 `ClockDialog` 补回 qmake 工程

目的：
- 修复链接阶段 `ClockDialog` 构造、析构和 `setData()` 未解析符号。

修改位置：
- `QGLWater.pro`
  - 添加 `ClockDialog.cpp`
  - 添加 `ClockDialog.h`
  - 添加 `ClockDialog.ui`

对应位置：
- `QGLWater.pro:39`
- `QGLWater.pro:85`
- `QGLWater.pro:131`

### 11. FMOD 链接改为按目标架构选择库目录

目的：
- 修复 `MSVC2022 64bit` 构建时误链接 `ext/FMOD/lib/x86/fmod_vc.lib` 的问题。

背景：
- 之前链接阶段出现：
  - `library machine type 'x86' conflicts with target machine type 'x64'`

修改位置：
- `QGLWater.pro`
  - 新增 `FMOD_LIB_DIR`
  - `i386` 使用 `ext/FMOD/lib/x86`
  - 其余 Windows 架构使用 `ext/FMOD/lib/x64`
  - 头文件路径统一改为 `ext/FMOD/inc`

对应位置：
- `QGLWater.pro:136`
- `QGLWater.pro:137`
- `QGLWater.pro:139`
- `QGLWater.pro:143`
- `QGLWater.pro:144`
- `QGLWater.pro:145`

## 本次验证结果

本次迁移完成后，已经执行以下验证：

1. 在 Qt Creator 的 Release build 目录重新运行 `qmake`
2. 在 `vcvars64` 环境下运行 `jom -f Makefile.Release`

验证结果：
- Release 构建通过
- 生成产物：
  - `build/Desktop_Qt_6_8_3_MSVC2022_64bit-Release/release/QGLWater.exe`
- 产物大小：
  - `1430528` 字节
- 构建时间：
  - `2026-03-14 14:53`

## 当前仍存在但未阻塞构建的问题

以下问题目前未继续处理，但不再阻塞 Qt 6.8.3 Release 构建：

- 多个头文件存在 `C4828` 编码警告
- 若干未使用变量 / 未使用参数 warning
- `QMap` 迭代器的旧写法 warning
- 其他历史 warning 尚未系统清理

## 建议的后续工作

建议按下面顺序继续收尾：

1. 清理 `C4828` 编码警告，统一源码编码
2. 处理 `QMap` 旧迭代器 warning
3. 清理未使用变量、未使用参数 warning
4. 做一轮 Debug/Release 双配置验证
5. 补一份更完整的 Qt5 -> Qt6 迁移 checklist
