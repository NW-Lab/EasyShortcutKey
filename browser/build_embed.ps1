<#
browser/build_embed.ps1
Same as scripts/build_embed.ps1 but placed inside browser/ for convenience.
#>
param(
    [string]$Template = "$PSScriptRoot/index.html",
    [string]$JsonFile = "$PSScriptRoot/shortcuts.json",
    [string]$OutFile = "$PSScriptRoot/index.built.html"
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

$replaced = [Regex]::Replace($tpl, '(?s)(<script[^>]*id=["\'']default-config["\''][^>]*type=["\'']application\/json["\''][^>]*>).*?(</script>)', { param($m) return $m.Groups[1].Value + "`n" + $json + "`n" + $m.Groups[2].Value })

Set-Content -Path $OutFile -Value $replaced -Encoding UTF8
Write-Host "Generated: $OutFile"
Start-Process "$OutFile"
