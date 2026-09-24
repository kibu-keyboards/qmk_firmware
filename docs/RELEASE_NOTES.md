# KIBU P75 JIS — 0812 Corresponding Source (R1)

This release provides the QMK/FS026 source, build dependencies, copyright and license notices, and VIA definition corresponding to the PRE-FIX01 package delivered on August 12, 2026. KIBU's product owner has identified this source as the production baseline. The release tag is `p75-jis-0812-r1`; the release verification record identifies the current commit, downloadable artifacts, and CI results.

## Downloads and use

- [Release files](https://github.com/kibu-keyboards/qmk_firmware/releases/tag/p75-jis-0812-r1): `KIBU-P75-JIS-0812-r1-en-ja-source.zip`, `kibu_p75_jis_via_pre_fix01.json`, `PUBLIC_RELEASE_NOTES.md`, and `SHA256SUMS.txt`.
- [Tagged source](https://github.com/kibu-keyboards/qmk_firmware/tree/p75-jis-0812-r1). Extract the source ZIP and start with its root `README.md` for build and provenance information.
- Configure VIA over wired USB using the supplied definition. See `docs/VIA_GUIDE.md`; Bluetooth and 2.4 GHz are input connections, not configuration transports.
- No firmware update is required for this source release. The included BIN is a retained reference artifact, not a newly recommended upgrade. Its historical `hardware_validated` filename does not establish complete testing of all three connection modes.
- Support: [KIBU contact form](https://kibushop.com/pages/contact) or [m@kibushop.com](mailto:m@kibushop.com).

## Documentation revision

KIBU publication text and Chinese product-code comments have been translated into English. Customer-facing documents include Japanese guidance at the end. Two historical product-reference documents now have readable English filenames. Original upstream translations, fonts, multilingual test fixtures, identifiers, and third-party attribution are retained. The previous publication archive is preserved in the project records; the revised archive has a distinct filename and SHA-256.

Executable source, the reference BIN, and the VIA definition are unchanged. This is a documentation and comment revision, not a firmware feature update. The release manifest describes the current files; metadata named `original` and the dated build records continue to describe their original inputs and observations.

## Source scope and build correspondence

The package includes USB input and the FS026-side bridge for Bluetooth and 2.4 GHz operation, together with fixed ChibiOS, ChibiOS-Contrib, and FS026 source files. Private dependency repositories are not required. The internal firmware of the separate 91680 wireless controller is not included or audited. This is not a claim that the entire keyboard is open source. Original attribution is retained; provenance and outstanding license-evidence gaps are documented in `docs/SOURCE_AND_LICENSES.md`.

On September 23, 2026, a clean GCC 10.3.1 build with the required attribution and fixed historical VIA date reproduced the 80,860-byte reference BIN byte for byte. Later publication checks exercised anonymous source/dependency acquisition and public CI. Results for the current documentation commit are recorded separately below on the release page; earlier results are not presented as a new build.

| Reference artifact | SHA-256 |
| --- | --- |
| Historical BIN, 80,860 bytes | `21DE5AAFA6684B72AD797C47C61712F8D40F5762477DCBA483DC7BB53A4332B2` |
| Matching VIA definition | `2CB4B1F7E9222060C3F55F353128A7278AD6C953ED0B327E95EB1D8089B025BF` |
| September 23 source candidate, retained as provenance | `0362C3BE9EE3FB8ECB40E9F0A6A4199D866D6D2687C53DD955C17A65A4B12C13` |

Use the downloadable `SHA256SUMS.txt` for the current publication archive and notes.

## Device-test coverage

On September 23–24, 2026, the user tested the specified BIN and JSON on a P75 JIS sample and confirmed USB typing/encoder operation, VIA remapping and retention after power removal, macros/lighting and settings retention, Bluetooth channels 1/2/3, USB/Bluetooth switching, and wake from Bluetooth sleep. The product owner accepted that source/build/materials and device-test delivery.

**2.4 GHz input, switching, reconnection, and wake were not tested because the receiver was unavailable.** The user chose to omit that testing; it is not counted as passed. Whether Bluetooth required re-pairing was not separately reported. Device program memory was not independently read back, and these observations do not identify the firmware installed in every production batch. An earlier macro issue was traced to testing a different keyboard and was not established as a defect in this sample.

Basic-operation checks do not establish stress, battery, or fault-injection coverage. The SPI length/wait, USB suspend, queue, wake, and status-parsing risks remain documented in `docs/KNOWN_ISSUES.md`. Subsequent maintenance fixes and Bluetooth-name migration are not included. Manual VIA compatibility is not official database inclusion or validation of advanced Web features.

## 日本語

KIBU P75 JISの0812版PRE-FIX01に対応するソースコードとVIAデバイス定義を公開しています。今回の改訂は公開資料と製品コード内コメントの英文化、およびお客様向け日本語案内の追加です。実行コード、参照用BIN、VIA定義は変更しておらず、お客様によるファームウェア更新は不要です。`KIBU-P75-JIS-0812-r1-en-ja-source.zip` と同梱のチェックサムをご利用ください。VIAの設定はUSB有線接続で行い、指定のJSONを手動で読み込んでください。USB入力、ノブ、キー割り当て、マクロ、ライティング、設定の電源断後保持、Bluetoothの3チャネル、USB／Bluetooth切り替え、Bluetoothスリープ復帰はユーザー確認済みですが、受信機が手元にないため2.4 GHz接続は未検証です。別体の91680内部ファームウェアは公開・監査範囲に含まれず、全製造ロットの書き込み内容や未実施の耐久・電池・異常系試験を保証するものではありません。既知の問題は同梱資料に記載しています。お問い合わせは [KIBUサポート](https://kibushop.com/pages/contact) または [m@kibushop.com](mailto:m@kibushop.com) へお願いいたします。
