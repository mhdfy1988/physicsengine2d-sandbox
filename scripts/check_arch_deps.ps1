$ErrorActionPreference = "Stop"

$root = Split-Path -Parent $PSScriptRoot
$failed = $false

function Assert-NoMatch {
    param(
        [string]$Pattern,
        [string[]]$Globs,
        [string]$Message
    )

    $files = @()
    foreach ($g in $Globs) {
        $files += Get-ChildItem -Path (Join-Path $root $g) -File -Recurse -ErrorAction SilentlyContinue
    }
    foreach ($f in $files) {
        $content = Get-Content -Path $f.FullName -Raw
        if ($content -match $Pattern) {
            Write-Host "[FAIL] ${Message}: $($f.FullName)"
            $script:failed = $true
        }
    }
}

# Public headers must not depend on internal source layout.
Assert-NoMatch -Pattern 'src/internal|physics_internal\.h' -Globs @('include\*.h') -Message 'public header depends on internal file'

# Internal headers must not depend on app layer.
Assert-NoMatch -Pattern 'apps\/|apps\\' -Globs @('src\internal\*.h', 'src\*.h') -Message 'internal header depends on app layer'

if ($failed) {
    Write-Host "Architecture dependency checks failed."
    exit 1
}

Write-Host "Architecture dependency checks passed."
