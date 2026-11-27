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
    "cmake --preset wsl-gcc-debug && " +
    "cmake --build --preset wsl-gcc-debug && " +
    "ctest --preset wsl-gcc-debug --output-on-failure"

& wsl -e sh -lc $command
exit $LASTEXITCODE
