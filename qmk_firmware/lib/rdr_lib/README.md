# RDR Common Source Implementation

`rdr_common.c` is the source implementation of the FS026-side keyboard
integration that was previously supplied only as `librdrcommon.a`.

The file is compiled directly by:

```make
SRC += $(LIB_PATH)/rdr_lib/rdr_common.c
```

No vendor archive or reconstructed object file is required to link the P75 JIS
firmware.

## Implemented scope

- QMK host-driver forwarding for keyboard, NKRO, mouse, system, and consumer
  reports.
- The fixed-size SPI protocol used to communicate with the separate wireless
  controller.
- BLE channel and 2.4 GHz mode commands, pairing commands, device-name writes,
  and connection-state handling.
- Battery ADC sampling, level filtering, charge-state handling, and battery
  updates to the wireless controller.
- Board GPIO, timers, DMA/SPI interrupt handling, power states, wake sources,
  USB suspend/resume, and bootloader entry.
- Delayed EEPROM persistence and keyboard runtime state.

## Verification completed

- A clean source-only build of `p75_jis/p75_jis:via` succeeds with GCC Arm
  Embedded 10.3-2021.10.
- All 57 functions expected from the former archive have source definitions.
- All 164 externally visible functions and data symbols from the extracted
  vendor object are present in the compiled source object. The final ELF keeps
  160 and garbage-collects four definitions that this keyboard does not call.
- Initialized public data, including `Keyboard_Info`, matches the vendor build.
- No `.a` or replacement `.o` file is needed by the build.

These checks establish source and link completeness. They do not prove radio,
power-management, timing, or bootloader behavior on production hardware.

## Compatibility issue retained from the vendor implementation

The original implementation allocates the report ring as 40 slots of 24 bytes,
but starts a 64-byte SPI DMA transfer from each slot. The source implementation
currently retains that behavior for binary-behavior parity. It can read across
adjacent slots, truncates the stored part of long reports, and can read beyond
the final slot. This must be treated as a known defect, not as a safe API.

A hardened follow-up can change the ring to 15 slots of 64 bytes while keeping
the same 960-byte RAM allocation. That change should only be released after
hardware testing because it intentionally changes queue depth and vendor
behavior.

The inherited SPI polling and DMA wait paths also have no timeout in several
places. A stalled wireless controller may therefore block the QMK MCU.

## Required hardware validation

1. USB typing, VIA remapping, NKRO, mouse, media, and system reports.
2. BLE channels 1-3 pairing, reconnect, switching, and wake behavior.
3. 2.4 GHz pairing, reconnect, report delivery, and wake behavior.
4. Wired/wireless mode switching and unplug/replug transitions.
5. Battery percentage, charging indication, low-battery handling, and shutdown.
6. Idle sleep and deep-sleep entry and wake from keys, encoder, mode switch,
   and USB insertion.
7. EEPROM persistence across reset and full power removal.
8. ESC bootloader entry, drag-and-drop BIN flashing, and automatic restart.

## Licensing note

The implementation carries `GPL-2.0-or-later` to match the surrounding QMK
firmware. Before public distribution, the publisher should confirm that it has
the necessary rights to release any vendor-derived behavior, headers, SDK code,
and board support files included in the complete repository.
