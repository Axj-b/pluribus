#requires -Version 7.0
param(
    [string]$RepositoryRoot = (Split-Path -Parent $PSScriptRoot)
)

$ErrorActionPreference = 'Stop'

$docsRoot = Join-Path $RepositoryRoot 'docs'
$files = @()
$files += Get-ChildItem -Path $docsRoot -Recurse -File -Filter '*.md'
foreach ($name in @('README.md', 'AGENTS.md')) {
    $path = Join-Path $RepositoryRoot $name
    if (Test-Path -LiteralPath $path) {
        $files += Get-Item -LiteralPath $path
    }
}
$todoRoot = Join-Path $RepositoryRoot 'todo'
if (Test-Path -LiteralPath $todoRoot) {
    $files += Get-ChildItem -Path $todoRoot -Recurse -File -Filter '*.md'
}

function Get-GitHubAnchors([string]$path) {
    $anchors = New-Object System.Collections.Generic.HashSet[string]
    foreach ($line in [System.IO.File]::ReadAllLines($path)) {
        if ($line -match '^(#{1,6})\s+(.*?)\s*#*\s*$') {
            $text = $matches[2].Trim()
            $text = $text -replace '`', ''
            $text = $text -replace '\[([^\]]+)\]\([^)]+\)', '$1'
            $text = $text.ToLowerInvariant()
            $text = $text -replace '[^\w\s-]', ''
            $text = $text -replace '\s', '-'
            $text = $text.Trim('-')
            if ($text) {
                [void]$anchors.Add($text)
            }
        }
    }
    return $anchors
}

$anchorCache = @{}
foreach ($file in $files) {
    $anchorCache[$file.FullName] = Get-GitHubAnchors $file.FullName
}

$errors = @()
$linkCount = 0
$anchorCount = 0

foreach ($file in $files) {
    $content = [System.IO.File]::ReadAllText($file.FullName)
    foreach ($match in [regex]::Matches($content, '\[[^\]]+\]\(([^)]+)\)')) {
        $target = $match.Groups[1].Value.Trim()
        if ($target -match '^(https?:|mailto:|#)') { continue }
        if ($target -match '^<.*>$') { $target = $target.Trim('<', '>') }

        $parts = $target -split '#', 2
        $pathPart = $parts[0]
        if ([string]::IsNullOrWhiteSpace($pathPart)) { continue }

        $decoded = [System.Uri]::UnescapeDataString($pathPart)
        $resolved = [System.IO.Path]::GetFullPath((Join-Path $file.DirectoryName $decoded))
        $linkCount++
        if (-not (Test-Path -LiteralPath $resolved)) {
            $errors += "$($file.FullName.Substring($RepositoryRoot.Length + 1)): missing link target '$target'"
            continue
        }

        if ($parts.Count -eq 2 -and -not [string]::IsNullOrWhiteSpace($parts[1])) {
            $anchor = $parts[1]
            if (-not $anchorCache.ContainsKey($resolved)) {
                $anchorCache[$resolved] = Get-GitHubAnchors $resolved
            }
            $anchorCount++
            if (-not $anchorCache[$resolved].Contains($anchor)) {
                $errors += "$($file.FullName.Substring($RepositoryRoot.Length + 1)): missing anchor '$target'"
            }
        }
    }
}

if ($errors.Count -gt 0) {
    $errors | ForEach-Object { Write-Host $_ }
    Write-Host "$($errors.Count) documentation link/anchor error(s)."
    exit 1
}

Write-Host "Checked $($files.Count) Markdown files: $linkCount relative links and $anchorCount local anchors resolve."
