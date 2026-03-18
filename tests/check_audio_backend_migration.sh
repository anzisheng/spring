#!/usr/bin/env bash
set -euo pipefail

script_dir=$(cd "$(dirname "$0")" && pwd)
project_dir=$(cd "$script_dir/.." && pwd)

cd "$project_dir"

require_file() {
    local path="$1"
    if [ ! -f "$path" ]; then
        echo "missing file: $path" >&2
        exit 1
    fi
}

require_contains() {
    local path="$1"
    local text="$2"
    if ! rg -F -q -- "$text" "$path"; then
        echo "missing text in $path: $text" >&2
        exit 1
    fi
}

require_not_contains() {
    local path="$1"
    local text="$2"
    if rg -F -q -- "$text" "$path"; then
        echo "unexpected text in $path: $text" >&2
        exit 1
    fi
}

require_file "Refactor/Audio/AudioPlayer.h"
require_file "Refactor/Audio/AudioPlayer.cpp"
require_file "Refactor/Audio/FmodAudioPlayer.h"
require_file "Refactor/Audio/FmodAudioPlayer.cpp"

require_contains "MusicWidget.h" "#include \"Refactor/Audio/AudioPlayer.h\""
require_contains "MusicWidget.h" "AudioPlayer *m_audioPlayer;"
require_contains "MusicWidget.cpp" "m_audioPlayer = new AudioPlayer(this);"
require_contains "MusicWidget.cpp" "m_audioPlayer->setSystem(pSystem);"
require_contains "QGLWater.pro" "Refactor/Audio/AudioPlayer.cpp"
require_contains "QGLWater.pro" "Refactor/Audio/FmodAudioPlayer.cpp"

require_not_contains "MusicWidget.h" "#include <QMediaPlayer>"
require_not_contains "MusicWidget.h" "#include <QAudioOutput>"
require_not_contains "MusicWidget.h" "QMediaPlayer *mediaPlayer;"
require_not_contains "MusicWidget.h" "QAudioOutput *audioOutput;"
require_not_contains "MusicWidget.cpp" "mediaPlayer->"
require_not_contains "MusicWidget.cpp" "audioOutput ="
require_not_contains "QGLWater.pro" "QT += multimedia"
