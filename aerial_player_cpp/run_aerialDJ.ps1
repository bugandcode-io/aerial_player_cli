# ------------------------------------------
#  Auto-detect Qt root installed by vcpkg
# ------------------------------------------
$qtRoot = "D:\Code\vcpkg\installed\x64-windows\Qt6"

if (!(Test-Path $qtRoot)) {
    Write-Host "ERROR: Qt root not found at $qtRoot" -ForegroundColor Red
    exit 1
}

# -----------------------------
#  Update environment variables
# -----------------------------
$env:PATH = "$qtRoot\bin;$env:PATH"
$env:QT_QPA_PLATFORM_PLUGIN_PATH = "$qtRoot\plugins\platforms"

# ---------------------------------------
#  Auto-detect AerialDJ.exe location
# ---------------------------------------
$exe = Get-ChildItem -Path . -Recurse -Filter "AerialDJ.exe" `
       | Sort-Object FullName `
       | Select-Object -First 1

if (-not $exe) {
    Write-Host "ERROR: AerialDJ.exe not found in repo. Did you build Qt UI?" -ForegroundColor Red
    exit 1
}

Write-Host "Launching: $($exe.FullName)" -ForegroundColor Green

# -----------------------
#  Start the Qt UI app
# -----------------------
Start-Process -FilePath $exe.FullName
