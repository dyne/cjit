#!/usr/bin/env bash

# Load the Visual Studio command-line environment into the current bash shell.

find_vsdevcmd_path() {
  local root
  local path

  for root in "/c/Program Files/Microsoft Visual Studio" \
    "/c/Program Files (x86)/Microsoft Visual Studio"; do
    [ -d "$root" ] || continue
    for path in "$root"/*/*/Common7/Tools/VsDevCmd.bat; do
      [ -r "$path" ] || continue
      printf '%s\n' "$path"
      return 0
    done
  done

  return 1
}

if [ "${CJIT_VSDEVCMD_LOADED:-}" = "1" ]; then
  return 0
fi

if ! command -v cygpath >/dev/null; then
  echo "cygpath is required to load the Visual Studio environment" >&2
  return 1
fi

VSDEVCMD_PATH="$(find_vsdevcmd_path)" || {
  echo "VsDevCmd.bat not found" >&2
  return 1
}
VSDEVCMD_WIN="$(cygpath -w "$VSDEVCMD_PATH")"
export VSDEVCMD_PATH VSDEVCMD_WIN

tmp="$(mktemp --suffix=.cmd)"
cat <<EOF > "$tmp"
@echo off
set "PATH=%SystemRoot%\System32;%SystemRoot%;%SystemRoot%\System32\Wbem;%SystemRoot%\System32\WindowsPowerShell\v1.0\;%PATH%"
call "$VSDEVCMD_WIN" -arch=x64 -host_arch=x64 >nul
set
EOF

env_dump="$(/c/Windows/System32/cmd.exe //c "$(cygpath -w "$tmp")" | tr -d '\r')"
rm -f "$tmp"

path_value="$(printf '%s\n' "$env_dump" | sed -n 's/^PATH=//p' | head -n 1)"
[ -n "$path_value" ] && export PATH="$(cygpath -up "$path_value")"

while IFS='=' read -r key value; do
  case "$key" in
    INCLUDE|LIB|LIBPATH|UCRTVersion|UniversalCRTSdkDir|VCToolsInstallDir|VCToolsRedistDir|VCINSTALLDIR|VSINSTALLDIR|VisualStudioVersion|WindowsLibPath|WindowsSdkBinPath|WindowsSdkDir|WindowsSdkVerBinPath|WindowsSDKLibVersion|WindowsSDKVersion|ExtensionSdkDir|DevEnvDir|Framework*|Platform)
      export "$key=$value"
      ;;
  esac
done <<EOF
$env_dump
EOF

export CJIT_VSDEVCMD_LOADED=1
