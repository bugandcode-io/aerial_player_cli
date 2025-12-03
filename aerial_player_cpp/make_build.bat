cd D:\Code\aerial_player\cli\aerial_player_cpp
rmdir /s /q build
mkdir build
cd build

cmake .. -DCMAKE_TOOLCHAIN_FILE=D:/Code/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows
cmake --build . --config Release
