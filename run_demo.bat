@echo off
setlocal enabledelayedexpansion

set "EXE=.\build\HallReverb_NOGUI.exe"
set "INPUT=clean.wav"

set "FMS=40"
set "DRY=0.8"
set "ELVL=0.1"
set "ESND=0.2"
set "LLVL=0.2"
set "EOHPF=4.0"
set "EOLPF=16000.0"
set "ERSZ=0.5"
set "ESW=1.0"
set "LAPFB=0.63"
set "LXH=3600.0"
set "LXL=500.0"
set "LDCY=5.6"
set "LDFH=0.3"
set "LDFL=1.3"
set "LDIFF=0.82"
set "LF1=0.9"
set "LF2=1.3"
set "LFFAC=0.31"
set "LOHPF=4.0"
set "LOLPF=16000.0"
set "LPRE=8.0"
set "LRSZ=0.5"
set "LSPN=2.4"
set "LSFAC=0.3"
set "LSW=1.0"
set "LWND=22.0"

set "OUTPUT=out_FMS%FMS%_D%DRY%_EL%ELVL%_ES%ESND%_LL%LLVL%_EOH%EOHPF%_EOL%EOLPF%_ERS%ERSZ%_ESW%ESW%_LAP%LAPFB%_LXH%LXH%_LXL%LXL%_LD%LDCY%_LDH%LDFH%_LDL%LDFL%_LDF%LDIFF%_LF1%LF1%_LF2%LF2%_LFF%LFFAC%_LOH%LOHPF%_LOL%LOLPF%_LPR%LPRE%_LRS%LRSZ%_LSP%LSPN%_LSF%LSFAC%_LSW%LSW%_LW%LWND%.wav"

if not exist "%EXE%" (
    echo [ERROR] Executable not found: %EXE%
    pause
    exit /b 1
)

echo Processing...
"%EXE%" "%INPUT%" "%OUTPUT%" ^
 --frame-ms %FMS% ^
 --dry %DRY% ^
 --early-level %ELVL% ^
 --early-send %ESND% ^
 --late-level %LLVL% ^
 --early-output-hpf %EOHPF% ^
 --early-output-lpf %EOLPF% ^
 --early-room-size %ERSZ% ^
 --early-stereo-width %ESW% ^
 --late-ap-feedback %LAPFB% ^
 --late-crossover-high %LXH% ^
 --late-crossover-low %LXL% ^
 --late-decay %LDCY% ^
 --late-decay-factor-high %LDFH% ^
 --late-decay-factor-low %LDFL% ^
 --late-diffusion %LDIFF% ^
 --late-lfo1-freq %LF1% ^
 --late-lfo2-freq %LF2% ^
 --late-lfo-factor %LFFAC% ^
 --late-output-hpf %LOHPF% ^
 --late-output-lpf %LOLPF% ^
 --late-predelay %LPRE% ^
 --late-room-size %LRSZ% ^
 --late-spin %LSPN% ^
 --late-spin-factor %LSFAC% ^
 --late-stereo-width %LSW% ^
 --late-wander %LWND%

if errorlevel 1 (
    echo [ERROR] Failed.
) else (
    echo [SUCCESS] Saved as: %OUTPUT%
)
endlocal