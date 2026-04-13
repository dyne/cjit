param(
    [Parameter(Mandatory = $true)][string]$Action,
    [Parameter(Mandatory = $true)][string]$Root,
    [string]$Prefix = "",
    [string]$Version = "",
    [string]$CurrentYear = "",
    [string]$SourceList = ""
)

$script = Join-Path $PSScriptRoot "win-msvc.cmd"
$env:CJIT_PREFIX = $Prefix
$env:CJIT_VERSION = $Version
$env:CJIT_CURRENT_YEAR = $CurrentYear
& $script $Action $Root $SourceList
exit $LASTEXITCODE
