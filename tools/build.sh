#!/usr/bin/env bash
set -euo pipefail
package="$(cd "$(dirname "$0")/.." && pwd)"
chmod +x "$package/tools/bin/qmk" "$package/tools/bin/python3"
export QMK_PYTHON="${QMK_PYTHON:-$(command -v python3)}"
export QMK_BUILD_DATE="${QMK_BUILD_DATE:-2026-08-07-00:00:00}"
export PYTHONIOENCODING=utf-8
export PYTHONDONTWRITEBYTECODE=1
export PATH="$package/tools/bin:${ARM_GCC_BIN:+$ARM_GCC_BIN:}$PATH"
cd "$package/qmk_firmware"
build_dir="${P75_BUILD_DIR:-.build_0812_release}"
if [[ -e "$build_dir" ]]; then
    printf 'Build directory already exists: %s. Choose a new P75_BUILD_DIR.\n' "$build_dir" >&2
    exit 1
fi
arm-none-eabi-gcc --version
"$QMK_PYTHON" --version
make --version
printf 'Generated QMK build-date metadata: %s\n' "$QMK_BUILD_DATE"
qmk hello
make QMK_BIN=qmk SKIP_GIT=yes p75_jis/p75_jis:via BUILD_DIR="$build_dir" -j2 VERBOSE=true
test -s "$build_dir/p75_jis_p75_jis_via.bin"
sha256sum "$build_dir/p75_jis_p75_jis_via.bin"
