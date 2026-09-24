# KIBU P75 JIS — 0812 Corresponding Source

This release provides the QMK/FS026 source, build dependencies, license notices, and VIA device definition corresponding to the P75 JIS PRE-FIX01 package supplied on August 12, 2026. The release tag is `p75-jis-0812-r1`.

**No firmware update is required to use this source release.** The included BIN is a reference artifact. VIA configuration requires wired USB; Bluetooth and 2.4 GHz are input connections.

## Downloads and setup

- [Release files](https://github.com/kibu-keyboards/qmk_firmware/releases/tag/p75-jis-0812-r1): `KIBU-P75-JIS-0812-r1-docs2-source.zip`, the matching `kibu_p75_jis_via_pre_fix01.json`, and `SHA256SUMS.txt`.
- [VIA setup](docs/VIA_GUIDE.md): connect over USB and load the supplied device definition manually.
- [Tagged source](https://github.com/kibu-keyboards/qmk_firmware/tree/p75-jis-0812-r1) and [exact checkout instructions](docs/GIT_IMPORT.md).
- [Build workflow](https://github.com/kibu-keyboards/qmk_firmware/actions/workflows/p75-0812.yml): see the run for the release commit.

## Source and documentation

The [firmware source](qmk_firmware/) includes USB input and the FS026-side SPI bridge for Bluetooth and 2.4 GHz operation. ChibiOS, ChibiOS-Contrib, and FS026 platform sources are included as fixed files. Internal firmware for the separate 91680 wireless controller is outside this package; see [source and licenses](docs/SOURCE_AND_LICENSES.md) for component provenance and license details.

| Reference | Contents |
| --- | --- |
| [Build instructions](docs/BUILD.md) | Toolchain, fixed dependencies, and reproducible build procedure. |
| [Version reference](docs/VERSION_AND_TESTS.md) | Baseline, artifact hashes, and device identifiers. |
| [Known issues](docs/KNOWN_ISSUES.md) | Reliability risks in the PRE-FIX01 baseline. |
| [Release notes](docs/RELEASE_NOTES.md) | Release scope and downloads. |
| [Reference BIN](artifacts/kibu_p75_jis_pre_fix01_hardware_validated.bin) and [VIA definition](artifacts/kibu_p75_jis_via_pre_fix01.json) | Firmware reference and matching device definition. |

## Support

Contact [KIBU support](https://kibushop.com/pages/contact) or [m@kibushop.com](mailto:m@kibushop.com). Include the model, connection mode, firmware and definition versions, and steps to reproduce the issue.

## 日本語

本リリースは、KIBU P75 JISの0812版PRE-FIX01に対応するQMK／FS026側のソースコード、ビルド依存ファイル、ライセンス表示、およびVIAデバイス定義を提供します。`KIBU-P75-JIS-0812-r1-docs2-source.zip` と付属のチェックサムをご利用ください。VIAの設定はUSB有線接続で行い、指定のJSONを手動で読み込みます。本資料の利用に伴うファームウェア更新は不要です。USB入力とBluetooth／2.4 GHz用のFS026側通信処理を含みますが、別体の91680無線コントローラー内部ファームウェアは公開範囲外です。ビルド方法、バージョン情報、ライセンス、既知の問題は上記の各資料をご参照ください。お問い合わせは、機種名、接続方式、使用バージョン、再現手順を添えて [KIBUサポート](https://kibushop.com/pages/contact) または [m@kibushop.com](mailto:m@kibushop.com) へお願いいたします。
