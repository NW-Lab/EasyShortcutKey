<#
 build_package.ps1
 KeyboardGW (Windows版) 配布 ZIP 作成スクリプト
 出力: dist/KeyboardGW-Win-x64.zip
#>

param(
    [string]$Configuration = "Release",
    [string]$Runtime = "win-x64",
    [switch]$SelfContained = $true
)

$ErrorActionPreference = 'Stop'

$projectPath = Join-Path $PSScriptRoot 'ResidentWin/ResidentWin.csproj'
$distDir = Join-Path $PSScriptRoot 'dist'
$publishDir = Join-Path $distDir 'publish'
$zipPath = Join-Path $distDir 'KeyboardGW-Win-x64.zip'

if (Test-Path $distDir) { Remove-Item $distDir -Recurse -Force }
New-Item -ItemType Directory -Path $publishDir | Out-Null

Write-Host "[0/3] Ensure icon" -ForegroundColor Cyan
$iconPath = Join-Path $PSScriptRoot 'ResidentWin/Resources/AppIcon.ico'
if (!(Test-Path $iconPath)) {
    $iconScript = Join-Path $PSScriptRoot 'tools/make_icon.ps1'
    if (Test-Path $iconScript) {
        Write-Host "  Generating AppIcon.ico" -ForegroundColor Yellow
        & $iconScript | Write-Host
    } else {
        Write-Host "  Icon script not found, skipping icon generation" -ForegroundColor Yellow
    }
} else {
    Write-Host "  Icon already exists" -ForegroundColor Green
}

Write-Host "[1/3] Publish" -ForegroundColor Cyan
$publishArgs = @(
    'publish', $projectPath,
    '-c', $Configuration,
    '-r', $Runtime,
    '--self-contained', ($SelfContained.IsPresent -or $SelfContained) ,
    '/p:PublishSingleFile=true',
    '/p:IncludeNativeLibrariesForSelfExtract=true',
    '/p:IncludeAllContentForSelfExtract=true',
    '--output', $publishDir
)

dotnet @publishArgs

Write-Host "[2/3] Clean unnecessary files" -ForegroundColor Cyan
# Remove pdb/xml if not needed for distribution
Get-ChildItem $publishDir -Include *.pdb,*.xml -File | ForEach-Object { Remove-Item $_.FullName -Force }

Write-Host "[3/3] Create zip" -ForegroundColor Cyan
if (Test-Path $zipPath) { Remove-Item $zipPath -Force }
Compress-Archive -Path (Join-Path $publishDir '*') -DestinationPath $zipPath -Force

Write-Host "Done: $zipPath" -ForegroundColor Green
