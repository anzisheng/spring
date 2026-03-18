# QGLWater 2026-03-14 工作汇总与交接说明

## 1. 文档目的

本文档用于汇总 `2026-03-14` 当天在当前工程上完成的迁移、合并、修复和验证工作，供后续维护、回归测试、继续合并特性时直接查阅。

本文档面向两类读者：

- 接手继续开发的工程人员
- 做联调、回归和发布的测试/实施人员

## 2. 项目背景

当前主工程：

- `D:\workspace\FY25\QGLWater-Src-2026-01-16_SL40 - Clock\Src-2026-01-16`

参考合并源工程：

- `D:\workspace\FY25\QGLWater683\QGLWater`

本次工作原则不是整项目覆盖，而是以当前工程为主线，选择性迁入以下能力：

- Qt 6.8.3 兼容与构建修复
- 中文乱码、编码、多语言相关修正
- 频谱/波形 L/R 双通道上下显示
- 编曲场景交互增强
- UI 风格与可读性优化
- `.pro` 输出目录、`cfgtemp`、第三方 DLL 和 `windeployqt` 发布逻辑
- 数据库 `route` 表兼容修复

## 3. 相关参考文档

今天已经产出的相关文档如下：

- `docs/2026-03-14-qt6-8-3-migration-changes.md`
- `docs/plans/2026-03-14-qglwater683-spectrum-ui-design.md`
- `docs/plans/2026-03-14-qglwater683-spectrum-ui-merge.md`
- `docs/plans/2026-03-14-qglwater683-interaction-wave-ui-cfgtemp-merge.md`

如果需要看更细的 Qt 6 API 迁移点或按任务拆分的实施计划，以上文档仍然有效；本文档负责做单点汇总和交接。

## 4. 今日完成项总览

### 4.1 Qt 6.8.3 迁移基线

已完成当前工程到 `Qt 6.8.3 + MSVC2022` 的主要兼容修正，重点包括：

- 项目自有 `*.cpp` / `*.h` 已统一为 `UTF-8 BOM`
- `qcustomplot` 改为引用仓库内 `vendors/qcustomplot`
- 去除旧 `QTextCodec` 依赖，改为 Qt 6 可用路径
- Qt 多媒体切到 `QMediaPlayer + QAudioOutput`
- `QTimeLine` 线性曲线切到 `QEasingCurve::Linear`
- `qSort` / `qStableSort` 迁移到 `std::sort` / `std::stable_sort`
- `QString::SkipEmptyParts` 迁移到 `Qt::SkipEmptyParts`
- 桌面尺寸接口迁移到 `QScreen`
- `ClockDialog` 已补回 qmake 工程
- FMOD 链接路径改为按目标架构选择

该部分详细改动请参考 `docs/2026-03-14-qt6-8-3-migration-changes.md`。

### 4.2 编码、乱码与多语言整理

今天围绕乱码和后续多语言支持，完成了以下工作：

- 项目源码统一到 `UTF-8 BOM`，减少中文注释、字符串在 Qt6/MSVC 下的混乱
- `QTreeMode.cpp` 一级节点文案已使用 `tr()`，包括：
  - 系统设置
  - 输出设备
  - 表演编队
  - 表演轨迹
  - 编曲文件
  - 表演序列
  - 电气设备
- `QSubTreeView.cpp` 右键菜单主项大部分已改为 `tr()`，例如新建轨迹、新建设备、新建编曲、删除、重新关联音乐、手工赋值 id
- `mainwindow.cpp` 中状态栏文案、Tab 页标题、提示信息等已大量切到 `tr()`
- `MusicWidget.cpp` 中场景内音乐标签已改为：
  - `音乐：%1`
  - `时长：%2 秒`
- `StaticValue.cpp` 中数据库错误日志已切到 `tr()`，便于后续多语言统一

额外确认结果：

- 当前仓库中未再扫描到 `QString::fromLocal8Bit` 的使用

当前仍有的多语言遗留：

- `QSubTreeView.cpp` 中 `16路输出设备` / `32路输出设备` / `48路输出设备` / `64路输出设备` 仍是直接字面量，尚未包进 `tr()`
- 仍有少量英文测试文案或旧提示文案未统一整理

结论：

- 本轮已经完成“先止乱码、再为多语言铺路”的第一阶段
- 若后续正式做翻译包，需要继续补一轮“纯字面量 -> tr()”清理

### 4.3 从 QGLWater683 选择性合并的能力

#### A. 频谱/波形双通道能力

已从 `QGLWater683` 合并并保留到当前工程的能力：

- `spectrumwidget` 恢复为上下双 `QCPAxisRect`
- 使用 `m_graphLeft` / `m_graphRight` 分别绘制 L/R
- 颜色区分左右声道
- 频谱背景切为暗色
- `MusicWidget` 中恢复 `qRegisterMetaType<float*>("float*")`
- `signalAddDataToGraph` 使用 `Qt::QueuedConnection`
- `slotAddDataToGraph()` 中恢复双声道拆分逻辑，把 PCM 数据拆成 `yLeft` / `yRight`
- 频谱绘制工作通过 `std::thread` 启动，UI 更新仍通过 queued signal 回到主线程，降低界面卡顿
- `m_spWidget->setLabelWidth(m_HeaderWidth)` 已接回，修正时间线左边界与标签区域对齐问题

这意味着当前工程已经支持：

- 波形 L/R 双通道上下显示
- 频谱 L/R 双通道上下显示
- 绘图计算与 UI 更新分离，减少主线程压力

#### B. 编曲交互增强

已落到代码中的交互增强包括：

- `MusicGraphicsView` 启用 `RubberBandDrag`，支持拖拽框选动作
- 支持鼠标左键单击定位参考线
- 支持 `Delete` 删除选中的红线和动作块
- 删除红线后会立即调用 `savecutlines()`
- `CutSolidVerticalLine` 选中后可以获得焦点
- `CollidingRectItem` 恢复圆角块样式
- `CollidingRectItem` 左右把手提示区域和 grip 指示点已恢复
- `CollidingRectItem::contains()` 扩大了命中范围，便于点击
- 左右拉伸时支持对红色分割线吸附
- 吸附逻辑仍保持当前工程的时间换算方式：
  - 以 `(m_EditView - 30.0)` 为时间宽度基准

#### C. UI 风格与可读性

已合并并保留的 UI 调整包括：

- `main.cpp` 启动时先从程序目录读取 `dark.qss`
- 若 `dark.qss` 缺失，则使用内置 fallback stylesheet
- 左侧 treeview 增加 `leftTreeView` objectName
- 左侧 treeview 使用 `Windows` 风格，并开启：
  - 交替行颜色
  - 统一行高
  - 动画
  - 整行选中
  - 单选
- 全局 dark 主题启用后，主场景默认底色调整为更适合阅读的浅色系
- 时间刻度文字改为白色
- 电气设备页背景改浅，文字改黑色
- `QGraphicsView` 在 dark 主题下仍保持浅底深字，避免主编辑区过暗

其中电气页相关实现已明确落在：

- `ElecDeviceShowWidget.cpp`
  - `QGraphicsScene` 背景为浅灰色
- `ElecDeviceRectItem.cpp`
  - 设备名称文字画笔为黑色

### 4.4 treeview 与数据展示方向

今天处理 tree 相关问题时，已经确认并保留了以下结果：

- `QTreeMode` 的一级节点中文已恢复正常
- `QSubTreeView` 的右键菜单主体中文大多已恢复正常
- `QTreeMode::setupModelData()` 会重新装配输出设备、表演编队、表演轨迹、编曲文件、表演序列、电气设备等一级数据

需要注意：

- 这部分属于“当前代码状态已经恢复”的结果
- 但如果后续继续大规模覆盖 `QTreeMode.cpp` / `QSubTreeView.cpp`，很容易把乱码修复和 tree 数据装配逻辑一起回退

## 5. `.pro`、构建输出与发布逻辑调整

### 5.1 输出目录结构

`QGLWater.pro` 已支持按架构分离输出目录：

```qmake
ARCH_NAME = x86 / x64
BUILD_ROOT  = $$PWD/build/$$ARCH_NAME

DESTDIR     = $$BUILD_ROOT
MOC_DIR     = $$BUILD_ROOT/objs/moc
RCC_DIR     = $$BUILD_ROOT/objs/rcc
UI_DIR      = $$BUILD_ROOT/objs/ui
OBJECTS_DIR = $$BUILD_ROOT/objs/obj
```

已兼容：

- `x64`
- `x86`

并且当前 `ARCH_NAME` 的判断优先使用目标架构信息，而不是简单跟随宿主机架构，避免在 64 位机器上误把 32 位构建输出到 `x64`。

### 5.2 `cfgtemp`、样式与第三方 DLL 拷贝

`QMAKE_POST_LINK` 已补齐以下发布动作：

- 复制 `dark.qss`
- 复制 `cfgtemp` 内容
- 复制 `yaml-cpp.dll`
- 复制 `mbedcrypto.dll`
- 复制 `mbedtls.dll`
- 复制 `mbedx509.dll`
- 复制 FMOD 对应 DLL
- 执行 `windeployqt --compiler-runtime`

需要特别说明一件事：

- 当前 `cfgtemp` 不是被复制到 `build/x64/cfgtemp/`
- 而是把 `cfgtemp` 目录下的内容直接复制到 `build/<arch>/` 根目录

这是当前实现刻意保留的行为，不是遗漏。原因是运行时代码直接按程序目录读取这些文件：

- `config.ini`
- `water.db`
- `pingcfg.ini`
- `pictures/`

如果后续把它改成 `build/<arch>/cfgtemp/`，则必须同步修改运行时路径解析逻辑，否则程序仍会找不到配置和数据库。

### 5.3 `windeployqt`

`.pro` 中已加入：

```qmake
WINDEPLOYQT = $$quote($$system_path($$[QT_INSTALL_BINS]/windeployqt.exe))
WINDEPLOYQT_ARGS = --compiler-runtime
```

生成的 Makefile 已确认会在链接后执行 `windeployqt --compiler-runtime`。

### 5.4 x86 支持状态

今天已完成：

- `.pro` 层面的 x86/x64 分流支持
- 依赖路径按 `ARCH_NAME` 选择
- 生成的 Makefile 中 x86 输出路径与依赖目录已校验

当前未做完整确认的部分：

- 尚未记录一次完整的 x86 GUI 运行回归

因此对 x86 的结论应写为：

- 构建路径和依赖分发逻辑已接通
- 完整运行回归还需要补一轮

## 6. 数据库兼容修复

### 6.1 问题现象

运行时曾大量出现以下日志：

```text
"获取编曲通道失败！error:" "No query Unable to fetch row"
```

### 6.2 根因

根因有两层：

1. 代码查询了 `route.ClockStyle` 和 `route.TimeLeadValue`
2. 现场数据库的 `route` 表缺少这两个字段

同时，原查询方式写成了：

- `QSqlQuery query(qStrSql); if (!query.exec())`

这种写法在当前上下文里没有把错误暴露清楚，导致日志只看到了 `No query Unable to fetch row`，不利于排障。

### 6.3 已做修复

#### A. 启动阶段 schema 兼容

当前在 `main.cpp` 中临时增加了：

- `ensureRouteColumn(QSqlDatabase&, const QString&)`
- `ensureDatabaseSchema(QSqlDatabase&)`

程序打开 SQLite 后，会先补齐：

- `route.ClockStyle INTEGER DEFAULT 0`
- `route.TimeLeadValue INTEGER DEFAULT 0`

然后再继续 `StaticValue::Parse()`。

#### B. 查询执行方式修复

`StaticValue::getRouteInfo()` 已改成：

- 先构造空 `QSqlQuery`
- 再执行 `query.exec(qStrSql)`
- 失败时同时输出失败 SQL 和 `lastError()`

这样后续再遇到数据库字段不兼容时，日志会更可读。

#### C. 数据文件已直接补齐

今天已直接处理的数据库文件包括：

- `cfgtemp/water.db`
- `build/x64/water.db`

上述文件中的 `route` 表已具备：

- `ClockStyle`
- `TimeLeadValue`

### 6.4 当前架构上的待改进点

虽然这个问题已经修住，但当前放置位置还不是最理想：

- 兼容 helper 目前在 `main.cpp`

从架构上更合适的归属应该是：

- `class StaticValue`

也就是说，当前状态可以运行，但后续建议把 schema 检查/迁移能力收回到 `StaticValue`，而不是继续留在 `main.cpp` 的匿名命名空间中。

## 7. 今日新增的验证与辅助脚本

新增脚本：

- `tests/check_route_schema.sh`

用途：

- 校验指定 SQLite 数据库是否已经具备 `route.ClockStyle` 和 `route.TimeLeadValue`

脚本核心检查为：

```bash
sqlite3 "$db_path" "select ClockStyle, TimeLeadValue from route limit 1;"
```

这个脚本已经用于验证：

- `cfgtemp/water.db`
- `build/x64/water.db`

## 8. 今日已完成的验证

### 8.1 构建验证

已完成：

- `qmake` 重新生成工程
- `jom -f Makefile.Release` 完成 Release 构建

结果：

- 在 route schema 修复后，Release 构建成功
- 链接完成后，post-link 拷贝和 `windeployqt --compiler-runtime` 也已执行

### 8.2 `.pro` 输出与分发验证

已确认：

- x64 Makefile 中包含 `build/x64` 输出路径
- x64 Makefile 中包含 `windeployqt --compiler-runtime`
- x86 探测 Makefile 中包含 `build/x86` 输出路径
- x86 探测 Makefile 中包含对应架构依赖路径

### 8.3 数据库兼容验证

已确认：

- `select ClockStyle, TimeLeadValue from route limit 1;` 可在已修复数据库上执行成功
- `tests/check_route_schema.sh` 可通过

## 9. 当前仍需注意的遗留问题

以下问题今天没有继续展开，后续接手时需要注意：

### 9.1 schema helper 位置不理想

当前问题已修复，但 `ensureRouteColumn()` / `ensureDatabaseSchema()` 仍在 `main.cpp`，建议后续迁入 `StaticValue`。

### 9.2 老式信号槽连接警告仍在

运行日志里仍能看到若干旧问题，例如：

- `QObject::connect: No such slot ...`
- `QMetaObject::connectSlotsByName: No matching signal ...`
- 基于字符串的旧 `connect()` 写法不够安全

这些目前不阻塞启动，但属于需要后续清理的技术债。

### 9.3 多语言清理尚未彻底完成

虽然 `fromLocal8Bit` 已清空、主要中文乱码已修正，但还存在：

- 少量直接字面量未走 `tr()`
- 个别旧英文/测试字符串未统一处理

### 9.4 x86 仍需完整回归

`.pro` 和 Makefile 层面已经支持 x86，但还缺一次完整的 x86 构建启动与手工 UI 回归记录。

## 10. 后续建议

推荐后续按以下顺序继续收尾：

1. 把 route schema 兼容逻辑从 `main.cpp` 移到 `StaticValue`
2. 再做一轮 `tr()` 全量扫尾，补齐剩余字面量
3. 处理旧式 `connect()` / `connectSlotsByName` 运行时警告
4. 补一次完整 x86 构建与启动回归
5. 最后再决定是否继续从 `QGLWater683` 合并其它非核心能力

## 11. 结论

截至今天，当前工程已经完成以下关键跃迁：

- Qt 6.8.3 构建链路打通
- 中文乱码与源码编码问题基本收敛
- 683 工程中的双通道频谱、部分编曲交互和 UI 风格已成功选择性迁入
- `.pro` 已具备按架构输出、复制配置资源、复制第三方依赖和 `windeployqt` 部署能力
- `route` 表兼容问题已经修住，运行时不再因为缺少 `ClockStyle` / `TimeLeadValue` 反复报错

当前状态已经适合继续做后续清理和小步迭代，但不建议再做“大文件直接覆盖式”合并，否则今天已经修好的 Qt6、乱码、tree、数据库兼容逻辑都很容易被一起冲掉。
