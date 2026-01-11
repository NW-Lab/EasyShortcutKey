<#
 make_icon.ps1
 指定PNG (デフォルト: iOS資産 EasyShortcutKey.png) から ICO (複数サイズ) を生成。
 出力: ../ResidentWin/Resources/AppIcon.ico

 使い方:
   cd ResidentWin/tools
   ./make_icon.ps1               # 既定パスから生成
   ./make_icon.ps1 -Source ..\iOS\EasyShortcutKey\Assets.xcassets\AppIcon.appiconset\EasyShortcutKey.png

 必要: .NET / System.Drawing.Common
#>
param(
  [string]$Source = "..\iOS\EasyShortcutKey\Assets.xcassets\AppIcon.appiconset\EasyShortcutKey.png",
  [string]$Out    = "..\ResidentWin\Resources\AppIcon.ico",
  [int[]]$Sizes   = @(16,32,48,64,128,256)
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'

if (-not (Test-Path $Source)) { throw "Source PNG not found: $Source" }

# Resolve output path if possible (PowerShell 5 互換)
$resolvedOut = $null
try { $resolvedOut = Resolve-Path -LiteralPath $Out -ErrorAction Stop } catch { }
if ($resolvedOut) { $Out = $resolvedOut.Path }
$outDir = Split-Path -Parent $Out
if (-not (Test-Path $outDir)) { New-Item -ItemType Directory -Path $outDir | Out-Null }

Add-Type -AssemblyName System.Drawing

function New-IconFromPng {
  param([string]$pngPath,[string]$icoPath,[int[]]$iconSizes)
  $bmpList = @()
  try {
    $original = [System.Drawing.Image]::FromFile($pngPath)
    foreach ($s in $iconSizes) {
      $bmp = New-Object System.Drawing.Bitmap $s,$s
      $g = [System.Drawing.Graphics]::FromImage($bmp)
      $g.Clear([System.Drawing.Color]::Transparent)
      $g.InterpolationMode = 'HighQualityBicubic'
      $g.DrawImage($original,0,0,$s,$s)
      $g.Dispose()
      $bmpList += $bmp
    }

    $fs = [IO.File]::Create($icoPath)
    $bw = New-Object IO.BinaryWriter($fs)
    $bw.Write([UInt16]0)
    $bw.Write([UInt16]1)
    $bw.Write([UInt16]$bmpList.Count)

    $imageDataList = @()
    foreach ($bmp in $bmpList) {
      $ms = New-Object IO.MemoryStream
      $bmp.Save($ms, [System.Drawing.Imaging.ImageFormat]::Png)
      $bytes = $ms.ToArray()
      $imageDataList += ,(@{ Size=$bmp.Width; Bytes=$bytes })
      $ms.Dispose()
    }

    $offset = 6 + (16 * $imageDataList.Count)
    foreach ($img in $imageDataList) {
      $w = $img.Size; $h = $img.Size
      $wByte = if ($w -ge 256) { 0 } else { $w }
      $hByte = if ($h -ge 256) { 0 } else { $h }
      $bw.Write([byte]$wByte)
      $bw.Write([byte]$hByte)
      $bw.Write([byte]0)          # Color count (0 if >=8bpp)
      $bw.Write([byte]0)          # Reserved
      $bw.Write([UInt16]1)        # Planes
      $bw.Write([UInt16]32)       # Bit count
      $bw.Write([UInt32]$img.Bytes.Length) # Size of image data
      $bw.Write([UInt32]$offset)  # Offset
      $offset += $img.Bytes.Length
    }

    foreach ($img in $imageDataList) { $bw.Write($img.Bytes) }
    $bw.Flush(); $bw.Dispose(); $fs.Dispose()
  }
  finally { foreach ($b in $bmpList) { $b.Dispose() } }
}

Write-Host "Generating icon from $Source -> $Out" -ForegroundColor Cyan
New-IconFromPng -pngPath $Source -icoPath $Out -iconSizes $Sizes
Write-Host "Done: $Out" -ForegroundColor Green
