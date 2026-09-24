# KIBU P75 JIS — 0812 源码发布资料

本资料以 2026-08-12 交付的 PRE-FIX01 包为基准，提供 P75 JIS 的 QMK/FS026 侧源码、配套 VIA 定义与版本说明。量产源码来源采用产品负责人的既有确认；发布版本为 `p75-jis-0812-r1`；实际公开验证结果以 GitHub Release 中的验证回执为准。

源码包含 USB 输入和 BLE／2.4G 的 FS026 侧桥接实现。独立 91680 无线控制器内部固件不在本包，未完成其内部审计；这里不宣称整机全部开源，也不代表 QMK 或 VIA 官方收录、认可或许可审查结论。

## 2026-09-24 测试补充

产品负责人已接受本版本的源码、构建资料及实机记录。USB 基本输入／旋钮、VIA 改键与断电保存、宏／灯光及断电保存、蓝牙三通道、USB／蓝牙切换及蓝牙休眠唤醒均获用户确认通过。**2.4G 因接收器丢失，本轮未测，不计通过。**测试按指定固件与用户操作回报关联，未独立回读设备程序 Flash，也不证明全部量产批次身份。

本包在 2026-09-23 固定候选基础上更新发布文档和文件清单，并新增 Git 获取说明、Windows 构建 CI 和自动下载／构建工具；固件源码、BIN、VIA JSON、原构建脚本及依赖字节均未变化。详见[发布摘要](docs/RELEASE_NOTES.md)和[完整验证范围](docs/VERSION_AND_TESTS.md)。

## 固定下载入口

- [完整源码与配套文件](https://github.com/kibu-keyboards/qmk_firmware/releases/tag/p75-jis-0812-r1)：下载 `KIBU-P75-JIS-0812-r1-source.zip`、配套 VIA JSON 和 `SHA256SUMS.txt`。
- [固定标签源码](https://github.com/kibu-keyboards/qmk_firmware/tree/p75-jis-0812-r1)：本仓库分支采用独立源码快照导入，见 [Git 来源与获取说明](docs/GIT_IMPORT.md)。
- [公开构建记录](https://github.com/kibu-keyboards/qmk_firmware/actions/workflows/p75-0812.yml)：请查看本标签／提交实际运行结果；源码提供本身不代表某次 CI 已成功。

## 文件入口

- [完整源码树](qmk_firmware/)：固定 0812 快照及必要的编码器来源注释，保留原产品名称和第三方通知。
- [构建说明](docs/BUILD.md)：构建输入、工具链、操作方法与本轮结果。
- [来源与许可](docs/SOURCE_AND_LICENSES.md)：组件范围、原作者及仍未核实的具体资料。
- [版本与测试记录](docs/VERSION_AND_TESTS.md)：文件 SHA-256、设备身份与历史测试覆盖。
- [已知问题](docs/KNOWN_ISSUES.md)：该基准保留的可靠性风险。
- [VIA 使用说明](docs/VIA_GUIDE.md)：仅通过有线 USB 配置，手动加载厂家 JSON。
- [0812 配套 BIN](artifacts/kibu_p75_jis_pre_fix01_hardware_validated.bin)与[配套 VIA JSON](artifacts/kibu_p75_jis_via_pre_fix01.json)：保留原交付文件及名称；BIN 文件名中的 `hardware_validated` 不代表逐项三模验收。

本次整理服务于对应源码公开，不要求消费者为此升级固件。0812 基准不含 FIX1、后续 Raw HID／旋钮修复、BLE 名称迁移或新的产品命名修改；已知风险不会因公开源码而消失。

项目入口：[KIBU QMK fork](https://github.com/kibu-keyboards/qmk_firmware)。售后请使用[官网联系页面](https://kibushop.com/pages/contact)或 [m@kibushop.com](mailto:m@kibushop.com)，说明 P75 JIS、连接模式、操作步骤及所用 JSON／固件版本。
