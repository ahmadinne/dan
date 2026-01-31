#!/usr/bin/env powershell
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

if (!(Copy-Item -Path "dan.ps1" -Destination "${path}\dan.ps1" -Force)) {
	Copy-Item -Path "$env:USERPROFILE\Documents\dan\dan.ps1" -Destination "$path\dan.ps1" -Force
}
