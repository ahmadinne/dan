#! \usr\bin\env powershell

# ,-----------,
# | Variables |
# '-----------'
$curdir = (gl).path
$option = $args[0]
if ($args.Length -gt 1) { $choice = $args[1..($args.Length - 1)] -join " " } else { $choice = "" }
$cfgdir = "~\.config\dan"
$config = "${cfgdir}\config"
$lokasi = (gc $config | sls "path" | foreach { ($_ -split '\s+')[2] })
$dancfg = "${lokasi}\.dan"

$pkglist = gc $dancfg | foreach { ($_ -split '\s+')[0] } | sort
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
	$username = ($env:USERNAME)
	$dotfilepath = ($lokasi -replace "C:\\Users\\${username}", "~")
	Write-Host "-- " -ForegroundColor black -NoNewLine
	Write-Host "[" -nonewline
	Write-Host "Dan" -ForegroundColor blue -nonewline
	Write-Host "]" -nonewline
	Write-Host " $dotfilepath"
	foreach ($list in $pkglist) {
		if (Test-Path "${lokasi}\${list}" -PathType Leaf) {
			Write-Host "-- " -ForegroundColor black -nonewline
			Write-Host " " -nonewline
			Write-Host "$list"
		}
		if (Test-Path "${lokasi}\${list}" -PathType Container) {
			Write-Host "-- " -ForegroundColor black -nonewline
			Write-Host " " -nonewline -ForegroundColor blue
			Write-Host "$list" -ForegroundColor blue
		}
	}
}

function init() {
	if ( (Test-Path "$config") -and ( (Get-Item "$config").length -eq 0) ) {
		"Set current directory as the dotfile path?"
		$answer = Read-Host "[y\N]"
		if ($answer -eq "Y" -or $answer -eq "y") {
			# $curdir -replace "\c", ""
			"path = ${curdir}" > $config
		}
	} else {
		"There are already path set for your dotfiles,"
		"are you sure to overwrite it?"
		$answer = Read-Host "[y\N]"
		if ($answer -eq "Y" -or $answer -eq "y") {
			# $curdir -replace "\c", ""
			"path = ${curdir}" > $config
		}
	}
}

function total_count() { $totals = 0; foreach ($pkg in ($choice -split " ")) { $totals++ }; return $totals }
$totals = total_count

function remove() {
	total_count
	foreach ($pkg in $choice) {
		if (Test-Path "${lokasi}\${pkg}") {
			Write-Host "(${number}\${totals}) $pkg" -nonewline
			# Delete choices in '.dan'
			$path = (gc $dancfg | sls -SimpleMatch $pkg | foreach { ($_ -split '\s+')[2] })
			gc $dancfg | ? { $_ -notmatch [regex]::Escape($path) } | sc $dancfg
			$?; if ($?) { Write-Host "Removed" -nonewline } else { Write-Host "Failed" -nonewline -foregroundcolor red }
			# Delete choice inside dotfiles directory
			rm -force ${lokasi}\${pkg}
			$?; if ($?) { Write-Host "Deleted" -nonewline } else { Write-Host "Failed" -nonewline -foregroundcolor red }
		} else {
			"there's no folder nor file named $pkg in the dotfiles!"
		}
		$number++
	}
}

function sync() {
	total_count
	if ([string]::IsNullOrWhiteSpace($choice)) {
		foreach ($pkg in $pkglist) { $totals++ }
		foreach ($pkg in $pkglist) {
			Write-Host "(${number}\${totals}) $pkg" -nonewline
			if (Test-Path "${lokasi}\${pkg}") { rm -Recurse -Force -Confirm:$false "${lokasi}\${pkg}" }
			$path = (gc $dancfg | sls -SimpleMatch $pkg | foreach { ($_ -split '\s+')[2] })
			cp -Recurse -Force -Confirm:$false "$path" "$lokasi"
			if ($?) { Write-Host " Copied" -foregroundcolor green } else { Write-Host " Failed" -foregroundcolor red } 
			$number++
		}
	} else {
		foreach ($pkg in ($choice -split " ")) {
			# if existed in dotfiles, delete
			if (Test-Path "${lokasi}\${pkg}") { rm -Recurse -Force -Confirm:$false "${lokasi}\${pkg}" }
			# if existed in current directory, then
			if (Test-Path "${curdir}\${pkg}") {
				$pkg = Split-Path $pkg -Leaf
				Write-Host "(${number}\${totals}) $pkg" -nonewline
				# check if exist in .dan, if not add to .dan
				$match = gc $dancfg | foreach { ($_ -split '\s+')[2] } | where { $_ -eq "${curdir}\${pkg}" }
				if (-not $match) { "$pkg = ${curdir}\${pkg}" | Out-File "$dancfg" -Encoding utf8 -Append }
				if ($?) { Write-Host " Synced" -nonewline -foregroundcolor green } else { Write-Host " Failed" -nonewline -foregroundcolor red }
				# then copy choices to dotfiles
				cp -Recurse -Force -Confirm:$false "${curdir}\${pkg}" "$lokasi"
				if ($?) { Write-Host " Copied" -foregroundcolor green } else { Write-Host " Failed" -foregroundcolor red } 
			} else {
				Write-Host "(${number}\${totals}) $pkg" -nonewline
				$path = (gc $dancfg | sls -SimpleMatch $pkg | foreach { ($_ -split '\s+')[2] })
				cp -Recurse -Force -Confirm:$false "$path" "$lokasi"
				if ($?) { Write-Host " Copied" -foregroundcolor green } else { Write-Host " Failed" -foregroundcolor red } 
			}
			$number++
		}
	}
}

# ,---------,
# | Run it! |
# '---------'
if ([string]::IsNullOrEmpty($option)) {
	"Dan - Dotfile mANager"
	""
	if ( (Test-Path "$config") -and ( (Get-Item "$config").length -eq 0) ) {
		"There are no path for your dotfiles."
		"Please do 'dan init' inside the directory"
		"to set it as the path."
		""
	} else {
		"Your dotfile path:"
		"$lokasi"
		""
	}
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
