# PRE-FIX01 Known Issues

> Modified by KIBU (kibu-keyboards) (M@kibu.jp) on 2026-09-24. Changes: Translated and edited the English descriptions of known baseline risks; added a Japanese summary. Original authorship and license notices are retained.

These source-analysis findings apply to the 0812 PRE-FIX01 baseline and its reference BIN. They identify potential failure conditions, not hardware reproductions. Line references use the original files with normalized line endings.

Implementation: [rdr_common.c](../qmk_firmware/lib/rdr_lib/rdr_common.c) and [rdr_common.h](../qmk_firmware/lib/rdr_lib/rdr_common.h).

| Finding | Evidence and potential impact |
| --- | --- |
| Queue-record size differs from the SPI read length | Header lines 32 and 345–346 define 64-byte frames and forty 24-byte slots. C line 427 copies only 24 bytes; line 699 passes a queue slot with a 64-byte length. The read can include adjacent slots or extend beyond the final slot; reports longer than 24 bytes are truncated on enqueue. The FIX1 zero-initialized staging-buffer fix is not included. |
| SPI polling has no timeout | `es_spi_send_recv_by_dma` at C lines 646–675 disables interrupts and polls the FIFO, despite its name. The waits have no timeout; an unresponsive peer can stall the MCU. |
| USB suspend wait has no timeout | C lines 1349–1350 wait for a DMA status bit without a timeout. An abnormal state can block suspend handling. |
| A full queue drops new reports | C lines 395–396 return immediately when the queue is full. Key-down or key-up reports may be lost, potentially leaving a key logically held. |
| USB wake uses limited layer/keycode handling | C lines 1511 and 1514 read a layer-0 keycode into `uint8_t`. Active-layer selection and keycodes wider than eight bits may not be handled as intended. |
| Status parsing lacks a separate integrity check | The call and parsing paths at C lines 766–835 and 1615 do not perform a separate length/integrity check. Malformed or incomplete status frames may be accepted or misinterpreted. |

The baseline does not include FIX1 or later Raw HID/encoder, Bluetooth-name migration, and FS026 guard changes.

Report issues through [KIBU support](https://kibushop.com/pages/contact), including the model, connection mode, firmware/JSON versions, reproduction steps, and observed behavior. See [VIA setup](VIA_GUIDE.md) for configuration. No firmware update is required solely to use the published source.

## 日本語

本ページは、0812版PRE-FIX01のソースコード解析で確認された潜在的な不具合をまとめたものです。キューの24バイト記録とSPIの64バイト読み出しの不一致、SPIおよびUSBサスペンド処理のタイムアウト不足、キュー満杯時のレポート破棄、USB復帰時のレイヤー／キーコード処理の制約、状態フレームの検証不足が該当します。各現象を実機で再現したことを示すものではありません。本版にはFIX1および後続のRaw HID、エンコーダー、Bluetooth名移行、FS026ガード処理の変更は含まれません。問題が生じた場合は、機種名、接続方式、使用バージョン、再現手順と症状を添えて [KIBUサポート](https://kibushop.com/pages/contact) へご連絡ください。VIAの設定手順は [VIAガイド](VIA_GUIDE.md) をご参照ください。ソースコードの利用だけを目的としたファームウェア更新は不要です。

変更記録：KIBU（kibu-keyboards、M@kibu.jp）が2026年9月24日に本資料の英訳・編集および日本語案内の追加を行いました。原著者の表記、著作権表示、ライセンス表示は保持しています。
