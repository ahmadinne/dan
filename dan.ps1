#! /usr/bin/env powershell

# ,-----------,
# | Variables |
# '-----------'
$curdir = (gl).path
$option = $args[0]
$choice = $args[1..($args.Length - 1)] -join ' '
$cfgdir = "~\.config\dan"
$config = "${cfgdir}\config"
$lokasi = (gc $config | sls "path" | foreach { ($_ -split '\s+')[2] })
$dancfg = "${lokasi}\.dan"

$pkglist = (gc $dancfg | foreach { ($_ -split '\s+')[0] })
$totals = 0
$number = 1

# ,-----------,
# | Functions |
# '-----------'
function help() {
	Write-Output "
DAN
a tiny and simple Dotfile mANager.

usage: dan <operation> [...]
operations:
	dan help			Show the help page, list of operations and batch operations.
	dan list			Show existing folder or files inside dotfiles.
	dan init .			Initialize current directory as dotfiles directory.
	dan sync [package(s)]		Add specified folder or file into dotfiles.
	dan apply [package(s)]		Apply specified folder or file from dotfiles into local.
	dan remove [package(s)]		Remove specified folder or file from dotfiles.

batch operations:
	dan sync			Sync all folder or files inside dotfiles with local.
	dan apply			Apply all folder or files inside dotfiles into local.

use 'dan help' to show this page."
}

function list() {
	$JARAK=30; $BLUE="\033[1;34m"; $BLACK="\033[0;30m"; $NONE="\033[0m"
	$username = ($env:USERNAME)
	$dotfilepath = ($lokasi -replace "/home/${username}", "~")
	$dotfilepath = ($lokasi -replace "/c/Users/${username}", "~")
}

function init() {
	if ( (Test-Path "$config") -and ( (Get-Item "$config").length -eq 0) ) {
		"There are no path for your dotfiles."
		"Please do 'dan init' inside the directory"
		"to set it as the path"
		""
	} else {
		"There are already path set for your dotfiles,"
		"are you sure to overwrite it?"
		$answer = Read-Host "[y/N]"
		if ($answer -eq "Y" -or $answer -eq "y") {
			$curdir -replace "/c", ""
			New-Item ${curdir}/.dan -ItemType File | Out-Null
			"path = ${curdir}" > $config
		}
	}
}



# ,---------,
# | Run it! |
# '---------'
if ([string]::IsNullOrEmpty($option)) {
	"Dan - Dotfile mANager"
	""
	"Your dotfile path:"
	"$lokasi"
	""
} else {
	switch ($option) {
		"help" { help }
		"list" { list }
		"init" { init }
		"sync" { sync }
		"apply" { apply }
		"remove" { remove }
		Default { "wrong options (use 'dan help' for options)" }
	}
}
