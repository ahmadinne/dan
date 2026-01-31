#!/usr/bin/env powershell
$curd = $(Get-Location)
$path = "$env:USERPROFILE\Scripts"
$conf = "$env:USERPROFILE\.config\dan"

if (!(Test-Path $path)) {
	mkdir $path
	[System.Environment]::SetEnvironmentVariable("PATH", $env:PATH + ";$path", "User")
	$env:PATH = [System.Environment]::GetEnvironmentVariable("PATH", "User")
}

if (!(Test-Path $conf)) {
	mkdir $conf
}

Copy-Item -Path "$curd\dan.ps1" -Destination "${path}\dan.ps1" -Force
