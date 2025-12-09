@echo off
setlocal

cd /d D:\Code\aerial_player\cli\aerial_player_cpp

echo ==============================
echo   Cleaning previous Qt build...
echo ==============================

if exist build_qt rmdir /s /q build_qt
mkdir build_qt

echo ==============================
echo   Configuring Qt build...
echo ==============================

cmake -B build_qt -S . ^
 -DCMAKE_TOOLCHAIN_FILE=D:/Code/vcpkg/scripts/buildsystems/vcpkg.cmake ^
 -DVCPKG_TARGET_TRIPLET=x64-windows ^
 -DCMAKE_BUILD_TYPE=Release ^
 -DBUILD_DESKTOP_UI=ON ^
 -DBUILD_DJ_UI=ON ^
 -DBUILD_VIDEO_EDITOR_UI=OFF

if errorlevel 1 (
    echo.
    echo ERROR: CMake configuration for Qt build failed.
    pause
    exit /b 1
)

echo ==============================
echo   Building Qt frontends...
echo ==============================

cmake --build build_qt --config Release

if errorlevel 1 (
    echo.
    echo ERROR: Qt build failed.
    pause
    exit /b 1
)

echo ==============================
echo   Preparing Qt dist output...
echo ==============================

if not exist dist_qt mkdir dist_qt

REM ----- Desktop player -----
set "PLAYER_SRC=build_qt\src\ui_desktop\Release\AerialPlayer.exe"
if exist "%PLAYER_SRC%" (
    copy /y "%PLAYER_SRC%" dist_qt\AerialPlayer.exe >nul
    echo Deployed desktop player to dist_qt\AerialPlayer.exe
) else (
    echo WARNING: %PLAYER_SRC% not found!
)

REM ----- DJ UI -----
set "DJ_SRC=build_qt\src\ui_dj\Release\AerialDJ.exe"
if exist "%DJ_SRC%" (
    copy /y "%DJ_SRC%" dist_qt\AerialDJ.exe >nul
    echo Deployed DJ UI to dist_qt\AerialDJ.exe
) else (
    echo WARNING: %DJ_SRC% not found!
)

REM ----- (Optional) CLI -----
set "CLI_SRC=build_qt\Release\aerial.exe"
if exist "%CLI_SRC%" (
    copy /y "%CLI_SRC%" dist_qt\aerial_cli.exe >nul
    echo Deployed CLI to dist_qt\aerial_cli.exe
) else (
    echo WARNING: %CLI_SRC% not found!
)

echo.
echo *********************************************
echo   QT BUILD COMPLETE
echo   Desktop: dist_qt\AerialPlayer.exe
echo   DJ:      dist_qt\AerialDJ.exe
echo   CLI:     dist_qt\aerial_cli.exe
echo   (Qt DLLs must be on PATH or beside the exe)
echo *********************************************

endlocal
pause
