P75 JIS source-reimplementation hardware validation candidate
==============================================================

Firmware:
  p75_jis_p75_jis_via_reverse_parity_candidate.bin

Size:
  80860 bytes

SHA-256:
  21DE5AAFA6684B72AD797C47C61712F8D40F5762477DCBA483DC7BB53A4332B2

Build target:
  p75_jis/p75_jis:via

Toolchain used:
  GNU Arm Embedded Toolchain 10.3-2021.10 (GCC 10.3.1)

Build result:
  Clean source-only compile and link passed.
  QMK lint passed.
  No vendor archive or replacement object was linked.
  The final ELF contains no undefined symbols.

Flash status:
  NOT YET VALIDATED ON PHYSICAL HARDWARE.

This BIN is intended for a controlled test sample. Preserve a known-good vendor
firmware and recovery method before flashing. With the described mass-storage
bootloader, hold ESC while connecting the keyboard, copy this BIN to the
bootloader drive, wait for the copy to finish, and allow the keyboard to reboot.

The reimplementation intentionally retains the vendor report-ring behavior for
initial parity testing. See lib/rdr_lib/README.md for the known 24-byte-slot /
64-byte-DMA defect and the complete hardware test checklist.
