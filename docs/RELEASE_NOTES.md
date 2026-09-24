# KIBU P75 JIS — 0812 Corresponding Source (R1)

> Modified by KIBU (kibu-keyboards) (M@kibu.jp) on 2026-09-24. Changes: Translated and edited the English release scope, download guidance, and revision notes; added a Japanese summary. Original authorship and license notices are retained.

This release provides the QMK/FS026 source, build dependencies, copyright and license notices, and VIA device definition for the P75 JIS PRE-FIX01 baseline supplied on August 12, 2026. The release tag is `p75-jis-0812-r1`.

## Downloads and use

- [Release files](https://github.com/kibu-keyboards/qmk_firmware/releases/tag/p75-jis-0812-r1): `KIBU-P75-JIS-0812-r1-notices-source.zip`, `kibu_p75_jis_via_pre_fix01.json`, `PUBLIC_RELEASE_NOTES.md`, and `SHA256SUMS.txt`.
- [Tagged source](https://github.com/kibu-keyboards/qmk_firmware/tree/p75-jis-0812-r1): start with the root `README.md` for documentation and build instructions.
- Configure VIA over wired USB by manually loading the supplied definition. See `docs/VIA_GUIDE.md` in the source package. Bluetooth and 2.4 GHz are input connections.
- No firmware update is required to use this source release. The included BIN is a reference artifact.
- Support: [KIBU contact form](https://kibushop.com/pages/contact) or [m@kibushop.com](mailto:m@kibushop.com).

## Source scope

The package includes USB input, the FS026-side SPI bridge for Bluetooth and 2.4 GHz operation, and fixed ChibiOS, ChibiOS-Contrib, and FS026 platform sources. No private dependency repository is required. Internal firmware for the separate 91680 wireless controller is outside this package. Component provenance and license details are in `docs/SOURCE_AND_LICENSES.md`; baseline reliability risks are in `docs/KNOWN_ISSUES.md`.

This revision adds dated KIBU modification notices for translated documentation and source comments. Firmware behavior, the reference BIN, and the VIA definition are unchanged.

## Build and checksums

The Windows build procedure uses GNU Arm Embedded `10.3-2021.10` and a fixed build date to reproduce the 80,860-byte reference BIN. See `docs/BUILD.md` for the procedure and the [build workflow](https://github.com/kibu-keyboards/qmk_firmware/actions/workflows/p75-0812.yml) for release-commit results.

| Reference artifact | SHA-256 |
| --- | --- |
| Reference BIN, 80,860 bytes | `21DE5AAFA6684B72AD797C47C61712F8D40F5762477DCBA483DC7BB53A4332B2` |
| Matching VIA definition | `2CB4B1F7E9222060C3F55F353128A7278AD6C953ED0B327E95EB1D8089B025BF` |

Use the downloadable `SHA256SUMS.txt` to verify the current source archive and release notes.

## 日本語

KIBU P75 JISの0812版PRE-FIX01に対応するソースコード、ビルド依存ファイル、ライセンス表示、およびVIAデバイス定義を提供します。`KIBU-P75-JIS-0812-r1-notices-source.zip` と付属のチェックサムをご利用ください。今回の改訂では、翻訳した文書とソースコード内コメントにKIBUの変更者・日付・変更内容を追記しました。ファームウェアの動作、参照用BIN、VIA定義は変更していません。お客様によるファームウェア更新は不要です。VIAの設定はUSB有線接続で行い、指定のJSONを手動で読み込んでください。USB入力とBluetooth／2.4 GHz用のFS026側通信処理を含みますが、別体の91680無線コントローラー内部ファームウェアは公開範囲外です。ビルド方法、ライセンス、既知の問題は同梱資料をご参照ください。お問い合わせは [KIBUサポート](https://kibushop.com/pages/contact) または [m@kibushop.com](mailto:m@kibushop.com) へお願いいたします。

変更記録：KIBU（kibu-keyboards、M@kibu.jp）が2026年9月24日に本資料の英訳・編集および日本語案内の追加を行いました。原著者の表記、著作権表示、ライセンス表示は保持しています。
