#!/usr/bin/env bash
set -euo pipefail

script_dir=$(cd "$(dirname "$0")" && pwd)
project_dir=$(cd "$script_dir/.." && pwd)

cd "$project_dir"

vcxproj_path="QGLWater.vcxproj"

require_file() {
    local path="$1"
    if [ ! -f "$path" ]; then
        echo "missing file: $path" >&2
        exit 1
    fi
}

require_file "$vcxproj_path"

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

require_contains "$vcxproj_path" "Release|x64"
require_contains "$vcxproj_path" "Debug|x64"
require_contains "$vcxproj_path" "build\\x64\\"
require_contains "$vcxproj_path" "build\\x64\\objs\\obj\\"
require_contains "$vcxproj_path" "build\\x64\\objs\\moc"
require_contains "$vcxproj_path" "build\\x64\\objs\\ui"
require_contains "$vcxproj_path" "msvc2022_64"
require_contains "$vcxproj_path" "ext\\FMOD\\lib\\x64\\fmod_vc.lib"
require_contains "$vcxproj_path" "ext\\FMOD\\lib\\x64;%(AdditionalLibraryDirectories)"
require_contains "$vcxproj_path" "vendors\\yaml-cpp\\x64\\bin\\yaml-cpp.dll"
require_contains "$vcxproj_path" "vendors\\mbedtls\\x64\\lib\\mbedcrypto.dll"
require_contains "$vcxproj_path" "vendors\\mbedtls\\x64\\lib\\mbedtls.dll"
require_contains "$vcxproj_path" "vendors\\mbedtls\\x64\\lib\\mbedx509.dll"
require_contains "$vcxproj_path" "ext\\FMOD\\lib\\x64\\fmod.dll"
require_contains "$vcxproj_path" "ext\\FMOD\\lib\\x64\\fmodL.dll"

require_not_contains "$vcxproj_path" "ext\\FMOD\\lib\\x86\\fmod_vc.lib"
require_not_contains "$vcxproj_path" "vendors\\yaml-cpp\\x86\\bin\\yaml-cpp.dll"
require_not_contains "$vcxproj_path" "vendors\\mbedtls\\x86\\lib\\mbedcrypto.dll"
require_not_contains "$vcxproj_path" "build\\x86\\QGLWater.exe"
