#!/bin/sh

# this script is needed to inscribe metadata in the .exe files
# targeting windows. it creates a .rs to be used when linking

cat << EOF > cjit.rc
1 VERSIONINFO
FILEVERSION     1,0,0,0
PRODUCTVERSION  1,0,0,0
BEGIN
  BLOCK "StringFileInfo"
  BEGIN
    BLOCK "040904E4"
    BEGIN
      VALUE "CompanyName", "Dyne.org Foundation"
      VALUE "FileDescription", "CJIT, Just in Time C"
      VALUE "FileVersion", "`date +'%Y%m%d'`"
      VALUE "InternalName", "cjit"
      VALUE "LegalCopyright", "Copyright by the Dyne.org Foundation"
      VALUE "OriginalFilename", "cjit.exe"
      VALUE "ProductName", "CJIT"
      VALUE "ProductVersion", "3"
    END
  END
  BLOCK "VarFileInfo"
  BEGIN
    VALUE "Translation", 0x409, 1252
  END
END
EOF

if [ "$(which windres)" = "" ]; then
    x86_64-w64-mingw32-windres cjit.rc -O coff -o cjit.res
else
    windres cjit.rc -O coff -o cjit.res
fi
