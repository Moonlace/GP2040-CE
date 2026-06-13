param(
	[switch]$Test,
	[switch]$Bot,
	[switch]$BotMatrix,
	[ValidateRange(60, 240)]
	[int]$Bpm = 120,
	[ValidateRange(1, 3)]
	[int]$Difficulty = 2,
	[ValidateSet(32, 64)]
	[int]$Height = 64,
	[ValidateRange(2, 300)]
	[int]$DurationSeconds = 20
)

$ErrorActionPreference = 'Stop'

function Find-Executable {
	param(
		[string]$Name,
		[string]$FallbackPattern
	)

	$command = Get-Command $Name -ErrorAction SilentlyContinue
	if ($command) {
		return $command.Source
	}

	$fallback = Get-ChildItem -Path $FallbackPattern -File -ErrorAction SilentlyContinue |
		Sort-Object FullName -Descending |
		Select-Object -First 1
	if (-not $fallback) {
		throw "Unable to find $Name"
	}
	return $fallback.FullName
}

if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
	$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
	if (-not (Test-Path -LiteralPath $vswhere)) {
		throw 'Visual Studio Build Tools with the Desktop development with C++ workload are required.'
	}

	$visualStudioPath = & $vswhere -latest -products * -property installationPath
	if (-not $visualStudioPath) {
		throw 'Visual Studio C++ build tools were not found.'
	}

	$devCommand = Join-Path $visualStudioPath 'Common7\Tools\VsDevCmd.bat'
	cmd.exe /s /c "`"$devCommand`" -arch=x64 -host_arch=x64 >nul && set" |
		ForEach-Object {
			if ($_ -match '^([^=]+)=(.*)$') {
				Set-Item -Path "Env:$($matches[1])" -Value $matches[2]
			}
		}
	if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
		throw 'Visual Studio is installed, but its C++ build tools are unavailable.'
	}
}

$cmake = Find-Executable `
	-Name 'cmake.exe' `
	-FallbackPattern "$env:USERPROFILE\.pico-sdk\tools\cmake\*\bin\cmake.exe"
$ninja = Find-Executable `
	-Name 'ninja.exe' `
	-FallbackPattern "$env:USERPROFILE\.pico-sdk\tools\ninja\*\ninja.exe"
$ctest = Join-Path (Split-Path -Parent $cmake) 'ctest.exe'
$buildDirectory = Join-Path $PSScriptRoot 'build'
$executable = Join-Path $buildDirectory 'gp2040-minigame-sim.exe'

& $cmake -S $PSScriptRoot -B $buildDirectory -G Ninja `
	"-DCMAKE_BUILD_TYPE=Release" `
	"-DCMAKE_MAKE_PROGRAM=$ninja"
if ($LASTEXITCODE -ne 0) {
	exit $LASTEXITCODE
}

& $cmake --build $buildDirectory --parallel
if ($LASTEXITCODE -ne 0) {
	exit $LASTEXITCODE
}

if ($Test) {
	& $ctest --test-dir $buildDirectory --output-on-failure
	exit $LASTEXITCODE
}

if ($BotMatrix) {
	& $executable --bot-matrix --bot-duration-ms ($DurationSeconds * 1000)
	exit $LASTEXITCODE
}

if ($Bot) {
	& $executable --bot `
		--bpm $Bpm `
		--difficulty $Difficulty `
		--height $Height `
		--bot-duration-ms ($DurationSeconds * 1000)
	exit $LASTEXITCODE
}

& $executable --bpm $Bpm --difficulty $Difficulty --height $Height
exit $LASTEXITCODE
