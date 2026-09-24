# Version Reference

> Modified by KIBU (kibu-keyboards) (M@kibu.jp) on 2026-09-24. Changes: Translated and reorganized the English version and artifact reference; added a Japanese summary. Original authorship and license notices are retained.

## Baseline

| Field | Value |
| --- | --- |
| Product | KIBU P75 JIS |
| Firmware baseline | PRE-FIX01, supplied August 12, 2026 |
| Release tag | `p75-jis-0812-r1` |
| Build target | `p75_jis/p75_jis:via` |
| Toolchain | GNU Arm Embedded `10.3-2021.10` / GCC `10.3.1` |
| Build date | `2026-08-07-00:00:00`; see [build instructions](BUILD.md) before changing it. |

## Reference artifacts

| Artifact | SHA-256 |
| --- | --- |
| Original `qmk_firmware_pre_fix01_exact_source_20260807.zip` | `B5432FB8B39ADFD91D3196C08056C09BC3D90542B151631B6AFB9C87A8119846` |
| [Reference BIN](../artifacts/kibu_p75_jis_pre_fix01_hardware_validated.bin), 80,860 bytes | `21DE5AAFA6684B72AD797C47C61712F8D40F5762477DCBA483DC7BB53A4332B2` |
| [Matching VIA definition](../artifacts/kibu_p75_jis_via_pre_fix01.json) | `2CB4B1F7E9222060C3F55F353128A7278AD6C953ED0B327E95EB1D8089B025BF` |

The [release page](https://github.com/kibu-keyboards/qmk_firmware/releases/tag/p75-jis-0812-r1) provides `KIBU-P75-JIS-0812-r1-notices-source.zip` and `SHA256SUMS.txt` for the current download. The original source archive hash above identifies the input baseline, not the current publication ZIP. See [source provenance](SOURCE_AND_LICENSES.md) and [build instructions](BUILD.md).

## Device identifiers

| Field | Value |
| --- | --- |
| USB VID / PID | `0x36B0` / `0x3156` |
| Source product name | `P75 JIS` |
| Source manufacturer | `RDMCTMZT` |
| Reference BIN USB device version | `0004` |
| Reference BIN USB serial number | None |
| VIA protocol version | `12` |

Use the matching [VIA definition](VIA_GUIDE.md) for configuration over wired USB. No firmware update is required to use the published materials. The PRE-FIX01 [known issues](KNOWN_ISSUES.md) apply to this baseline.

## 日本語

対象はKIBU P75 JISの0812版PRE-FIX01です。ビルド対象は `p75_jis/p75_jis:via`、ツールチェーンはGNU Arm Embedded `10.3-2021.10`（GCC `10.3.1`）です。参照用BINは80,860バイトで、USB VID／PIDは `0x36B0`／`0x3156`、USBデバイスバージョンは `0004`、VIAプロトコルはバージョン `12` です。USBシリアル番号は設定されていません。ビルド日時はVIA設定の保持に影響するため、変更する場合はビルド手順をご確認ください。上表には元のソースアーカイブと参照用ファイルの識別情報を記載しています。現在の配布ZIPの照合には、リリースページの `SHA256SUMS.txt` を使用してください。VIAの設定にはUSB有線接続と指定のデバイス定義を使用します。本資料の利用に伴うファームウェア更新は不要です。

変更記録：KIBU（kibu-keyboards、M@kibu.jp）が2026年9月24日に本資料の英訳・編集および日本語案内の追加を行いました。原著者の表記、著作権表示、ライセンス表示は保持しています。
