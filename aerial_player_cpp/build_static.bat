@echo off
setlocal

echo ==============================
echo  Cleaning previous build...
echo ==============================

if exist build rmdir /s /q build
mkdir build

echo ==============================
echo  Configuring static build...
echo ==============================

cmake -B build -S . ^
 -DCMAKE_TOOLCHAIN_FILE=D:/Code/vcpkg/scripts/buildsystems/vcpkg.cmake ^
 -DVCPKG_TARGET_TRIPLET=x64-windows-static ^
 -DCMAKE_BUILD_TYPE=Release ^
 -DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded

if errorlevel 1 (
    echo ERROR: CMake configuration failed.
    pause
    exit /b 1
)

echo ==============================
echo  Building Aerial (static)...
echo ==============================

cmake --build build --config Release --target aerial

if errorlevel 1 (
    echo ERROR: Build failed.
    pause
    exit /b 1
)

echo ==============================
echo  Preparing single-exe output...
echo ==============================

if exist dist rmdir /s /q dist
mkdir dist

copy build\Release\aerial.exe dist\

echo.
echo *********************************************
echo   STATIC BUILD COMPLETE
echo   Your standalone exe is in: dist\aerial.exe
echo *********************************************

endlocal
pause
