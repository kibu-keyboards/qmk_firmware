# KIBU P75 JIS — 0812 对应源码发布（p75-jis-0812-r1）

本资料用于公开 2026-08-12 PRE-FIX01 交付包所对应的 P75 JIS QMK／FS026 侧源码、构建依赖、版权／许可通知及配套 VIA 定义。产品负责人已确认以此源码作为量产对应发布依据；固定版本为 `p75-jis-0812-r1`；实际匿名下载构建及 CI 结果在 Release 验证回执中单独记录，不以此摘要代替验收证据。

## 用户入口

- 完整源码、构建脚本和说明包含在源码 ZIP 中；解压后从根目录 README.md 开始。
- VIA 配置请通过有线 USB 连接，使用随包外部 `kibu_p75_jis_via_pre_fix01.json`，按 `docs/VIA_GUIDE.md` 手动加载。蓝牙／2.4G 是输入方式，配置仅支持 USB。
- 此次源码公开不要求消费者升级固件。包内 BIN 保留作历史版本对应证据，不作为推荐的新升级固件；文件名中的 `hardware_validated` 不代表本轮三模全通过。
- 固定源码：https://github.com/kibu-keyboards/qmk_firmware/tree/p75-jis-0812-r1 。下载：https://github.com/kibu-keyboards/qmk_firmware/releases/tag/p75-jis-0812-r1 。完整源码包为 `KIBU-P75-JIS-0812-r1-source.zip`。
- 售后：https://kibushop.com/pages/contact ，邮箱 m@kibushop.com。

## 本版内容

源码含 USB 输入及 BLE／2.4G 的 FS026 侧桥接实现；ChibiOS、ChibiOS-Contrib 和 FS026 源码按固定文件清单随包提供，不要求访问私有依赖仓库。独立 91680 无线控制器内部固件未包含、未审计，不宣称整机全部开源。保留原组件署名和许可通知，具体来源说明及未知项见包内 `docs/SOURCE_AND_LICENSES.md`。

2026-09-23 已使用 GCC 10.3.1 完成两次干净构建；补充编码器署名并固定历史 VIA 日期元数据后，80,860 字节 BIN 与 0812 归档原件逐字节一致。2026-09-24 候选仅更新 README、测试说明、此摘要及文件清单；全部源码、BIN、JSON、构建工具和依赖与已接受候选一致。上述构建为9月23日记录；本次发布的匿名获取重建结果另见 Release 验证回执。

| 资产 | SHA-256 |
| --- | --- |
| 对应历史 BIN（80,860 字节） | `21DE5AAFA6684B72AD797C47C61712F8D40F5762477DCBA483DC7BB53A4332B2` |
| 配套 VIA JSON | `2CB4B1F7E9222060C3F55F353128A7278AD6C953ED0B327E95EB1D8089B025BF` |
| 2026-09-23 固定候选 ZIP（保留原件） | `0362C3BE9EE3FB8ECB40E9F0A6A4199D866D6D2687C53DD955C17A65A4B12C13` |

当前发布 ZIP 的完整哈希见与下载文件一起交付的 `SHA256SUMS.txt`。

## 发布整理补充

在9月24日文档更新候选基础上，本发布补充固定下载入口、Git导入说明、Windows构建CI和自动下载／构建工具。固件源码、原BIN、VIA JSON及原依赖字节不变；没有把后续维护代码混入0812快照。文件身份以包内 `metadata/release-manifest.csv` 和随Release提供的校验文件为准。

## 实机验证范围

2026-09-23～24，用户按指定 BIN／JSON 在现有样机操作并确认：USB 基本输入及旋钮、VIA 改键和断电保存、宏与灯光及断电保存、蓝牙 1／2／3、USB／蓝牙切换及蓝牙休眠唤醒通过。产品负责人已接受此交付。

**2.4G 因接收器丢失，本轮未测**，包括其输入、切换、重连和唤醒，不计通过。蓝牙是否需要重新配对未单独回报；没有助手自动回读程序 Flash，不据此证明全部量产批次身份。此前宏不生效的观察已澄清为在另一把键盘上测试，未认定本版本宏缺陷。

基础操作通过不覆盖压力、电池或故障注入测试。原有 SPI 长度／等待、USB 挂起、队列及状态解析风险继续见包内 `docs/KNOWN_ISSUES.md`。本版未并入后续维护修复或 BLE 命名迁移；VIA 手动兼容不代表官方收录或自家网页高级功能验收。
