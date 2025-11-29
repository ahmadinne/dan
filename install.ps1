#!/usr/bin/env powershell
$path = "$env:USERPROFILE\Scripts"
mkdir $path
[System.Environment]::SetEnvironmentVariable("PATH", $env:PATH + ";$path", "User")
$env:PATH = [System.Environment]::GetEnvironmentVariable("PATH", "User")

Copy-Item dan.ps1 $path
