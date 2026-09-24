# KIBU P75 JIS — 0812 Corresponding Source

This repository provides the QMK/FS026 source, build dependencies, license notices, and VIA device definition corresponding to the PRE-FIX01 package delivered on August 12, 2026. KIBU's product owner has identified that package as the production-source baseline. The release tag is `p75-jis-0812-r1`; the [release page](https://github.com/kibu-keyboards/qmk_firmware/releases/tag/p75-jis-0812-r1) records the published files and verification results.

**An update is not required to use this source release.** The included BIN is a retained reference artifact, not a newly recommended firmware upgrade. Configuration is available over wired USB; Bluetooth and 2.4 GHz wireless are input connections.

## Downloads and support

- [Release files](https://github.com/kibu-keyboards/qmk_firmware/releases/tag/p75-jis-0812-r1): download `KIBU-P75-JIS-0812-r1-en-ja-source.zip`, the matching `kibu_p75_jis_via_pre_fix01.json`, and `SHA256SUMS.txt`.
- [Source at the release tag](https://github.com/kibu-keyboards/qmk_firmware/tree/p75-jis-0812-r1). See [source provenance and exact checkout](docs/GIT_IMPORT.md) before verifying the file manifest.
- [Build verification](https://github.com/kibu-keyboards/qmk_firmware/actions/workflows/p75-0812.yml): select the run for the commit identified on the release page.
- [VIA setup](docs/VIA_GUIDE.md): load the supplied device definition manually and configure the keyboard over USB.
- Support: [KIBU contact form](https://kibushop.com/pages/contact) or [m@kibushop.com](mailto:m@kibushop.com). Include the model, connection mode, firmware and definition versions, and steps to reproduce the issue.

## What is included

The source includes USB input and the FS026-side SPI bridge for Bluetooth and 2.4 GHz operation. ChibiOS, ChibiOS-Contrib, and the FS026 platform sources are included as fixed files; no private dependency repository is required. Firmware internal to the separate 91680 wireless controller is not included and has not been audited. This release does not claim that every component in the keyboard is open source, or that QMK or VIA has reviewed or endorsed the product.

| Reference | Purpose |
| --- | --- |
| [Firmware source](qmk_firmware/) | The 0812 snapshot, required encoder attribution, and English translations of product comments; executable code is unchanged. |
| [Build instructions](docs/BUILD.md) | Pinned inputs, toolchain setup, reproducible build procedure, and verification evidence. |
| [Source and licenses](docs/SOURCE_AND_LICENSES.md) | Component provenance, retained attribution, and outstanding evidence gaps. |
| [Version and test record](docs/VERSION_AND_TESTS.md) | Artifact hashes, device identity, and the limits of the reported test coverage. |
| [Known issues](docs/KNOWN_ISSUES.md) | Reliability risks retained in this baseline. |
| [Release notes](docs/RELEASE_NOTES.md) | Release scope and the documentation revision. |
| [Reference BIN](artifacts/kibu_p75_jis_pre_fix01_hardware_validated.bin) and [VIA definition](artifacts/kibu_p75_jis_via_pre_fix01.json) | Original reference firmware and the matching corrected device definition. The historical filename `hardware_validated` is not evidence that every connection mode passed testing. |

## Validation scope

The product owner accepted the source, build materials, and reported device tests on September 24, 2026. The user confirmed USB typing and encoder operation; VIA remapping and retention after power removal; macros and lighting with settings retention; all three Bluetooth channels; USB/Bluetooth switching; and wake from Bluetooth sleep. **2.4 GHz operation was not tested because the receiver was unavailable.** These are user-reported results tied to the specified BIN and JSON, not an independent readback of device program memory or proof of the firmware installed across every production batch.

The firmware retains the PRE-FIX01 behavior. FIX1, subsequent Raw HID/encoder fixes, Bluetooth-name migration, and later product-renaming changes have not been incorporated. Publication does not resolve the [known issues](docs/KNOWN_ISSUES.md).

This documentation revision replaces KIBU's Chinese publication text and product comments with English, with Japanese summaries for customer-facing material. Original upstream translations, font data, test fixtures, identifiers, and third-party notices remain intact. Firmware logic, the reference BIN, and the VIA definition are unchanged. Current file hashes are recorded in `metadata/release-manifest.csv`; historical input records remain historical evidence.

## 日本語

本リリースは、KIBU P75 JIS の2026年8月12日納品版（PRE-FIX01）に対応するQMK／FS026側のソースコード、ビルド依存ファイル、ライセンス表示、およびVIAデバイス定義を提供します。本資料の利用に伴うファームウェア更新は不要です。VIAの設定はUSB有線接続で行い、同梱の `kibu_p75_jis_via_pre_fix01.json` を手動で読み込んでください。USB入力、ノブ、キー割り当て、マクロ、ライティング、設定の電源断後保持、Bluetoothの3チャネル、USB／Bluetooth切り替え、およびBluetoothスリープ復帰については、指定ファイルを用いたユーザー確認結果を記録しています。受信機が手元にないため、2.4 GHz接続は未検証です。別体の91680無線コントローラー内部ファームウェアは本公開範囲に含まれず、製品全体の完全なオープンソース化やQMK／VIAの公式承認を示すものではありません。既知の問題と検証範囲をご確認のうえ、不具合は機種名、接続方式、使用バージョン、再現手順を添えて [KIBUサポート](https://kibushop.com/pages/contact) または [m@kibushop.com](mailto:m@kibushop.com) へご連絡ください。
