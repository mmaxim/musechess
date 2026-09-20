@echo off
setlocal
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat" || exit /b 1
set "PATH=C:\Program Files\CMake\bin;%PATH%"
cd /d "%~dp0" || exit /b 1
echo === CMAKE CONFIGURE ===
cmake -S . -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON || exit /b 1
echo === CMAKE BUILD ===
cmake --build build || exit /b 1
echo === COPY COMPILE COMMANDS ===
if exist build\compile_commands.json (
    copy build\compile_commands.json compile_commands.json >nul
    echo compile_commands.json updated
)
echo === BUILD OK ===
