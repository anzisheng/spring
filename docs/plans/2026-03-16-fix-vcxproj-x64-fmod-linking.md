# Fix Vcxproj X64 Fmod Linking Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 修复 `QGLWater.vcxproj` 的 `x64` 配置错误，避免 VS2022 在 `x64` 构建时继续链接到 `x86` 的 FMOD 和第三方库。

**Architecture:** 本次不改业务代码，保留已正确的 `QGLWater.pro`，通过 `qmake -tp vc` 重新生成 `QGLWater.vcxproj`，并新增一个静态校验脚本守住 `x64` 输出目录和 FMOD 库路径。这样既能解决当前 `unresolved external symbol`，也能避免后续配置再次回退。

**Tech Stack:** Qt 6.8.3, qmake, Qt VS Tools, MSVC2022, bash 校验脚本

---

### Task 1: 固化失败场景

**Files:**
- Create: `tests/check_vcxproj_x64_fmod_linking.sh`
- Test: `QGLWater.vcxproj`

**Step 1: Write the failing test**

新增一个脚本，检查 `Debug|x64` 和 `Release|x64` 配置是否满足下面条件：

- 输出目录为 `build\\x64\\`
- FMOD 链接库使用 `ext\\FMOD\\lib\\x64\\fmod_vc.lib`
- FMOD 库目录使用 `ext\\FMOD\\lib\\x64`
- Debug x64 拷贝 `fmodL.dll`
- Release x64 拷贝 `fmod.dll`
- 工程文件中不再出现 `ext\\FMOD\\lib\\x86\\fmod_vc.lib`

**Step 2: Run test to verify it fails**

Run: `bash tests/check_vcxproj_x64_fmod_linking.sh`

Expected: FAIL，指出当前 `x64` 配置仍落在 `x86`

### Task 2: 修正 VS 工程文件

**Files:**
- Modify: `QGLWater.vcxproj`

**Step 3: Write minimal implementation**

不手工维护大型 XML，直接执行：

- `C:\\Qt\\6.8.3\\msvc2022_64\\bin\\qmake.exe -tp vc QGLWater.pro`

让 `QGLWater.pro` 中现有的 `ARCH_NAME` / `FMOD_LIB_DIR` 配置重新落到 `QGLWater.vcxproj`，并生成新的 `.filters`

**Step 4: Run test to verify it passes**

Run: `bash tests/check_vcxproj_x64_fmod_linking.sh`

Expected: PASS，无输出

### Task 3: 补充验证

**Files:**
- Modify: `docs/2026-03-14-work-summary.md`（如需要）

**Step 5: Verify**

补充执行：

- `bash tests/check_audio_backend_migration.sh`

Expected: PASS，确保这次没有把已有 FMOD 音频迁移结果改坏
