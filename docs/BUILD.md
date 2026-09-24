# 0812 构建与依赖

目标为 `p75_jis/p75_jis:via`。全部 QMK、FS026、ChibiOS、ChibiOS-Contrib 与产品桥接源码随本包提供；使用这些固定内容，不执行 `git submodule update` 来替换成其他版本。本包不依赖 Web 项目或私有产品 object/archive。

## 已固定的输入

- 原源码 ZIP：`B5432FB8B39ADFD91D3196C08056C09BC3D90542B151631B6AFB9C87A8119846`。
- GCC：GNU Arm Embedded **10.3-2021.10 / 10.3.1 20210824**。Windows 原工具链 ZIP 为 `gcc-arm-none-eabi-10.3-2021.10-win32.zip`，200,578,763 字节，SHA-256 `D287439B3090843F3F4E29C7C41F81D958A5323AECEFCF705C203BFD8AE3F2E7`。
- [Arm 官方下载入口](https://developer.arm.com/additional-resources/downloads-legacy-gnu-toolchain-for-embedded-processors)；[该版本 Windows ZIP](https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-win32.zip)。2026-09-23 对 ZIP 的匿名 HTTP HEAD 返回 200；本机实际使用的编译器、链接器、objcopy、nm 及四项运行库与此归档对应成员逐字节一致，见 [toolchain.json](../metadata/toolchain.json)。没有把 HEAD 检查写成另一次完整网络下载校验。
- 本轮宿主：Windows、QMK MSYS Bash、GNU Make 4.4.1、Python 3.12.10；使用本包 `lib/python` 中的 QMK CLI。Python 的实际包版本记录于 [python-packages-observed.txt](../metadata/python-packages-observed.txt)。这是本轮已用版本清单，不是假定的原生产 Python 环境，也不是带下载哈希的依赖锁文件。
- [原源码逐文件清单](../metadata/original-source-files.csv)与[依赖内容身份](../metadata/dependencies.json)固定随包的依赖字节。依赖没有完整可确证的 Git revision；不从新开发树补造 gitlink。

## 构建方法

以下命令在 Bash 中执行。Windows 使用 QMK MSYS；先安装 Python 3.12、上述 GCC 与 GNU Make，并创建独立 Python 环境：

```bash
python3 -m venv .venv
# Windows：.venv/Scripts/python.exe；Linux：.venv/bin/python
.venv/Scripts/python.exe -m pip install -r metadata/python-packages-observed.txt
export QMK_PYTHON="$PWD/.venv/Scripts/python.exe"
export ARM_GCC_BIN="/path/to/gcc-arm-none-eabi-10.3-2021.10/bin"
bash tools/build.sh
```

把 `ARM_GCC_BIN` 换成自己的 GCC 目录；没有绑定本机绝对路径。Linux 应使用对应 Linux 工具链和 `.venv/bin/python`，但本轮只验证了 Windows 工具链，不能直接声明跨平台逐字节一致。

脚本实际运行：

```bash
make QMK_BIN=qmk SKIP_GIT=yes p75_jis/p75_jis:via BUILD_DIR=.build_0812_release -j2 VERBOSE=true
```

输出为 `qmk_firmware/.build_0812_release/p75_jis_p75_jis_via.bin`、ELF、MAP 和对象清单。构建目录已存在时脚本停止；重复构建可设置新的 `P75_BUILD_DIR=.build_check2`，不清理用户原有文件。

`tools/bin` 只处理旧 QMK CLI 在 Windows 下的路径分隔符及文本换行。`tools/run-qmk.py` 使用本包 CLI，不需要安装新版 QMK CLI，也不修改固件实现。

## 为什么固定构建日期

本快照的 `quantum/via.c` 从 `QMK_BUILDDATE` 提取年月日，编入 VIA EEPROM 的有效性标记。直接在不同日期重建会改变固件字节，并可能在刷入后使旧 VIA 键位／宏被初始化；编译日期不是纯展示字符串。

构建脚本默认将**生成头文件的元数据**固定为 `2026-08-07-00:00:00`，由启动器替换旧 CLI 的日期生成函数。2026-08-07 来自历史归档日期及 BIN 中的 VIA 日期常量；`00:00:00` 是复现选择，不宣称知道原编译时分秒。实际本轮执行日期仍是 2026-09-23。该设置没有修改 QMK 程序源码或主机时间。

如果有意构建其他日期，可设置 `QMK_BUILD_DATE=YYYY-MM-DD-HH:MM:SS`；这时不要承诺与原 BIN 一致或保存现有 VIA 设置。源码公开本身不要求消费者刷入重建固件。

## 验证证据

首轮未改程序源码、使用当日构建日期的干净构建成功：80,860 字节，SHA-256 `898C292DD72B86AE6CC08C2700392D72A66929F278B45ED364731E0D8E4E913D`。与原 BIN 仅 7 字节不同，均定位到 `via_eeprom_is_valid` / `via_eeprom_set_valid` 的日期立即数或相关算术常量。

第二轮从空构建目录重新编译，补署名并固定历史日期后，产物与 0812 原 BIN **逐字节一致**：80,860 字节，SHA-256 `21DE5AAFA6684B72AD797C47C61712F8D40F5762477DCBA483DC7BB53A4332B2`。这同时验证了署名注释未改变固件，且首轮 7 字节差异可由日期元数据解释。结果见 [build-results.json](../metadata/build-results.json)；实机覆盖单独见 [VERSION_AND_TESTS.md](VERSION_AND_TESTS.md)。

实际链接的 183 个对象来自本次源码编译；其中包括 `lib/rdr_lib/rdr_common.c`，没有链接 `librdrcommon.a`。外部 archive 仅为该工具链的 `libgcc.a`、`libc_nano.a`、`libg_nano.a`、`libm.a`。输入列表与哈希见 [链接记录](../metadata/link-inputs-original.json)和[编译输入记录](../metadata/compiled-inputs-original.json)。本轮 ELF 的未定义符号检查为空。上述证据只证明这个 FS026 构建输入，不代表已审计独立 91680 内部固件。

旧编译器／源文件的编译警告保留在构建记录，没有通过修改代码消除。编译、文件一致性和公开下载均不能代替设备功能验收。

## p75-jis-0812-r1 自动下载与构建

Windows 可在 PowerShell 7 中运行 `tools/ci-build.ps1`，指定 Windows Python 3.12 的 `PythonPath`、现有 MSYS2/QMK MSYS 的 `BashPath`，以及包外全新且不含空格的 `WorkDirectory`。脚本完整下载并核验上述官方 GCC ZIP，创建独立 Python 环境、从公开 PyPI 安装记录的依赖，复制构建输入到工作目录后调用原 `tools/build.sh`，最后断言 BIN 的完整 SHA-256 等于上文原件。原源码目录保持原样；脚本拒绝复用已存在的工作目录。

```powershell
# 把三个路径替换为自己的 Python、MSYS Bash 和新建工作目录。
./tools/ci-build.ps1 -PythonPath C:/Python312/python.exe -BashPath C:/msys64/usr/bin/bash.exe -WorkDirectory C:/p75-build-1
```

GitHub workflow 使用 Windows runner、Python 3.12 与 MSYS2；此文件只说明运行方法，具体运行是否通过以对应提交的 Actions 记录为准。CI 检查不能代替2.4G等未做的设备测试。

Git下载时请遵循 [精确源码获取](GIT_IMPORT.md)，避免源码自带属性规则改变本地换行。ZIP直接解压使用固定成员字节，不需要初始化子模块。
