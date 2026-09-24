# PRE-FIX01 Known Issues

These findings apply to the 0812 source baseline and its reference BIN. They are risks identified from source inspection, not claims that each failure was reproduced on hardware. This release does not change the affected behavior or add fault-injection testing. Reports of normal sample operation do not rule out these conditions. Line references use the original files with normalized line endings; comment translations retain their positions where practicable.

Implementation: [rdr_common.c](../qmk_firmware/lib/rdr_lib/rdr_common.c) and [rdr_common.h](../qmk_firmware/lib/rdr_lib/rdr_common.h).

| Finding | Evidence and potential impact |
| --- | --- |
| Queue-record size differs from the SPI read length | Header lines 32 and 345–346 define 64-byte frames and forty 24-byte slots. C line 427 copies only 24 bytes; line 699 passes a queue slot with a 64-byte length. The read can include adjacent slots or extend beyond the final slot; reports longer than 24 bytes are truncated on enqueue. The FIX1 zero-initialized staging-buffer fix is not included. |
| SPI polling has no timeout | `es_spi_send_recv_by_dma` at C lines 646–675 disables interrupts and polls the FIFO, despite its name. The waits have no timeout; an unresponsive peer can stall the MCU. Historical notes call the preceding finding a “64-byte DMA read”; this document describes the actual SPI implementation. |
| USB suspend wait has no timeout | C lines 1349–1350 wait for a DMA status bit without a timeout. An abnormal state can block suspend handling. |
| A full queue drops new reports | C lines 395–396 return immediately when the queue is full. Key-down or key-up reports may be lost; user-visible effects such as a held key require targeted device testing. |
| USB wake uses limited layer/keycode handling | C lines 1511 and 1514 read a layer-0 keycode into `uint8_t`. Active-layer selection and keycodes wider than eight bits may not be handled as intended. |
| Status parsing lacks a separate integrity check | The call and parsing paths at C lines 766–835 and 1615 do not perform a separate length/integrity check. Handling of malformed or incomplete status frames has not been validated. |

Later Raw HID/encoder fixes, Bluetooth-name migration, and FS026 guard changes have not been backported. Results from a different firmware revision do not establish this baseline's behavior.

VIA configuration is supported over wired USB only. Manual definition loading does not imply inclusion in VIA's official database or validation of KIBU's advanced Web features. Use the [supplied definition](../artifacts/kibu_p75_jis_via_pre_fix01.json), which includes `MW_CH`, rather than the historical `P75 JIS.JSON` inside the source snapshot.

Report issues through [KIBU support](https://kibushop.com/pages/contact), including the model, connection mode, firmware/JSON versions, reproduction steps, and observed behavior. Customers do not need to flash firmware solely because the source has been published or a device name differs.

## 日本語

本ページは0812版PRE-FIX01に残る、ソースコード調査で確認されたリスクを整理したものです。キューの24バイト記録とSPIの64バイト読み出しの不一致、SPIおよびUSBサスペンド処理のタイムアウト不足、キュー満杯時のレポート破棄、USB復帰時のレイヤー／キーコード処理の制約、状態フレームの検証不足が該当します。各現象を実機で再現したという意味ではなく、今回これらの動作は変更していません。基本動作の確認だけで、まれな障害が発生しないことまでは保証できません。VIAの設定にはUSB有線接続と同梱のデバイス定義を使用してください。問題が生じた場合は、接続方式、使用バージョン、再現手順を添えて [KIBUサポート](https://kibushop.com/pages/contact) へご連絡ください。ソース公開や表示名の違いだけを理由に、ファームウェアを書き換える必要はありません。
