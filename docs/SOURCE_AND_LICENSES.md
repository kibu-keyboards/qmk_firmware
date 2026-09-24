# 来源与许可

## 发布范围与基准

本副本来自 `qmk_firmware_pre_fix01_exact_source_20260807.zip`，SHA-256 为 `B5432FB8B39ADFD91D3196C08056C09BC3D90542B151631B6AFB9C87A8119846`，随 2026-08-12 PRE-FIX01 包交付。产品负责人已确认其为量产所用源码；该来源确认与本轮构建、实机或许可核验分别记录。

原 ZIP 含 22,260 个文件；排除 9 个失效的 `.git` 指针后，副本保留 22,251 个原文件。原指针另存于整理证据中；这不构造新的生产 Git 历史。源码的程序文本变更仅为 P75 编码器片段补充 QMK 原作者／来源注释，不改变行为。原 ZIP 保持不变；副本不再声称与原 ZIP 每个字节相同。未提供可确证的生产 commit、tag 或完整依赖 revision，不能从失效指针或较新开发树推断。

本包的 [QMK/FS026 源码](../qmk_firmware/)包括产品代码、平台支持、USB 输入、SPI 协议、BLE／2.4G 模式及报告桥接。独立 91680 无线控制器内部固件未包含、内部状态为 NOT AUDITED；不能把 FS026 侧源码公开解释为整机全部开源，也不能据此断言 QMK 的名单决定。

## 原作者与许可通知

| 组件 | 已有声明与保留方式 |
| --- | --- |
| QMK | 保留 [GPL v2 许可文本](../qmk_firmware/LICENSE)及各文件版权／许可声明；不同组件以其原文件声明为准。 |
| P75 产品代码 | 保留 Finalkey、LiWenLiu、Yiancar 和其他现有作者通知。产品增量与 JSON 的权利范围不能仅凭片段来源推定。 |
| P75 编码器片段 | 对应同包 [QMK encoder_quadrature.c](../qmk_firmware/drivers/encoder/encoder_quadrature.c) 的四段代码；补充 Jack Humbert（2018）和 Nick Brassel（2018–2023）及 GPL-2.0-or-later 来源说明，保留原产品作者。 |
| ChibiOS | 保留 [原许可说明](../qmk_firmware/lib/chibios/license.txt)、文件头和随附通知。 |
| ChibiOS-Contrib／FS026 | 保留 [Contrib 原说明](../qmk_firmware/lib/chibios-contrib/README.md)及各文件声明。已核对的 120 个 FS026 路径中，103 个有 Apache 文本／声明，2 个为空占位文件，15 个非空文件的许可覆盖未查明。 |
| rdr_common | [C 实现](../qmk_firmware/lib/rdr_lib/rdr_common.c)已有 `GPL-2.0-or-later` SPDX；0812 [头文件](../qmk_firmware/lib/rdr_lib/rdr_common.h)没有后续版本的 SPDX，不猜测其作者或补写无依据的许可。 |

这些说明不替代各组件原许可文本，也不将整包统一改署 KIBU。所有原有许可和版权通知继续保留。

## rdr 恢复／重实现说明

`lib/rdr_lib/rdr_common.c` 是原 `librdrcommon.a` 的恢复／重实现 C 版本，通过源码直接编译。范围包括 QMK 报告转发、无线控制器 SPI 交互、模式和配对命令、电池、电源、USB 挂起／唤醒及持久化等。它不是重新取得的厂家原始 C 文件，也不能以源码存在推定原 archive／object 的完整权利链。

随包 [恢复说明](../qmk_firmware/REIMPLEMENTATION_STATUS.md)、[实现说明](../qmk_firmware/lib/rdr_lib/README.md)和[静态验证记录](../qmk_firmware/VERIFICATION.txt)保留历史形成过程。历史记录记载 164／164 外部符号及源码构建结果，只能按原记录的日期和范围引用；本轮结果见[构建说明](BUILD.md)。

## 仍未核实的具体资料

原 archive／object 输入的公开权利依据、产品增量及 JSON 的许可范围、FS026 下列 15 个非空文件的许可覆盖，以及 `hal_pal_lld.h` 仅年份版权行中的作者信息，仍未完整核实。资料不足与已查明的实际违规分别处理；本说明不将未知项标为 VERIFIED，也不猜写作者或许可证。

未找到禁止条款不能代替许可依据；这些记录既不是违规认定，也不是完整可分发证明。必要的来源说明、署名修正和实际构建证据分别提供，不把未知项笼统升级为授权已核实。

下列路径相对 `qmk_firmware/lib/chibios-contrib/`：

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
