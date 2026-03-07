$ErrorActionPreference = "Stop"

$root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$failed = $false

$legacyPatterns = @(
    '#include\s+"asset_database\.hpp"',
    '#include\s+"asset_fs_poll\.hpp"',
    '#include\s+"asset_fs_watch\.hpp"',
    '#include\s+"asset_hot_reload\.hpp"',
    '#include\s+"asset_importer\.hpp"',
    '#include\s+"asset_invalidation\.hpp"',
    '#include\s+"asset_pipeline\.hpp"',
    '#include\s+"asset_watch\.hpp"',
    '#include\s+"diagnostic_bundle\.hpp"',
    '#include\s+"editor_plugin\.hpp"',
    '#include\s+"prefab_schema\.hpp"',
    '#include\s+"prefab_semantics\.hpp"',
    '#include\s+"project_workspace\.hpp"',
    '#include\s+"runtime_snapshot_repo\.hpp"',
    '#include\s+"scene_schema\.hpp"',
    '#include\s+"session_recovery\.hpp"',
    '#include\s+"subsystem_render_audio_animation\.hpp"',
    '#include\s+"body\.hpp"',
    '#include\s+"collision\.hpp"',
    '#include\s+"constraint\.hpp"',
    '#include\s+"shape\.hpp"',
    '#include\s+"physics_math\.hpp"'
)

$targets = @('apps', 'src', 'tests', 'examples', 'tools')

foreach ($target in $targets) {
    $files = Get-ChildItem -Path (Join-Path $root $target) -Recurse -File -Include *.cpp,*.hpp,*.h -ErrorAction SilentlyContinue
    foreach ($file in $files) {
        $content = Get-Content -Path $file.FullName -Raw
        foreach ($pattern in $legacyPatterns) {
            if ($content -match $pattern) {
                Write-Host "[FAIL] legacy top-level module include in $($file.FullName): $pattern"
                $failed = $true
            }
        }
    }
}

if ($failed) {
    Write-Host "Canonical include checks failed."
    exit 1
}

Write-Host "Canonical include checks passed."
