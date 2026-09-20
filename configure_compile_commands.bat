@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\18\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
cd /d "D:\AI\Projects\Chess"
cmake -S . -B build -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
if %errorlevel% neq 0 exit /b %errorlevel%
copy build\compile_commands.json compile_commands.json
echo compile_commands.json copied to project root
