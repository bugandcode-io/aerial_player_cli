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
 -DCMAKE_TOOLCHAIN_FILE=D:/Code/vcpkg/scripts/buildsystems/vcpkg.cmake ^
 -DVCPKG_TARGET_TRIPLET=x64-windows ^
 -DCMAKE_BUILD_TYPE=Release ^
 -DBUILD_QT_UI=ON

if errorlevel 1 (
    echo.
    echo ERROR: CMake configuration for Qt build failed.
    pause
    exit /b 1
)

echo ==============================
echo   Building AerialDJ (Qt UI)...
echo ==============================

cmake --build build_qt --config Release --target AerialDJ

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

if exist build_qt\Release\AerialDJ.exe (
    copy /y build_qt\Release\AerialDJ.exe dist_qt\AerialDJ.exe >nul
    echo Deployed Qt UI to dist_qt\AerialDJ.exe
) else (
    echo WARNING: build_qt\Release\AerialDJ.exe not found!
)

echo.
echo *********************************************
echo   QT BUILD COMPLETE
echo   Your Qt exe is in: dist_qt\AerialDJ.exe
echo   (Qt DLLs must be on PATH or beside the exe)
echo *********************************************

endlocal
pause
