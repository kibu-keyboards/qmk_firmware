# Source Provenance and Exact Checkout

`p75-jis-0812` is an independent snapshot branch in the existing KIBU fork. Its initial commit has no parent and records publication preparation, not an asserted historical production commit. The existing `master` and `kibu` branches retain their upstream history. The published release tag is `p75-jis-0812-r1`.

The input was the accepted 0812 corresponding-source package. The September 23 candidate ZIP has SHA-256 `0362C3BE9EE3FB8ECB40E9F0A6A4199D866D6D2687C53DD955C17A65A4B12C13`; the September 24 candidate incorporating the device-test update has SHA-256 `30992703061754763E61C5D5994D385DB2618B3AEECD93463B0684502C922747`. Publication added stable download links, this provenance record, and Windows build automation.

The English documentation revision translates KIBU publication text and product comments and gives two product-reference documents readable English filenames. It does not change executable code, firmware behavior, the reference BIN, or the VIA definition. Upstream language resources and original third-party attribution remain intact. Original-input metadata retains its dated meaning; the release manifest describes the revised files.

Nine unusable `.git` pointer files were excluded from the original snapshot. Dependencies are ordinary files, not gitlinks requiring private repositories. The historical `qmk_firmware/.gitmodules` is retained for provenance and must not be used to replace the fixed dependencies. Source is under `qmk_firmware/`; build entry points are under `tools/`.

## Preserve exact file bytes

The source retains its original `.gitattributes` and `.gitignore`. During import, local `.git/info/attributes` disabled text, end-of-line, filter, and ident transformations; explicit file selection prevented ignore rules from omitting required inputs. These are local repository settings, not changes to global Git configuration.

A normal checkout may transform files according to the original attributes. To verify the release manifest byte for byte, download the release ZIP or run the following in Bash using a new directory:

```bash
git clone --no-checkout --branch p75-jis-0812-r1 https://github.com/kibu-keyboards/qmk_firmware.git p75-jis-0812-r1
cd p75-jis-0812-r1
printf '* -text -eol -filter -ident\n' > .git/info/attributes
git checkout --detach p75-jis-0812-r1
```

No submodule checkout is required. In its fresh workspace, CI sets the exact-byte attributes and extracts `git archive HEAD` over the initial checkout. This avoids an index-cache shortcut retaining line endings from the initial checkout. `metadata/release-manifest.csv` records the size and SHA-256 of every release file except itself; the Git commit and ZIP checksum cover the manifest.

KIBU publication commits use `KIBU <M@kibu.jp>` and are published through `kibu-keyboards`. This publication identity does not replace the authorship or license notices of upstream components. Consult the release verification record for the current commit and archive checksum rather than relying on a branch name alone.
