@echo off
rem One-shot source build for a machine that has nothing prebuilt.
rem Builds libMinHook.x86.lib (static, x86) from MinHook source, then calls the
rem author's build.bat unchanged, then prints the artifact hash.
rem
rem Everything is overridable - no drive letters baked in:
rem   MH_SRC   MinHook source folder (needs include\ and src\)   default: %~dp0minhook-src
rem   OUT_DIR  where libMinHook.x86.lib goes (build.bat expects the parent)  default: %~dp0..
rem   VCVARS   full path to vcvars32.bat (x86)   default: looked up with vswhere
rem Example:
rem   set "MH_SRC=D:\src\minhook" && set "VCVARS=D:\VS\BuildTools\VC\Auxiliary\Build\vcvars32.bat" && build-all.cmd

setlocal
set "REPO=%~dp0"
if not defined MH_SRC set "MH_SRC=%REPO%minhook-src"
if not defined OUT_DIR set "OUT_DIR=%REPO%.."

if defined VCVARS goto :havevcvars
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" goto :novcvars
for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -find VC\Auxiliary\Build\vcvars32.bat`) do set "VCVARS=%%I"
if not defined VCVARS goto :novcvars
:havevcvars
if not exist "%VCVARS%" goto :novcvars
call "%VCVARS%" >nul || exit /b 1

if not exist "%MH_SRC%\src\hook.c" (
  echo MinHook source not found at "%MH_SRC%".
  echo Get MinHook source there, or set MH_SRC to where it lives.
  exit /b 2
)

echo ===MINHOOK-STATIC-BUILD===
if defined LOCALAPPDATA set "BLD=%LOCALAPPDATA%\Temp\da2_minhook_build"
if not defined BLD set "BLD=%TEMP%\da2_minhook_build"
if not exist "%BLD%" mkdir "%BLD%" || exit /b 3
if not exist "%BLD%\mhobj" mkdir "%BLD%\mhobj" || exit /b 3
pushd "%BLD%" || exit /b 3
cl /c /MT /O2 /nologo /I "%MH_SRC%\include" /Fomhobj\ ^
   "%MH_SRC%\src\buffer.c" ^
   "%MH_SRC%\src\hook.c" ^
   "%MH_SRC%\src\trampoline.c" ^
   "%MH_SRC%\src\hde\hde32.c" ^
   "%MH_SRC%\src\hde\hde64.c" || (popd & exit /b 4)
lib /nologo /OUT:"libMinHook.x86.lib" mhobj\buffer.obj mhobj\hook.obj mhobj\trampoline.obj mhobj\hde32.obj mhobj\hde64.obj || (popd & exit /b 5)
if not defined MH_LIB set "MH_LIB=%OUT_DIR%\libMinHook.x86.lib"
copy /Y "libMinHook.x86.lib" "%MH_LIB%" || (popd & exit /b 6)
echo libMinHook built: "%MH_LIB%"
popd

echo ===PROXY-BUILD===
cd /d "%REPO%" || exit /b 7
set "MH_INC=%MH_SRC%\include"
call build.bat || exit /b 8

echo ===ARTIFACT===
if not exist dinput8.dll exit /b 9
dir dinput8.dll | findstr dinput8
certutil -hashfile dinput8.dll SHA256
echo ===ALL-DONE===
endlocal
exit /b 0

:novcvars
echo Could not find the x86 MSVC environment (vcvars32.bat).
echo Either run this from a "x86 Native Tools Command Prompt", or:
echo   set "VCVARS=C:\path\to\vcvars32.bat"
echo ^(Visual Studio Build Tools with the Desktop C++ workload.^)
exit /b 10
