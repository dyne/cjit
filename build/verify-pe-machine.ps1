# SPDX-License-Identifier: GPL-3.0-or-later
param(
    [Parameter(Mandatory = $true)][string]$Path,
    [Parameter(Mandatory = $true)][ValidateSet("x86_64", "arm64")][string]$Architecture
)

$expected = @{
    x86_64 = 0x8664
    arm64 = 0xAA64
}[$Architecture]

$stream = [System.IO.File]::OpenRead($Path)
$reader = [System.IO.BinaryReader]::new($stream)
try {
    if ($stream.Length -lt 64 -or $reader.ReadUInt16() -ne 0x5A4D) {
        throw "$Path is not a PE executable"
    }
    $stream.Position = 0x3C
    $peOffset = $reader.ReadUInt32()
    if ($peOffset -gt $stream.Length - 6) {
        throw "$Path has an invalid PE header offset"
    }
    $stream.Position = $peOffset
    if ($reader.ReadUInt32() -ne 0x00004550) {
        throw "$Path has no PE signature"
    }
    $actual = $reader.ReadUInt16()
    if ($actual -ne $expected) {
        throw ('{0} machine is 0x{1:X4}, expected {2} (0x{3:X4})' -f $Path, $actual, $Architecture, $expected)
    }
} finally {
    $reader.Dispose()
    $stream.Dispose()
}

Write-Output ('PE_MACHINE path={0} architecture={1} machine=0x{2:X4}' -f $Path, $Architecture, $expected)
