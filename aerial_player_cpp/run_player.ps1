# ------------------------------------------
#  Qt6 from vcpkg (x64-windows triplet)
# ------------------------------------------
$qtRoot = "D:\Code\vcpkg\installed\x64-windows"

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
#  AerialPlayer.exe in dist_qt
# ---------------------------------------
$exe = "D:\Code\aerial_player\cli\aerial_player_cpp\dist_qt\AerialPlayer.exe"

if (!(Test-Path $exe)) {
    Write-Host "ERROR: AerialPlayer.exe not found at $exe" -ForegroundColor Red
    exit 1
}

Write-Host "Launching: $exe" -ForegroundColor Green

Start-Process -FilePath $exe -WorkingDirectory (Split-Path $exe)
