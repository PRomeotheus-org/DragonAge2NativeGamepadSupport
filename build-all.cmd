@echo off
rem Builds libMinHook.x86.lib (static, x86) from E:\RE\.tools\minhook source,
rem then builds the shipping dinput8.dll from repo source with the author's flags.
rem Hermes adaptation 2026-11-01 (author's workspace expected a prebuilt lib in ..\).

call "I:\VISUAL Studio\BuildTools\VC\Auxiliary\Build\vcvars32.bat" >nul || exit /b 1

echo ===MINHOOK-STATIC-BUILD===
if exist "E:\RE\Projects\build_mh" rmdir /s /q "E:\RE\Projects\build_mh"
mkdir "E:\RE\Projects\build_mh" || exit /b 2
cd /d "E:\RE\Projects\build_mh" || exit /b 2
mkdir mhobj || exit /b 2
cl /c /MT /O2 /nologo /I "E:\RE\Projects\minhook-src\include" /Fomhobj\ ^
   "E:\RE\Projects\minhook-src\src\buffer.c" ^
   "E:\RE\Projects\minhook-src\src\hook.c" ^
   "E:\RE\Projects\minhook-src\src\trampoline.c" ^
   "E:\RE\Projects\minhook-src\src\hde\hde32.c" ^
   "E:\RE\Projects\minhook-src\src\hde\hde64.c" || exit /b 3
lib /nologo /OUT:"libMinHook.x86.lib" mhobj\buffer.obj mhobj\hook.obj mhobj\trampoline.obj mhobj\hde32.obj mhobj\hde64.obj || exit /b 4
copy /Y "libMinHook.x86.lib" "E:\RE\Projects\libMinHook.x86.lib" || exit /b 5
echo libMinHook built:
dir "E:\RE\Projects\libMinHook.x86.lib" | findstr libMinHook

echo ===PROXY-BUILD===
cd /d "E:\RE\Projects\DA2NativeGamepadSupport" || exit /b 6
call build.bat || exit /b 7

echo ===ARTIFACT===
if not exist dinput8.dll exit /b 8
dir dinput8.dll | findstr dinput8
certutil -hashfile dinput8.dll SHA256 | sed -n 2p 2>nul || certutil -hashfile dinput8.dll SHA256
echo ===ALL-DONE===
