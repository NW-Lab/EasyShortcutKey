<#
build_embed.ps1
PowerShell script to embed browser/shortcuts.json into browser/index.html and write browser/index.built.html
#>
param(
    [string]$RootDir = (Split-Path -Parent $PSScriptRoot),
    [string]$Template = "$PSScriptRoot/../browser/index.html",
    [string]$JsonFile = "$PSScriptRoot/../browser/shortcuts.json",
    [string]$OutFile = "$PSScriptRoot/../browser/index.built.html"
)

if (-not (Test-Path $Template)) {
    Write-Error "Template $Template not found"
    exit 1
}
if (-not (Test-Path $JsonFile)) {
    Write-Error "JSON $JsonFile not found"
    exit 1
}

$tpl = Get-Content -Raw -Path $Template
$json = Get-Content -Raw -Path $JsonFile

# Replace the content between <script id="default-config" ...> and </script>
$pattern = '(?s)(<script[^>]*id=["'']default-config["''][^>]*type=["'']application\/json["''][^>]*>).*?(</script>)'
$replacement = '${1}'+[Regex]::Escape($json)+'${2}'

# But we don't want to escape JSON for HTML; keep raw. Use a regex evaluator callback instead
$replaced = [Regex]::Replace($tpl, '(?s)(<script[^>]*id=["\'']default-config["\''][^>]*type=["\'']application\/json["\''][^>]*>).*?(</script>)', { param($m) return $m.Groups[1].Value + "`n" + $json + "`n" + $m.Groups[2].Value })

Set-Content -Path $OutFile -Value $replaced -Encoding UTF8
Write-Host "Generated: $OutFile"
Start-Process "$OutFile"
