#!/usr/bin/env powershell
$path = "$env:USERPROFILE\Scripts"
if (!(Test-Path $path)) {
	mkdir $path
}
[System.Environment]::SetEnvironmentVariable("PATH", $env:PATH + ";$path", "User")
$env:PATH = [System.Environment]::GetEnvironmentVariable("PATH", "User")

Copy-Item -Path "dan.ps1" -Destination "${path}\dan.ps1" -Force
