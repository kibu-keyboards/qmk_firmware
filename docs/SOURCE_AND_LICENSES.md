# Source Provenance and License Notices

> Modified by KIBU (kibu-keyboards) (M@kibu.jp) on 2026-09-24. Changes: Translated and clarified the English provenance and component-license documentation. Original authorship and license notices are retained.

## Baseline and scope

This package derives from `qmk_firmware_pre_fix01_exact_source_20260807.zip`, SHA-256 `B5432FB8B39ADFD91D3196C08056C09BC3D90542B151631B6AFB9C87A8119846`, supplied with the August 12, 2026 PRE-FIX01 package.

The publication copy includes attribution and documentation/comment updates and excludes unusable Git pointer files. Executable code is unchanged; the publication ZIP is not byte-identical to the original archive. A production Git commit and complete dependency Git revisions have not been established. Dependencies are provided as fixed files.

The [QMK/FS026 source](../qmk_firmware/) includes product code, platform support, USB input, the SPI protocol, and Bluetooth/2.4 GHz mode and report bridging. Internal firmware for the separate 91680 wireless controller is not included and has not been audited. The source release covers the FS026 firmware, not every firmware component in the keyboard.

## Retained authorship and notices

| Component | Notices and treatment |
| --- | --- |
| QMK | Retain the [GPL v2 text](../qmk_firmware/LICENSE) and each file's copyright/license notices. Individual component declarations remain authoritative. |
| P75 product code | Retain existing Finalkey, LiWenLiu, Yiancar, and other author notices. Rights covering the product-specific changes and JSON cannot be inferred solely from the origin of individual excerpts. |
| P75 encoder excerpts | Four excerpts correspond to the bundled [encoder_quadrature.c](../qmk_firmware/drivers/encoder/encoder_quadrature.c). Attribution to Jack Humbert (2018), Nick Brassel (2018–2023), and the GPL-2.0-or-later source was added while retaining the product authors. |
| ChibiOS | Retain the [original license information](../qmk_firmware/lib/chibios/license.txt), file headers, and accompanying notices. |
| ChibiOS-Contrib / FS026 | Retain the [Contrib documentation](../qmk_firmware/lib/chibios-contrib/README.md) and file-level notices. Of 120 examined FS026 paths, 103 contain Apache text/notices, two are empty placeholders, and license coverage for 15 nonempty files remains unresolved. |
| rdr_common | The [C implementation](../qmk_firmware/lib/rdr_lib/rdr_common.c) carries `GPL-2.0-or-later` SPDX. The 0812 [header](../qmk_firmware/lib/rdr_lib/rdr_common.h) has no SPDX license declaration. |

This document does not replace the original license texts or reattribute the package to KIBU. All existing copyright and license notices are retained.

## rdr recovery and reimplementation

`lib/rdr_lib/rdr_common.c` is a recovered/reimplemented C replacement for `librdrcommon.a`, compiled directly from source. It covers QMK report forwarding, wireless-controller SPI communication, mode/pairing commands, battery and power handling, USB suspend/wake, and persistence. It is not a newly obtained copy of the manufacturer's original C source. Its availability does not establish the complete rights chain for the original archive/object inputs.

See the bundled [reimplementation record](../qmk_firmware/REIMPLEMENTATION_STATUS.md), [implementation notes](../qmk_firmware/lib/rdr_lib/README.md), and [static verification record](../qmk_firmware/VERIFICATION.txt) for technical background. Build instructions are in [BUILD.md](BUILD.md).

## Unresolved provenance and license details

The rights basis for the original archive/object inputs, license scope of product-specific changes and JSON, and license coverage for the 15 nonempty FS026 files below have not been fully established. The year-only copyright line in `hal_pal_lld.h` also omits an author. Existing license declarations are retained; this document does not supply missing permissions or authorship.

Paths below are relative to `qmk_firmware/lib/chibios-contrib/`:

```text
.github/workflows/build.yml
demos/ES32/FS026/Makefile
demos/ES32/FS026/readme.txt
os/common/startup/ARMCMx/compilers/GCC/mk/startup_FS026.mk
os/hal/boards/FS026/board.mk
os/hal/ports/ES32/FS026/platform.mk
os/hal/ports/ES32/LLD/ADCv1/driver.mk
os/hal/ports/ES32/LLD/ADCv1/notes.txt
os/hal/ports/ES32/LLD/GPIOv1/driver.mk
os/hal/ports/ES32/LLD/I2Cv1/driver.mk
os/hal/ports/ES32/LLD/SPIv1/driver.mk
os/hal/ports/ES32/LLD/TIMv1/driver.mk
os/hal/ports/ES32/LLD/UARTv1/driver.mk
os/hal/ports/ES32/LLD/USBv1/driver.mk
os/hal/ports/ES32/LLD/WDTv1/driver.mk
```
