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

Write-Host "`nSetup complete! You can now use build.bat and run.bat."
