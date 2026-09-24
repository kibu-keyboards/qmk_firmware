# Hardware Revision Notes (Historical)

These engineering notes were delivered with the product source. They describe
historical hardware/firmware combinations and changes; they are not validation
results for the published 0812 snapshot. For the current release build procedure,
use [the release build guide](../../../../docs/BUILD.md).

Descriptive portions of the original revision labels have been translated to
English below. Model identifiers, checksums, version numbers, dates, and RF
suffixes are retained. The original labels remain available in Git history.
All entries describe the Japanese-layout, tri-mode mechanical keyboard, with
WS2812 backlighting, WS2812 logo LEDs, and USB VID/PID `0x36B0/0x3156`.

## V0100 — 2026-02-05

- Revision label: `FS026+91680-(SN003)Spectra75-JP-Japanese-Tri-mode-Mechanical-Keyboard-CSBD81-V0100-20260205_NEW_RF3_0`
- Previous revision: none recorded.
- Checksum: `BD81 (0x0100)`.
- Other notes: none recorded.
- RF requirement: must be paired with the new RF firmware and receiver.
- Change: initial project revision.

## V0101 — 2026-04-25

- Revision label: `RDM026+91680-(SN003)P75_JIS-Japanese-Tri-mode-Mechanical-Keyboard-CSCD82-V0101-20260425_NEW_RF3_0`
- Previous revision label as recorded: `RDM026+91680-(SN003)Spectra75-Japanese-Tri-mode-Mechanical-Keyboard-CSBD81-V0100-20260205_NEW_RF3_0`
- Checksum: `CD82 (0x0101)`.
- Other notes: none recorded.
- RF requirement: must be paired with the new RF firmware and receiver.
- Changes:
  1. Changed the device name to `P75 JIS` and the Bluetooth names to `P75 JIS BT1/2/3`.
  2. Fixed Japanese keys that did not respond in Mac mode.

## V0102 — 2026-05-15

- Revision label: `RDM026+91680-P75_JIS-Japanese-Tri-mode-Mechanical-Keyboard-CSE1FE-V0102-20260515_NEW_RF3_0`
- Previous revision label as recorded: `RDM026+91680-P75_JIS-Japanese-Tri-mode-Mechanical-Keyboard-CSCD82-V0101-20260425_NEW_RF3_0`
- Checksum: `E1FE (0x0102)`.
- Other notes: none recorded.
- RF requirement: must be paired with the new RF firmware and receiver.
- Change: improved automatic recognition of the Japanese keyboard layout on macOS after power-on.

## V0103 — 2026-07-20

- Revision label: `RDM026+91680-P75_JIS-Japanese-Tri-mode-Mechanical-Keyboard-CS54F0-V0103-20260720_NEW`
- Previous revision label as recorded: `RDM026+91680-P75_JIS-Japanese-Tri-mode-Mechanical-Keyboard-CSE1FE-V0102-20260515_NEW`
- Checksum: `54F0 (0x0103)`.
- Other notes: none recorded.
- RF requirement: must be paired with RF 3.0 firmware and its receiver.
- Change: updated the dot-matrix glyphs for Japanese keys in the typewriter lighting effect.

## V0104 — 2026-07-22

- Revision label: `RDM026+91680-P75_JIS-Japanese-Tri-mode-Mechanical-Keyboard-CS1328-V0104-20260722_NEW`
- Previous revision label as recorded: `RDM026+91680-P75_JIS-Japanese-Tri-mode-Mechanical-Keyboard-CS54F0-V0103-20260720_NEW`
- Checksum: `1328 (0x0104)`.
- Other notes: none recorded.
- RF requirement: must be paired with RF 3.0 firmware and its receiver.
- Changes:
  1. Updated the typewriter-effect glyphs for the Muhenkan, Henkan, and Kana keys on macOS.
  2. Updated the typewriter-effect glyphs for multimedia keys.
