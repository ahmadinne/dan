#!/usr/bin/env powershell
Copy-Item dan.ps1 ~/scoop/shims/dan.ps1

# '
# @echo off
# REM wrapper that runs the .ps1 file located next to this .cmd
# setlocal
#
# REM path to this wrapper directory
# set "WRAPPER_DIR=%~dp0"
#
# REM call PowerShell (prefer pwsh if installed)
# where pwsh >nul 2>&1
# if %ERRORLEVEL%==0 (
#   set "PS_EXE=pwsh"
# ) else (
#   set "PS_EXE=powershell.exe"
# )
#
# "%PS_EXE%" -NoProfile -ExecutionPolicy Bypass -File "%WRAPPER_DIR%dan.ps1" %*
# exit /b %ERRORLEVEL%
# ' > ~/scoop/shims/dan.bat
