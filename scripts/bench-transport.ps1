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
    "cmake --preset wsl-transport-bench && " +
    "cmake --build --preset wsl-transport-bench --target pluribus_transport_bench && " +
    "./build/wsl-transport-bench/benchmarks/transport/pluribus_transport_bench"

& wsl -e sh -lc $command
exit $LASTEXITCODE
