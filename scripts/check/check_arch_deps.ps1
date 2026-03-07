$ErrorActionPreference = "Stop"

$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
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

# Runtime source layers must not depend on app/editor layer.
Assert-NoMatch -Pattern 'apps\/|apps\\' -Globs @('src\c_api\*.cpp', 'src\core\*.cpp', 'src\physics2d\*.cpp') -Message 'runtime layer depends on app layer'

# Physics2D implementation must not include c_api implementation headers/paths.
Assert-NoMatch -Pattern 'c_api\/|c_api\\' -Globs @('src\physics2d\*.cpp') -Message 'physics2d depends on c_api layer'

# Pure physics kernel must not depend on content/editor systems.
Assert-NoMatch -Pattern 'src\/content|src\\content|prefab_|asset_|project_workspace|editor_plugin|session_recovery|diagnostic_bundle' -Globs @('src\c_api\*.cpp', 'src\core\*.cpp', 'src\physics2d\*.cpp') -Message 'physics kernel depends on content/editor layer'

# Public physics headers must remain content/editor free.
Assert-NoMatch -Pattern 'prefab_|asset_|project_workspace|editor_plugin|session_recovery|diagnostic_bundle|content\/|content\\' -Globs @('include\physics2d\*.hpp', 'include\physics.h') -Message 'public physics api depends on content/editor layer'

# Runtime support must not depend on app/editor layer.
Assert-NoMatch -Pattern 'apps\/|apps\\|editor_plugin|project_workspace|prefab_|asset_|session_recovery|diagnostic_bundle' -Globs @('src\runtime\*.cpp') -Message 'runtime support depends on app/content-editor layer'

if ($failed) {
    Write-Host "Architecture dependency checks failed."
    exit 1
}

Write-Host "Architecture dependency checks passed."
