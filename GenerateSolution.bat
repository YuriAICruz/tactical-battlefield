@echo off
rem Generates BattlefieldCpp.sln (Visual Studio 2022 / Rider) into .\vs2022
cd /d "%~dp0"
cmake -S . -B vs2022 -G "Visual Studio 17 2022" -A x64 || exit /b 1
echo.
echo Open vs2022\BattlefieldCpp.sln  (startup project: BattlefieldTestRunner)
