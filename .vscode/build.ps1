# -----------------------------------------------------------------------------
# FILE: .vscode/build.ps1
# DESCRIPTION: PRO-level C++20 Smart Auto-Resolving Builder (PS 5.1 Safe)
# -----------------------------------------------------------------------------

# Force UTF-8 for proper terminal output
[Console]::OutputEncoding = [System.Text.Encoding]::UTF8
$OutputEncoding = [System.Text.Encoding]::UTF8

Write-Host "[INFO] STARTING MAGIC MSVC BUILDER..." -ForegroundColor Cyan

# 1. ENVIRONMENT CHECK
if (-not (Get-Command cl.exe -ErrorAction SilentlyContinue)) {
    Write-Host "[ERROR] cl.exe not found. Run from Developer PowerShell!" -ForegroundColor Red
    exit 1
}

$BuildDir = ".build_debug"
if (-not (Test-Path $BuildDir)) { New-Item -Path $BuildDir -ItemType Directory | Out-Null }
$ExeName = "debug_build_999.exe"
$ExePath = "${BuildDir}\${ExeName}"

# 2. SAFELY COLLECT FILES (Bulletproof against PS parser bugs)
Write-Host "[INFO] Finding source files..." -ForegroundColor Gray
$AllFiles = Get-ChildItem -Path . -File -Recurse

# Using -notlike is safer than regex -match in PowerShell
$IxxFiles = $AllFiles | Where-Object { $_.Extension -eq '.ixx' -and $_.FullName -notlike '*node_modules*' -and $_.FullName -notlike '*.build_debug*' -and $_.FullName -notlike '*.git*' -and $_.FullName -notlike '*.vscode*' }
$CppFiles = $AllFiles | Where-Object { $_.Extension -eq '.cpp' -and $_.FullName -notlike '*node_modules*' -and $_.FullName -notlike '*.build_debug*' -and $_.FullName -notlike '*.git*' -and $_.FullName -notlike '*.vscode*' }

if (-not $IxxFiles -and -not $CppFiles) {
    Write-Host "[FATAL] No .ixx or .cpp files found!" -ForegroundColor Red
    exit 1
}

# Base compiler arguments passed safely as an array. Adding /Fd to push vc140.pdb to build dir.
$BaseArgs = @("/std:c++20", "/EHsc", "/Zi", "/nologo", "/utf-8", "/c", "/Fo${BuildDir}\", "/Fd${BuildDir}\", "/ifcSearchDir", $BuildDir)

# 3. MAGIC MODULE RESOLVER (Let the compiler figure out the order!)
$PendingModules = New-Object System.Collections.Generic.List[string]
if ($IxxFiles) { foreach ($f in $IxxFiles) { $PendingModules.Add($f.FullName) } }

Write-Host "[INFO] Compiling Modules (Auto-resolving dependencies)..." -ForegroundColor Gray

while ($PendingModules.Count -gt 0) {
    $CompiledThisPass = 0
    $NextPending = New-Object System.Collections.Generic.List[string]
    $LastErrorOutput = $null
    $FirstFailedName = ""

    foreach ($Mod in $PendingModules) {
        $FileName = Split-Path $Mod -Leaf
        $Args = $BaseArgs + @("/ifcOutput", "${BuildDir}\", $Mod)
        
        # Try to compile and capture the output silently
        $Output = & cl.exe $Args 2>&1
        
        if ($LASTEXITCODE -eq 0) {
            Write-Host "   [SUCCESS] Compiled: $FileName" -ForegroundColor Green
            $CompiledThisPass++
        } else {
            # Failed? It probably needs another module. Put it back in the queue!
            $NextPending.Add($Mod)
            if ($LastErrorOutput -eq $null) {
                $LastErrorOutput = $Output
                $FirstFailedName = $FileName
            }
        }
    }

    # If we made a full circle and compiled nothing, we have a real syntax error
    if ($CompiledThisPass -eq 0) {
        Write-Host "[FATAL] Syntax error or unresolvable dependency!" -ForegroundColor Red
        Write-Host "[ERROR] Real Compiler Error in ${FirstFailedName}:" -ForegroundColor Yellow
        $LastErrorOutput | ForEach-Object { Write-Host $_ -ForegroundColor Red }
        exit 1
    }
    $PendingModules = $NextPending
}

# 4. COMPILE CPP FILES
Write-Host "[INFO] Compiling Source Files..." -ForegroundColor Gray
if ($CppFiles) {
    foreach ($Cpp in $CppFiles) {
        $FileName = Split-Path $Cpp.FullName -Leaf
        Write-Host "   -> Compiling: $FileName" -ForegroundColor Gray
        
        $Args = $BaseArgs + @($Cpp.FullName)
        $Output = & cl.exe $Args 2>&1
        
        if ($LASTEXITCODE -ne 0) { 
            Write-Host "[ERROR] IN $FileName" -ForegroundColor Red
            $Output | ForEach-Object { Write-Host $_ -ForegroundColor Red }
            exit $LASTEXITCODE 
        }
    }
}

# 5. LINKING
Write-Host "[INFO] Linking Executable..." -ForegroundColor Gray
$Objs = Get-ChildItem -Path $BuildDir -Filter *.obj
$ObjPaths = $Objs | ForEach-Object { $_.FullName }

if (-not $ObjPaths) {
    Write-Host "[FATAL] No object files found to link!" -ForegroundColor Red
    exit 1
}

# Explicitly directing EXE, PDB, and ILK to the build folder
$LinkerArgs = @("/nologo", "/Zi", "/utf-8") + $ObjPaths + @("/Fe${ExePath}", "/link", "/DEBUG", "/PDB:${BuildDir}\${ExeName}.pdb", "/ILK:${BuildDir}\${ExeName}.ilk")
& cl.exe $LinkerArgs

if ($LASTEXITCODE -eq 0) {
    Write-Host "[SUCCESS] MAGIC COMPLETE! Launching debugger..." -ForegroundColor Green
} else {
    Write-Host "[ERROR] LINKING FAILED!" -ForegroundColor Red
    exit 1
}