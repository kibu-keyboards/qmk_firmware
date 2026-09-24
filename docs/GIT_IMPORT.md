# Source Provenance and Exact Checkout

The release tag is `p75-jis-0812-r1` on the `p75-jis-0812` snapshot branch. The branch records the supplied source snapshot; it does not reconstruct production Git history. The existing `master` and `kibu` branches retain their upstream history. See [source and licenses](SOURCE_AND_LICENSES.md) for the baseline and component provenance.

Source is under `qmk_firmware/`; build entry points are under `tools/`. Dependencies are ordinary files included in the release. The historical `qmk_firmware/.gitmodules` is retained for provenance: **do not run `git submodule update` to replace the fixed dependencies.** No private dependency repository is required.

## Preserve exact file bytes

The source retains its original `.gitattributes` and `.gitignore`. A normal checkout may transform file bytes according to those attributes. To verify the release manifest, download the release ZIP or run the following in Bash using a new directory:

```bash
git clone --no-checkout --branch p75-jis-0812-r1 https://github.com/kibu-keyboards/qmk_firmware.git p75-jis-0812-r1
cd p75-jis-0812-r1
printf '* -text -eol -filter -ident\n' > .git/info/attributes
git checkout --detach p75-jis-0812-r1
```

These attributes apply only to this clone. No submodule checkout is required. The build workflow also extracts `git archive HEAD` over its checkout to preserve the committed bytes.

[The release manifest](../metadata/release-manifest.csv) records the size and SHA-256 of every release file except itself. The ZIP checksum covers the complete package, including the manifest. Original-input records in `metadata/` describe the supplied baseline; use the release manifest for the current files.
