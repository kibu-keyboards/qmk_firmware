# P75 JIS — VIA Setup

> Modified by KIBU (kibu-keyboards) (M@kibu.jp) on 2026-09-24. Changes: Translated and clarified the English VIA setup instructions; added Japanese customer guidance. Original authorship and license notices are retained.

This guide applies to the P75 JIS PRE-FIX01 firmware and the [matching VIA definition](../artifacts/kibu_p75_jis_via_pre_fix01.json), VID/PID `36B0` / `3156`. Configuration requires wired USB. Bluetooth and 2.4 GHz are input connections; wireless configuration is not supported.

## Connect and load the definition

1. Select wired USB mode on the keyboard and connect it with a USB cable that supports data transfer.
2. Open [VIA](https://usevia.app/) in Chrome or Edge.
3. Enable `Show Design tab` in VIA settings, then open **Design**.
4. Load the supplied `kibu_p75_jis_via_pre_fix01.json`.
5. Return to **Configure**, authorize the USB device, and select P75 JIS (VID `36B0`, PID `3156`). Confirm that the expected JIS layout appears before remapping keys.

This workflow manually loads a manufacturer-provided definition; it does not imply that the model is included in VIA's official device database. Interface labels may change, but the required sequence is to load the matching definition, authorize the USB device, and open configuration.

## Use the matching file

The historical `P75 JIS.JSON` inside the source snapshot omits `MW_CH`, which can offset subsequent custom-keycode labels. The supplied external definition adds that entry without changing firmware. Its SHA-256 is:

```text
2CB4B1F7E9222060C3F55F353128A7278AD6C953ED0B327E95EB1D8089B025BF
```

A device definition describes the keyboard's layout and capabilities; it is not a backup of your remapped keys. Do not substitute a definition for another model or a later development firmware. If you need to preserve an existing layout, save a backup using VIA before making changes.

## If the keyboard is not detected

Check wired mode, the data cable, browser device permission, and the loaded definition. Record the device name shown by the browser and the steps that fail, then contact [KIBU support](https://kibushop.com/pages/contact) or [m@kibushop.com](mailto:m@kibushop.com).

The firmware retains its original device names. Spaces or other historical naming details are not, by themselves, a reason to update firmware. No preliminary flashing is required solely to use this guide or obtain the source. See [version reference](VERSION_AND_TESTS.md) and [known issues](KNOWN_ISSUES.md).

## 日本語

キーボードをUSB有線モードに切り替え、データ通信対応のUSBケーブルで接続してください。ChromeまたはEdgeで [VIA](https://usevia.app/) を開き、設定の `Show Design tab` を有効にして **Design** から同梱の `kibu_p75_jis_via_pre_fix01.json` を読み込みます。**Configure** に戻り、USBデバイスへのアクセスを許可してP75 JIS（VID `36B0`／PID `3156`）を選択し、JIS配列が表示されることを確認してください。設定はUSB接続のみ対応し、Bluetooth／2.4 GHz経由では行いません。この手順はメーカー提供定義の手動読み込みであり、VIA公式データベースへの登録を意味しません。ソース内の旧 `P75 JIS.JSON` には `MW_CH` が欠けているため使用せず、必ず指定ファイルをご利用ください。デバイス定義は個人設定のバックアップとは異なります。既存のキー割り当てを残す場合は、変更前にVIAで保存してください。認識されない場合は接続モード、ケーブル、アクセス許可、読み込んだ定義を確認し、表示名と再現手順を添えて [KIBUサポート](https://kibushop.com/pages/contact) へご連絡ください。本資料の利用だけを目的としたファームウェアの書き換えは不要です。

変更記録：KIBU（kibu-keyboards、M@kibu.jp）が2026年9月24日に本資料の英訳・編集および日本語案内の追加を行いました。原著者の表記、著作権表示、ライセンス表示は保持しています。
