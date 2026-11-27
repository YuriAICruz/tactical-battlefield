@echo off
rem Generates the Visual Studio 2022 solution (also usable from Rider) and optionally builds / installs.
rem   Generate.bat                   -> vs2022\BattlefieldCpp.sln only
rem   Generate.bat build [Config]    -> also builds (default Debug)
rem   Generate.bat run [Config]      -> build, then run the test suite
rem   Generate.bat install [Config]  -> build, then install the DLL package into .\dist (used by ..\BattlefieldGame)
rem   Generate.bat -h                -> show this help
setlocal
cd /d "%~dp0"
if /i "%~1"=="-h" goto :help
if /i "%~1"=="--help" goto :help
if "%~1"=="/?" goto :help
set ACTION=%1
set CONFIG=%2
if "%CONFIG%"=="" set CONFIG=Debug

cmake -S . -B vs2022 -G "Visual Studio 17 2022" -A x64 || exit /b 1
echo.
echo Solution: %~dp0vs2022\BattlefieldCpp.sln  (startup project: BattlefieldTestRunner)

if "%ACTION%"=="" exit /b 0
cmake --build vs2022 --config %CONFIG% -- /m /nologo /v:m || exit /b 1
if /i "%ACTION%"=="run" vs2022\bin\%CONFIG%\BattlefieldTestRunner.exe --quiet || exit /b 1
if /i "%ACTION%"=="install" cmake --install vs2022 --config %CONFIG% --prefix dist || exit /b 1
exit /b 0

:help
for /f "usebackq tokens=1*" %%a in (`findstr /b /c:"rem " "%~f0"`) do echo %%b
exit /b 0
