#requires -Version 5.1

$ErrorActionPreference = 'Stop'

$repositoryRoot = Split-Path -Parent $PSScriptRoot
$scanRoots = @(
    (Join-Path $repositoryRoot 'core'),
    (Join-Path $repositoryRoot 'master\scheduler'),
    (Join-Path $repositoryRoot 'fabric\include')
)

$forbidden = @(
    'windows.h', 'winsock2.h', 'ws2tcpip.h', 'mswsock.h', 'afunix.h',
    'unistd.h', 'pthread.h', 'dlfcn.h', 'sys/', 'arpa/', 'netinet/',
    'mach/', 'TargetConditionals.h', 'crtdbg.h', 'direct.h', 'io.h', 'process.h'
)

$errors = @()
foreach ($root in $scanRoots) {
    if (-not (Test-Path -LiteralPath $root)) { continue }
    $files = Get-ChildItem -Path $root -Recurse -File |
        Where-Object { $_.Extension -in '.h', '.hpp', '.c', '.cc', '.cpp', '.cxx' }
    foreach ($file in $files) {
        $lineNumber = 0
        foreach ($line in [System.IO.File]::ReadAllLines($file.FullName)) {
            $lineNumber++
            if ($line -match '^\s*#\s*include\s*<([^>]+)>') {
                $header = $matches[1].ToLowerInvariant()
                foreach ($name in $forbidden) {
                    if ($header -like "$name*" -or $header -like "*/$name*") {
                        $relative = $file.FullName.Substring($repositoryRoot.Length + 1)
                        $errors += "$($relative):$($lineNumber): forbidden include <$header>"
                    }
                }
            }
        }
    }
}

if ($errors.Count -gt 0) {
    $errors | ForEach-Object { Write-Host $_ }
    Write-Host "$($errors.Count) forbidden include(s)."
    exit 1
}

Write-Host "Dependency include scan passed."
