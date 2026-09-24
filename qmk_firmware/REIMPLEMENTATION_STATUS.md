# P75 JIS RDR Reimplementation Status

## Release status

This tree contains a buildable C source replacement for the former
`librdrcommon.a` dependency. It is a hardware-validation candidate, not a
production-approved firmware release.

The source-only build target is:

```text
p75_jis/p75_jis:via
```

The implementation and its detailed validation notes are in
`lib/rdr_lib/README.md`.

## What "complete" means here

- The former archive is no longer part of the source or link inputs.
- Every externally visible symbol required from the former object is provided.
- The complete QMK firmware links and emits BIN, HEX, ELF, and MAP artifacts.
- The major recovered control paths are implemented as maintainable C rather
  than decompiler pseudocode or inline assembly stubs.

It does not mean that RF timing and power-state behavior have already been
confirmed on a physical P75 JIS sample. Final release approval requires the
hardware test matrix in `lib/rdr_lib/README.md`.

## Publication boundary

Only the QMK/FS026-side integration is represented here. The separate wireless
controller firmware remains outside this repository and communicates with QMK
through the recovered SPI command/report protocol.
