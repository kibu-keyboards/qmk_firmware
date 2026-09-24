# Building the 0812 Baseline

> Modified by KIBU (kibu-keyboards) (M@kibu.jp) on 2026-09-24. Changes: Translated and clarified the English build instructions and reference-output documentation. Original authorship and license notices are retained.

The build target is `p75_jis/p75_jis:via`. QMK, FS026, ChibiOS, ChibiOS-Contrib, and the product bridge implementation are included in this package. Build these fixed files; do not run `git submodule update` to substitute a different dependency revision. The build does not depend on the Web project or a private product object/archive.

## Pinned inputs

- Original source ZIP SHA-256: `B5432FB8B39ADFD91D3196C08056C09BC3D90542B151631B6AFB9C87A8119846`.
- Compiler: **GNU Arm Embedded 10.3-2021.10 / GCC 10.3.1 20210824**.
- Windows toolchain: `gcc-arm-none-eabi-10.3-2021.10-win32.zip`, 200,578,763 bytes; SHA-256 `D287439B3090843F3F4E29C7C41F81D958A5323AECEFCF705C203BFD8AE3F2E7`.
- [Arm download index](https://developer.arm.com/additional-resources/downloads-legacy-gnu-toolchain-for-embedded-processors) and [exact Windows archive](https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-win32.zip).
- Build environment: Windows, QMK MSYS Bash, GNU Make 4.4.1, and Python 3.12.10, using the QMK CLI in this package's `lib/python`. The [Python package list](../metadata/python-packages-observed.txt) records the reproduction environment; it is not a hash-locked dependency file.
- [Original source manifest](../metadata/original-source-files.csv), [dependency identities](../metadata/dependencies.json), and [toolchain checks](../metadata/toolchain.json) record the original inputs. Complete dependency Git revisions could not be established; none have been inferred from a later development tree.

Use [the release manifest](../metadata/release-manifest.csv) to verify current package files. Original-input metadata records the supplied baseline.

## Automated Windows build

Run the following from the package root in PowerShell 7. Substitute your Windows Python 3.12 interpreter, existing MSYS2/QMK MSYS Bash executable, and a new work directory outside the package, with no spaces in its path:

```powershell
./tools/ci-build.ps1 -PythonPath C:/Python312/python.exe -BashPath C:/msys64/usr/bin/bash.exe -WorkDirectory C:/p75-build-1
```

The script downloads and verifies the exact Arm archive, creates a separate Python environment, installs the recorded dependencies from public PyPI, copies the build inputs into the new work directory, invokes `tools/build.sh`, and compares the complete BIN SHA-256 with the reference below. It rejects an existing work directory and leaves the source package unchanged. The GitHub workflow uses a Windows runner, Python 3.12, and MSYS2. Consult the release page for the result of the run associated with a particular commit.

## Manual build

The commands below use Bash on Windows with QMK MSYS. Install Python 3.12, the specified Arm toolchain, and GNU Make, then run from the package root:

```bash
python3 -m venv .venv
.venv/Scripts/python.exe -m pip install -r metadata/python-packages-observed.txt
export QMK_PYTHON="$PWD/.venv/Scripts/python.exe"
export ARM_GCC_BIN="/path/to/gcc-arm-none-eabi-10.3-2021.10/bin"
bash tools/build.sh
```

Replace `ARM_GCC_BIN` with your own toolchain path. On Linux the Python executable would be `.venv/bin/python` and an appropriate Linux toolchain would be needed; byte-for-byte reproduction has been verified on Windows, not on Linux.

The script invokes:

```bash
make QMK_BIN=qmk SKIP_GIT=yes p75_jis/p75_jis:via BUILD_DIR=.build_0812_release -j2 VERBOSE=true
```

The BIN is written to `qmk_firmware/.build_0812_release/p75_jis_p75_jis_via.bin`, alongside the ELF, map, and object records. If the output directory already exists, the script stops. Set a new `P75_BUILD_DIR`, such as `.build_check2`, for another build instead of deleting existing results.

The wrappers in `tools/bin` adapt Windows path separators and text line endings for the historical QMK CLI. `tools/run-qmk.py` uses the bundled CLI; installing a current QMK CLI is not required.

## Why the build date is fixed

In this snapshot, `quantum/via.c` derives a date from `QMK_BUILDDATE` and embeds it in the VIA EEPROM validity marker. A different build date changes the firmware and can cause existing VIA keymaps or macros to be reinitialized after flashing. It is not merely a display string.

The launcher defaults generated-header metadata to `2026-08-07-00:00:00` by replacing the historical CLI's date-generation function. The date is supported by the archive and the BIN's VIA constants. Midnight is a reproducibility choice; the original time of day is unknown. The launcher does not change the system clock or the firmware implementation.

Set `QMK_BUILD_DATE=YYYY-MM-DD-HH:MM:SS` only if a different date is intentional. Such a build must not be represented as identical to the reference BIN or as guaranteed to preserve existing VIA settings. Source availability does not require customers to flash a rebuilt image.

## Reference output

The fixed-date Windows build reproduces the 80,860-byte reference BIN byte for byte:

```text
21DE5AAFA6684B72AD797C47C61712F8D40F5762477DCBA483DC7BB53A4332B2
```

[Build records](../metadata/build-results.json) describe the original reproduction. Results for the current release commit are available through the [build workflow](https://github.com/kibu-keyboards/qmk_firmware/actions/workflows/p75-0812.yml).

The 183 linked objects were compiled from the supplied source, including `lib/rdr_lib/rdr_common.c`; `librdrcommon.a` was not linked. External archives were limited to the toolchain's `libgcc.a`, `libc_nano.a`, `libg_nano.a`, and `libm.a`. See [link inputs](../metadata/link-inputs-original.json), [compilation inputs](../metadata/compiled-inputs-original.json), and [undefined-symbol results](../metadata/undefined-symbols.json). The ELF had no undefined symbols. This evidence covers the FS026 build, not the internal firmware of the separate 91680 controller.

Existing compiler and Python warnings remain visible in the build logs. For Git downloads, follow [exact checkout instructions](GIT_IMPORT.md); release ZIPs already contain the recorded bytes and need no submodule initialization.
