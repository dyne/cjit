# Instructions to submit a new release to winpkg

Download the latest `CJIT_installer.exe` and note the version.

Run `./build/winpkg-update.sh` with `CJIT_installer.exe` as file and version string.

This will generate the new manifest in `manifest/`.

Then validate the manifest:

```
winget validate --manifest manifests/d/Dyne/CJIT/X.X.X/
```

Then use wingetcreate to submit the update:

```
wingetcreate submit --prtitle "New Version: Dyne.CJIT version X.X.X" manifests/d/Dyne/CJIT/X.X.X/
```
