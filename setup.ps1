$ErrorActionPreference = "Stop"

$ToolsDir = "$PSScriptRoot\tools"
if (-not (Test-Path $ToolsDir)) {
    New-Item -ItemType Directory -Path $ToolsDir | Out-Null
}

# --- Download and extract NASM ---
$NasmDir = "$ToolsDir\nasm"
if (-not (Test-Path "$NasmDir\nasm.exe")) {
    Write-Host "Downloading NASM 2.16.03..."
    $NasmZip = "$ToolsDir\nasm.zip"
    Invoke-WebRequest -Uri "https://www.nasm.us/pub/nasm/releasebuilds/2.16.03/win64/nasm-2.16.03-win64.zip" -OutFile $NasmZip
    
    Write-Host "Extracting NASM..."
    Expand-Archive -Path $NasmZip -DestinationPath "$ToolsDir\nasm_temp" -Force
    
    if (-not (Test-Path $NasmDir)) {
        New-Item -ItemType Directory -Path $NasmDir | Out-Null
    }
    
    # Move contents from the extracted subfolder
    Move-Item -Path "$ToolsDir\nasm_temp\nasm-2.16.03\*" -Destination $NasmDir -Force
    
    Remove-Item -Path "$ToolsDir\nasm_temp" -Recurse -Force
    Remove-Item -Path $NasmZip -Force
    Write-Host "NASM installed successfully."
} else {
    Write-Host "NASM is already installed."
}

# --- Download and extract QEMU ---
$QemuDir = "$ToolsDir\qemu"
if (-not (Test-Path "$QemuDir\qemu-system-x86_64.exe")) {
    Write-Host "Downloading QEMU Portable v8.2.0..."
    $QemuZip = "$ToolsDir\qemu.zip"
    # Using dirkarnez's portable QEMU build for Windows
    Invoke-WebRequest -Uri "https://github.com/dirkarnez/qemu-portable/releases/download/20240822/qemu-w64-portable-20240822.zip" -OutFile $QemuZip
    
    Write-Host "Extracting QEMU..."
    Expand-Archive -Path $QemuZip -DestinationPath $QemuDir -Force
    
    Remove-Item -Path $QemuZip -Force
    Write-Host "QEMU installed successfully."
} else {
    Write-Host "QEMU is already installed."
}

# --- Download and extract LLVM/Clang ---
$LlvmDir = "$ToolsDir\llvm"
if (-not (Test-Path "$LlvmDir\bin\clang.exe")) {
    Write-Host "Downloading LLVM-MinGW Portable..."
    $LlvmZip = "$ToolsDir\llvm.zip"
    Invoke-WebRequest -Uri "https://github.com/mstorsjo/llvm-mingw/releases/download/20260826/llvm-mingw-20260826-msvcrt-x86_64.zip" -OutFile $LlvmZip
    
    Write-Host "Extracting LLVM..."
    Expand-Archive -Path $LlvmZip -DestinationPath "$ToolsDir\llvm_temp" -Force
    
    if (-not (Test-Path $LlvmDir)) {
        New-Item -ItemType Directory -Path $LlvmDir | Out-Null
    }
    
    Move-Item -Path "$ToolsDir\llvm_temp\llvm-mingw-20260826-msvcrt-x86_64\*" -Destination $LlvmDir -Force
    Remove-Item -Path "$ToolsDir\llvm_temp" -Recurse -Force
    Remove-Item -Path $LlvmZip -Force
    Write-Host "LLVM installed successfully."
} else {
    Write-Host "LLVM is already installed."
}

# --- Download and extract xorriso (for ISO creation) ---
$XorrisoDir = "$ToolsDir\xorriso"
if (-not (Test-Path "$XorrisoDir\xorriso.exe")) {
    Write-Host "Downloading xorriso..."
    $XorrisoZip = "$ToolsDir\xorriso.zip"
    Invoke-WebRequest -Uri "https://github.com/dEajL3kA/xorriso-win32/releases/download/2026-02-25/xorriso-win32.2026-02-25.zip" -OutFile $XorrisoZip
    Write-Host "Extracting xorriso..."
    Expand-Archive -Path $XorrisoZip -DestinationPath "$ToolsDir\xorriso_temp" -Force
    if (-not (Test-Path $XorrisoDir)) { New-Item -ItemType Directory -Path $XorrisoDir | Out-Null }
    Move-Item -Path "$ToolsDir\xorriso_temp\*" -Destination $XorrisoDir -Force
    Remove-Item -Path "$ToolsDir\xorriso_temp" -Recurse -Force
    Remove-Item -Path $XorrisoZip -Force
    Write-Host "xorriso installed successfully."
} else { Write-Host "xorriso is already installed." }

# --- Download and extract Limine (Bootloader for ISO) ---
$LimineDir = "$ToolsDir\limine"
if (-not (Test-Path "$LimineDir\limine-bios-cd.bin")) {
    Write-Host "Downloading Limine..."
    $LimineZip = "$ToolsDir\limine.zip"
    $LimineUrl = (Invoke-RestMethod -Uri "https://api.github.com/repos/limine-bootloader/limine/releases/latest").assets | Where-Object { $_.name -eq "limine-binary.zip" } | Select-Object -First 1 -ExpandProperty browser_download_url
    Invoke-WebRequest -Uri $LimineUrl -OutFile $LimineZip
    Write-Host "Extracting Limine..."
    Expand-Archive -Path $LimineZip -DestinationPath "$ToolsDir\limine_temp" -Force
    if (-not (Test-Path $LimineDir)) { New-Item -ItemType Directory -Path $LimineDir | Out-Null }
    Move-Item -Path "$ToolsDir\limine_temp\limine-binary\*" -Destination $LimineDir -Force
    Remove-Item -Path "$ToolsDir\limine_temp" -Recurse -Force
    Remove-Item -Path $LimineZip -Force
    Write-Host "Limine installed successfully."
} else { Write-Host "Limine is already installed." }

Write-Host "`nSetup complete! You can now use build.bat, run.bat, and iso.bat."
