param(
    [Parameter(Mandatory = $true)][string]$Action,
    [Parameter(Mandatory = $true)][string]$Root,
    [string]$Prefix = "",
    [string]$Version = "",
    [string]$CurrentYear = "",
    [string]$SourceList = "",
    [ValidateSet("x86_64", "arm64")][string]$TargetArch = "x86_64"
)

$script = Join-Path $PSScriptRoot "win-msvc.cmd"
$env:CJIT_PREFIX = $Prefix
$env:CJIT_VERSION = $Version
$env:CJIT_CURRENT_YEAR = $CurrentYear
$env:CJIT_TARGET_ARCH = $TargetArch
& $script $Action $Root $SourceList
exit $LASTEXITCODE
