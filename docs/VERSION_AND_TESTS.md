# Version and Test Record

## Fixed baseline

Product: KIBU P75 JIS. This release uses the PRE-FIX01 snapshot delivered on August 12, 2026. The build target is `p75_jis/p75_jis:via`; the recorded toolchain is GNU Arm Embedded `10.3-2021.10` / GCC `10.3.1`. A later development commit or a different toolchain output is not a substitute for this baseline.

| Artifact | SHA-256 |
| --- | --- |
| Original `qmk_firmware_pre_fix01_exact_source_20260807.zip` | `B5432FB8B39ADFD91D3196C08056C09BC3D90542B151631B6AFB9C87A8119846` |
| [Reference BIN](../artifacts/kibu_p75_jis_pre_fix01_hardware_validated.bin), 80,860 bytes | `21DE5AAFA6684B72AD797C47C61712F8D40F5762477DCBA483DC7BB53A4332B2` |
| [Matching VIA definition](../artifacts/kibu_p75_jis_via_pre_fix01.json) | `2CB4B1F7E9222060C3F55F353128A7278AD6C953ED0B327E95EB1D8089B025BF` |

The BIN is a retained historical artifact. [BUILD.md](BUILD.md) records its relationship to rebuilt output. Attribution, excluded Git pointers, and documentation/comment translations mean the publication copy is not byte-identical to the original ZIP; see [provenance](SOURCE_AND_LICENSES.md). None of these documentation changes is a new firmware feature.

## Device identity

- VID/PID: `0x36B0` / `0x3156`; source product name: `P75 JIS`; manufacturer: `RDMCTMZT`. Original identifiers are retained.
- The 0812 BIN reports USB device version `0004` and no serial number; the source uses VIA protocol version `12`.
- A previously examined sample, before flashing, reported USB `0002`, serial `20250901`, and VIA `12`. That difference remains on record. The sample's original BIN has not been shown to be byte-identical to the packaged BIN.
- The product owner's production-source attestation is the basis for selecting the 0812 package. It is not a verified production commit, batch-specific programming hash, or substitute for testing.

## User-reported tests and acceptance: September 23–24, 2026

The specified firmware and JSON are the files in the table above. The user operated an existing P75 JIS sample and reported the results; the observations are associated with those specified artifacts. They are not automated readback or verification of program Flash. On September 24 the user explicitly accepted the source, build materials, documentation, and this test delivery.

| Check | Result and evidence boundary |
| --- | --- |
| USB input and encoder | User confirmed typing, key release, and encoder volume control, with particular attention to W/M. No per-key trace or stress test was recorded. |
| VIA remapping and persistence | User confirmed temporary Q-to-A remapping, retention after complete power removal, and restoration. No automated settings snapshot was taken. |
| Macros, lighting, and persistence | User confirmed operation and retention after power removal. An earlier macro observation was caused by testing a different keyboard and was not established as a defect in this sample. |
| Bluetooth channels 1/2/3 | User confirmed the stated USB-disconnected input, key-release, and power-cycle reconnection checks. Whether re-pairing was necessary was not reported separately. |
| USB/Bluetooth switching and Bluetooth wake | User confirmed input after switching and input recovery after Bluetooth idle sleep. |
| 2.4 GHz input, switching, reconnection, and wake | **Not tested.** The receiver was unavailable and the user chose to omit these checks. They are not counted as passed. |

Acceptance of this delivery did not itself establish publication or the firmware installed across every production batch. Basic-operation results do not establish stress, battery, reliability, or fault-injection coverage.

## Historical build and preparation evidence: September 23, 2026

| Evidence | Supported conclusion | Limitation |
| --- | --- | --- |
| Bundled August 7 static verification | Records a source build, lint, 164/164 external symbols, and no link dependency on the old archive. | Historical evidence, not a rerun. The record explicitly states `Hardware tests: NOT RUN`. |
| August 12 delivery notes and later feedback | Report normal sample operation and working VIA after flashing. | No complete connection-mode, sleep, battery, persistence, or rollback test matrix was found. |
| Source and artifact comparison | Fixes the ZIP, BIN, and JSON identities; the supplied definition adds the missing `MW_CH`. | Does not replace build, fault-injection, or device testing. |
| Two clean builds | The current-date build differed by seven bytes; with attribution and the historical date, the 80,860-byte BIN matched the reference exactly. | Establishes correspondence with the archived BIN, not the original USB0002 sample or every production batch. See [BUILD.md](BUILD.md). |

Tests for other revisions must not be reassigned to this baseline. Historical task KIBUP75JIS-9 covered `d29767a6`, a BIN hash beginning `320FD89C`, and GCC `15.2`; subsequent Bluetooth-migration sample results also concern another version.

During the September 23 documentation-preparation stage, no new flashing, pairing, reset, remapping, or hardware test was performed. The user initially deferred device tests because an unmodified production sample and original receiver were unavailable. The later September 23–24 user reports above supersede that initial deferral only for their stated coverage. The receiver-dependent 2.4 GHz checks remain untested.

## Publication and documentation revision

Public-source acquisition, dependency download, build reproduction, CI, and release-asset checks are recorded on the [release page](https://github.com/kibu-keyboards/qmk_firmware/releases/tag/p75-jis-0812-r1). This English documentation revision does not introduce additional device-test results. The [known issues](KNOWN_ISSUES.md) remain, and customers do not need a firmware upgrade merely to use the published materials.

## 日本語

対象はP75 JISの0812版PRE-FIX01で、参照用BINは80,860バイト、VIAプロトコルはバージョン12です。表に示したBINとJSONを指定し、USB入力とノブ、キー割り当ての変更と電源断後の保持、マクロ、ライティング、Bluetoothの3チャネル、USB／Bluetooth切り替え、およびBluetoothスリープ復帰をユーザーが確認しています。2.4 GHzの入力・切り替え・再接続・復帰は、受信機が手元にないため未検証です。これらはユーザーによる操作結果の報告であり、プログラムFlashの読み戻しや全製造ロットの書き込み内容の確認ではありません。過去の書き換え前サンプルと参照BINにはUSBバージョン等の差異があり、同一バイナリーだったとは断定していません。今回の文書・コメント改訂による追加の実機検証はなく、耐久・電池・異常系の試験結果も追加していません。別バージョンの検証結果は本版の結果として扱いません。
