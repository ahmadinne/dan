#! \usr\bin\env powershell

# ,-----------,
# | Variables |
# '-----------'
$curdir = (gl).path
$option = $args[0]
if ($args.Length -gt 1) { $choice = $args[1..($args.Length - 1)] -join " " } else { $choice = "" }
$cfgdir = "~\.config\dan" #Dan configuration options folder
$config = "${cfgdir}\config" #Dan configuration options file
$lokasi = (gc $config | sls -SimpleMatch "path" | foreach { ($_ -split '=',2)[1].Trim() }) #path for the dotfiles
$dancfg = "${lokasi}\.dan" #path for the list of path for the folders inside the dotfiles

$pkglist = gc $dancfg | foreach { ($_ -split '\s+')[0] } | sort #List of Path for the folders inside the dotfiles
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
			if ($?) { Write-Host "Path has been set!" -foregroundcolor green } else { Write-Host "Failed to set path." -foregroundcolor red }
		}
	}
}

function total_count() { $totals = 0; foreach ($pkg in ($choice -split " ")) { $totals++ }; return $totals }
function total_counts() { $totals = -1; foreach ($pkg in ($choice -split " ")) { $totals++ }; return $totals }
$totals = total_count

function remove() {
	total_count > $null 2>&1
	Write-Host "You will remove those from the dotfiles" -foregroundcolor red
	Write-Host "Continue to remove those? " -nonewline
	$answer = Read-Host "[y\N]"
	if ($answer -eq "Y" -or $answer -eq "y") {
		foreach ($pkg in ($choice -split " ")) {
			Write-Host "(${number}\${totals}) $pkg" -nonewline
			if (Test-Path "${lokasi}\${pkg}") {
				# Delete choices in '.dan'
				$rem = gc $dancfg | where {
					($_.Split('=',2)[0].Trim()) -ne "$pkg"
				}
				$rem | sc $dancfg -Encoding utf8
				if ($?) { Write-Host " Removed" -foregroundcolor green -nonewline } else { Write-Host " Failed" -foregroundcolor red -nonewline }
				# Delete choice inside dotfiles directory
				rm -Recurse -Force -Confirm:$false ${lokasi}\${pkg}
				if ($?) { Write-Host " Deleted" -foregroundcolor green -nonewline } else { Write-Host " Failed" -foregroundcolor red -nonewline }
			} else {
				"there's no folder nor file named $pkg in the dotfiles!"
			}
			$number++
		}
	} else { Write-Host "Process cancelled." -foregroundcolor yellow }
}

function sync() {
	total_counts > $null 2>&1
	if ([string]::IsNullOrWhiteSpace($choice)) {
		Write-Host "It'll  replace everything inside the dotfiles" -foregroundcolor red
		Write-Host "with everything from the localhost" -foregroundcolor red
		Write-Host "Continue to sync all? " -nonewline
		$answer = Read-Host "[y\N]"
		if ($answer -eq "Y" -or $answer -eq "y") {
			foreach ($pkg in $pkglist) { $totals++ }
			foreach ($pkg in $pkglist) {
				Write-Host "(${number}\${totals}) $pkg" -nonewline
				if (Test-Path "${lokasi}\${pkg}") { rm -Recurse -Force -Confirm:$false "${lokasi}\${pkg}" }
				$path = (gc $dancfg | sls -SimpleMatch $pkg | foreach { ($_ -split '=',2)[1].Trim() })
				cp -Recurse -Force -Confirm:$false "$path" "$lokasi"
				if ($?) { Write-Host " Copied" -foregroundcolor green } else { Write-Host " Failed" -foregroundcolor red } 
				$number++
			}
		} else { Write-Host "Process cancelled." -foregroundcolor yellow }
	} else {
		Write-Host "You will replace these from the dotfiles:" -foregroundcolor red
		foreach ($pkg in ($choice -split " ")) { Write-Host "$pkg" -foregroundcolor blue }
		Write-Host "Continue to sync those? " -nonewline
		$answer = Read-Host "[y\N]"
		if ($answer -eq "Y" -or $answer -eq "y") {
			foreach ($pkg in ($choice -split " ")) {
				# if existed in dotfiles, delete
				if (Test-Path "${lokasi}\${pkg}") { rm -Recurse -Force -Confirm:$false "${lokasi}\${pkg}" }
				# if existed in current directory, then
				if (Test-Path "${curdir}\${pkg}") {
					$pkg = Split-Path $pkg -Leaf
					Write-Host "(${number}\${totals}) $pkg" -nonewline
					# check if exist in .dan, if not add to .dan
					$match = gc $dancfg | foreach { ($_ -split '\s+')[2] } | where { $_ -eq "${curdir}\${pkg}" }
					if (-not $match) { 
						$line = "$pkg = ${curdir}\${pkg}"
						$clean = $line -replace "[`r`n]+$", ""
						($clean + "`n") | ac "$dancfg" -Encoding utf8
						(gc $dancfg | Select-Object -SkipLast 1) | sc $dancfg -Encoding utf8
					}
					if ($?) { Write-Host " Synced" -nonewline -foregroundcolor green } else { Write-Host " Failed" -nonewline -foregroundcolor red }
					# then copy choices to dotfiles
					cp -Recurse -Force -Confirm:$false "${curdir}\${pkg}" "$lokasi"
					if ($?) { Write-Host " Copied" -foregroundcolor green } else { Write-Host " Failed" -foregroundcolor red } 
				} else {
					Write-Host "(${number}\${totals}) $pkg" -nonewline
					$path = (gc $dancfg | sls -SimpleMatch $pkg | foreach { ($_ -split '=',2)[1].Trim() })
					cp -Recurse -Force -Confirm:$false "$path" "$lokasi"
					if ($?) { Write-Host " Copied" -foregroundcolor green } else { Write-Host " Failed" -foregroundcolor red } 
				}
				$number++
			}
		} else { Write-Host "Process cancelled." -foregroundcolor yellow }
	}
}

function apply() {
	total_counts > $null 2&>1
	if ([string]::IsNullorWhiteSpace($choice)) {
		Write-Host "It'll  replace everything on the local side" -foregroundcolor red
		Write-Host "with everything from the dotfiles" -foregroundcolor red
		Write-Host "Continue to apply all? " -nonewline
		$answer = Read-Host "[y\N]"
		if ($answer -eq "Y" -or $answer -eq "y") {
			foreach ($pkg in $pkglist) { $totals++ }
			foreach ($pkg in $pkglist) {
				$path = (gc $dancfg | sls -SimpleMatch $pkg | foreach { ($_ -split '=',2)[1].Trim() })
				Write-Host "(${number}\${totals}) $pkg" -nonewline
				if (Test-Path "${path}") { rm -Recurse -Force -Confirm:$false "${path}" } #Delete first if folders already there.
				cp -Recurse -Force -Confirm:$false "${lokasi}/${pkg}" "$path"
				if ($?) { Write-Host " Applied" -foregroundcolor green } else { Write-Host " Failed" -foregroundcolor red } 
				$number++
			}
		} else { Write-Host "Process cancelled." -foregroundcolor yellow }
	} else {}
}

# ,---------,
# | Run it! |
# '---------'
if (-not (Test-Path "$cfgdir")) { ni -ItemType Directory -Path "$cfgdir" -Force -Confirm:$false }
if (-not (Test-Path "$config")) { ni -ItemType File -Path "$config" -Force -Confirm:$false }
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
