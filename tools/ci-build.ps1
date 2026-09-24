#Requires -Version 7.0
<#
.SYNOPSIS
Build the fixed 0812 firmware in a new directory, without writing into this package.
.EXAMPLE
./tools/ci-build.ps1 -PythonPath C:/Python312/python.exe -BashPath C:/msys64/usr/bin/bash.exe -WorkDirectory C:/p75-build-1
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$PythonPath,
    [Parameter(Mandatory)][string]$BashPath,
    [Parameter(Mandatory)][string]$WorkDirectory
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
# Check native exit codes explicitly, including when stdout is piped into a log.
$PSNativeCommandUseErrorActionPreference = $false

function Invoke-LoggedNative {
    param([string]$Program, [string[]]$Arguments, [string]$LogPath)
    & $Program @Arguments 2>&1 | Tee-Object -FilePath $LogPath
    if ($LASTEXITCODE -ne 0) {
        throw "$Program failed with exit code $LASTEXITCODE; see $LogPath"
    }
}

if (-not $IsWindows) { throw 'This build recipe requires Windows.' }
$package = Split-Path -Parent $PSScriptRoot
$PythonPath = (Resolve-Path -LiteralPath $PythonPath).Path
$BashPath = (Resolve-Path -LiteralPath $BashPath).Path
$work = [IO.Path]::GetFullPath($WorkDirectory)
if ($work -match '\s') { throw 'WorkDirectory must not contain whitespace (legacy Make paths).' }
$packagePrefix = [IO.Path]::GetFullPath($package).TrimEnd('\', '/') + [IO.Path]::DirectorySeparatorChar
if ($work.Equals($packagePrefix.TrimEnd('\'), [StringComparison]::OrdinalIgnoreCase) -or
    $work.StartsWith($packagePrefix, [StringComparison]::OrdinalIgnoreCase)) {
    throw 'WorkDirectory must be outside the source package.'
}
if (Test-Path -LiteralPath $work) { throw "WorkDirectory already exists; choose a new directory: $work" }

New-Item -ItemType Directory -Path $work | Out-Null
$logs = New-Item -ItemType Directory -Path (Join-Path $work 'logs')
$temp = New-Item -ItemType Directory -Path (Join-Path $work 'temp')
$environmentNames = @('TEMP', 'TMP', 'PYTHONDONTWRITEBYTECODE', 'PYTHONIOENCODING',
    'P75_CI_PYTHON', 'P75_CI_GCC_BIN', 'P75_CI_BUILD_DIR', 'P75_CI_SOURCE', 'MSYSTEM')
$savedEnvironment = @{}
foreach ($name in $environmentNames) { $savedEnvironment[$name] = [Environment]::GetEnvironmentVariable($name, 'Process') }
Start-Transcript -Path (Join-Path $logs 'session.log') | Out-Null
try {
    $env:TEMP = $temp.FullName
    $env:TMP = $temp.FullName
    $env:PYTHONDONTWRITEBYTECODE = '1'
    $env:PYTHONIOENCODING = 'utf-8'
    Invoke-LoggedNative $PythonPath @('-c', 'import os,sys; print(sys.executable, sys.version); assert os.name == "nt" and sys.version_info[:2] == (3,12), "Windows Python 3.12 is required"') (Join-Path $logs 'python.log')

    $toolchainUrl = 'https://developer.arm.com/-/media/Files/downloads/gnu-rm/10.3-2021.10/gcc-arm-none-eabi-10.3-2021.10-win32.zip'
    $archive = Join-Path $work 'gcc-arm-none-eabi-10.3-2021.10-win32.zip'
    Write-Host "Downloading anonymously: $toolchainUrl"
    Invoke-WebRequest -Uri $toolchainUrl -OutFile $archive
    $archiveBytes = (Get-Item -LiteralPath $archive).Length
    $archiveHash = (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash
    Write-Host "Toolchain download: $archiveBytes bytes; SHA256 $archiveHash"
    if ($archiveBytes -ne 200578763 -or $archiveHash -ne 'D287439B3090843F3F4E29C7C41F81D958A5323AECEFCF705C203BFD8AE3F2E7') {
        throw 'Toolchain archive size or SHA256 mismatch.'
    }
    $toolchain = Join-Path $work 'toolchain'
    Expand-Archive -LiteralPath $archive -DestinationPath $toolchain
    $compilers = @(Get-ChildItem -LiteralPath $toolchain -Filter arm-none-eabi-gcc.exe -File -Recurse)
    if ($compilers.Count -ne 1) { throw 'Expected exactly one Arm compiler in the verified archive.' }

    $venv = Join-Path $work 'venv'
    Invoke-LoggedNative $PythonPath @('-m', 'venv', $venv) (Join-Path $logs 'venv.log')
    $venvPython = Join-Path $venv 'Scripts/python.exe'
    $requirements = Join-Path $package 'metadata/python-packages-observed.txt'
    Write-Host "Installing pinned Python versions from $requirements"
    Invoke-LoggedNative $venvPython @('-m', 'pip', '--isolated', 'install', '--no-cache-dir', '--index-url', 'https://pypi.org/simple', '-r', $requirements) (Join-Path $logs 'pip-install.log')
    Invoke-LoggedNative $venvPython @('-m', 'pip', '--isolated', 'check') (Join-Path $logs 'pip-check.log')
    Invoke-LoggedNative $venvPython @('-m', 'pip', '--isolated', 'freeze', '--all') (Join-Path $logs 'python-packages-installed.txt')

    # QMK also copies the final BIN into its source root. Build a private copy so
    # the caller's source package and Git checkout remain untouched.
    $source = New-Item -ItemType Directory -Path (Join-Path $work 'source')
    Copy-Item -LiteralPath (Join-Path $package 'qmk_firmware') -Destination $source.FullName -Recurse
    Copy-Item -LiteralPath (Join-Path $package 'tools') -Destination $source.FullName -Recurse
    $env:P75_CI_PYTHON = $venvPython
    $env:P75_CI_GCC_BIN = $compilers[0].DirectoryName
    $env:P75_CI_BUILD_DIR = Join-Path $work 'build'
    $env:P75_CI_SOURCE = $source.FullName
    $env:MSYSTEM = 'MSYS'
    # Do not inherit another Python, compiler, or make from the Windows PATH.
    # Windows executables receive native paths via MSYS argument conversion;
    # shell executables and QMK_PYTHON use explicit cygpath-converted paths.
    $buildCommand = @'
set -euo pipefail
export PATH=/usr/bin:/bin
export QMK_PYTHON="$(cygpath -u "$P75_CI_PYTHON")"
export ARM_GCC_BIN="$(cygpath -u "$P75_CI_GCC_BIN")"
export P75_BUILD_DIR="$(cygpath -u "$P75_CI_BUILD_DIR")"
export QMK_BUILD_DATE=2026-08-07-00:00:00
cd "$(cygpath -u "$P75_CI_SOURCE")"
bash tools/build.sh
'@
    Invoke-LoggedNative $BashPath @('--noprofile', '--norc', '-c', $buildCommand) (Join-Path $logs 'build.log')

    $binary = Join-Path $work 'build/p75_jis_p75_jis_via.bin'
    $binaryHash = (Get-FileHash -LiteralPath $binary -Algorithm SHA256).Hash
    $binaryBytes = (Get-Item -LiteralPath $binary).Length
    if ($binaryBytes -ne 80860 -or $binaryHash -ne '21DE5AAFA6684B72AD797C47C61712F8D40F5762477DCBA483DC7BB53A4332B2') {
        throw "Rebuilt firmware mismatch: $binaryBytes bytes; SHA256 $binaryHash"
    }
    [ordered]@{
        status = 'passed'
        binary = $binary
        binary_bytes = $binaryBytes
        binary_sha256 = $binaryHash
        toolchain_url = $toolchainUrl
        toolchain_bytes = $archiveBytes
        toolchain_sha256 = $archiveHash
        build_date_metadata = '2026-08-07-00:00:00'
        verified_at_utc = [DateTime]::UtcNow.ToString('o')
    } | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $work 'build-result.json') -Encoding utf8
    Write-Host "PASS: $binaryBytes bytes; SHA256 $binaryHash"
}
finally {
    Stop-Transcript | Out-Null
    foreach ($name in $environmentNames) { [Environment]::SetEnvironmentVariable($name, $savedEnvironment[$name], 'Process') }
}
