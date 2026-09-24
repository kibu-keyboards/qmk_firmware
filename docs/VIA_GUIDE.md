# P75 JIS：VIA 配置

此说明适用于本包的 P75 JIS PRE-FIX01 固件及[配套 VIA JSON](../artifacts/kibu_p75_jis_via_pre_fix01.json)，VID／PID 为 `36B0`／`3156`。VIA 配置仅支持有线 USB；蓝牙和 2.4G 是输入连接方式，不是本资料承诺的配置通道。

## 连接步骤

1. 将键盘切换到有线 USB 模式，用可传输数据的 USB 线连接电脑。
2. 使用 Chrome 或 Edge 打开 [VIA](https://usevia.app/)。
3. 在 VIA 设置中启用 `Show Design tab`，进入 Design 页面。
4. 加载本包的 `kibu_p75_jis_via_pre_fix01.json`。
5. 返回 Configure，授权并选择 P75 JIS（VID `36B0`、PID `3156`）。确认出现对应 JIS 布局后使用改键功能。

本说明采用手动加载厂家定义的方式，不代表此型号已被 VIA 官方数据库收录。VIA 的界面文案可能调整，核心步骤是加载配套定义、授权 USB 设备后进入配置。

## 必须使用配套定义

源码快照内的历史 `P75 JIS.JSON` 漏列 `MW_CH`，可能使后续自定义键码显示错位。本包外部 JSON 只补上这一项，不改变固件；其 SHA-256 为 `2CB4B1F7E9222060C3F55F353128A7278AD6C953ED0B327E95EB1D8089B025BF`。

设备定义 JSON 描述布局和功能，不是用户保存的改键备份。不要用其他型号或较新开发固件的定义替换；已有配置需要保留时，先使用 VIA 提供的布局保存功能备份。

## 无法识别时

确认 USB 模式、数据线、浏览器设备权限，以及是否已加载本包 JSON。记录浏览器显示的设备名和故障步骤，通过[官网售后页面](https://kibushop.com/pages/contact)或 [m@kibushop.com](mailto:m@kibushop.com)反馈。设备名称保留原版本形式，名称中的空格不是升级理由；仅为使用本说明或获得源码，无需先刷写固件。

版本差异及验证范围见[版本与测试](VERSION_AND_TESTS.md)，保留的固件风险见[已知问题](KNOWN_ISSUES.md)。
