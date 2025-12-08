@echo off
setlocal

echo ==============================
echo   Cleaning previous Qt build...
echo ==============================

if exist build_qt rmdir /s /q build_qt
mkdir build_qt

echo ==============================
echo   Configuring Qt build...
echo ==============================

cmake -B build_qt -S . ^
 -DCMAKE_TOOLCHAIN_FILE=C:/Code/vcpkg/scripts/buildsystems/vcpkg.cmake ^
 -DQt6_DIR=C:/Qt/6.10.1/mingw_64/lib/cmake/Qt6 ^
 -DVCPKG_TARGET_TRIPLET=x64-windows ^
 -DCMAKE_BUILD_TYPE=Release ^
 -DBUILD_DESKTOP_UI=ON ^
 -DBUILD_DJ_UI=ON ^
 -DBUILD_VIDEO_EDITOR_UI=ON

if errorlevel 1 (
    echo.
    echo ERROR: CMake configuration for Qt build failed.
    pause
    exit /b 1
)

echo ==============================
echo   Building Qt frontends...
echo ==============================

REM Build all Qt targets (AerialPlayer, AerialDJ, AerialVideoEditor)
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

REM Desktop player
if exist build_qt\Release\AerialPlayer.exe (
    copy /y build_qt\Release\AerialPlayer.exe dist_qt\AerialPlayer.exe >nul
    echo Deployed desktop player to dist_qt\AerialPlayer.exe
) else (
    echo WARNING: build_qt\Release\AerialPlayer.exe not found!
)

REM DJ UI
if exist build_qt\Release\AerialDJ.exe (
    copy /y build_qt\Release\AerialDJ.exe dist_qt\AerialDJ.exe >nul
    echo Deployed DJ UI to dist_qt\AerialDJ.exe
) else (
    echo WARNING: build_qt\Release\AerialDJ.exe not found!
)

REM (Optional) Video editor
if exist build_qt\Release\AerialVideoEditor.exe (
    copy /y build_qt\Release\AerialVideoEditor.exe dist_qt\AerialVideoEditor.exe >nul
    echo Deployed video editor to dist_qt\AerialVideoEditor.exe
)

echo.
echo *********************************************
echo   QT BUILD COMPLETE
echo   Desktop: dist_qt\AerialPlayer.exe
echo   DJ:      dist_qt\AerialDJ.exe
echo   Editor:  dist_qt\AerialVideoEditor.exe (if built)
echo   (Qt DLLs must be on PATH or beside the exe)
echo *********************************************

endlocal
pause
