#requires -Version 5.1

$ErrorActionPreference = 'Stop'

$repositoryRoot = Split-Path -Parent $PSScriptRoot
if (-not (Get-Command wsl -ErrorAction SilentlyContinue)) {
    Write-Host 'WSL is not available on this host.'
    exit 1
}

$wslPath = (& wsl -e wslpath -a $repositoryRoot).Trim()
if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($wslPath)) {
    Write-Host 'Could not translate the repository path for WSL.'
    exit 1
}

$command = "cd '$wslPath' && " +
    "cmake --preset wsl-codec-bench && " +
    "cmake --build --preset wsl-codec-bench --target pluribus_codec_bench && " +
    "./build/wsl-codec-bench/benchmarks/codec/pluribus_codec_bench"

& wsl -e sh -lc $command
exit $LASTEXITCODE
