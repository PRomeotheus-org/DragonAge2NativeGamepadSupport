@echo off
rem x86 Native Tools prompt. Builds the shipping proxy: dinput8.dll
rem Same flags as the development build on purpose. /O2 is NOT added:
rem every hook here was verified against code the dev build generated,
rem and a shipping binary is the wrong place to find out that a
rem calling convention only held at /Od.
rem
rem PRomeotheus-org/DragonAge2NativeGamepadSupport#build
rem The MinHook include dir and static lib were hard-wired to the author's own
rem workspace ( ..\libMinHook.x86.lib ), so a clean clone of this repo could not
rem build without recreating that layout by hand. The two lines below keep those
rem exact defaults and just let anyone override them without editing this file:
rem   set "MH_INC=C:\path\to\minhook-src\include"
rem   set "MH_LIB=C:\path\to\libMinHook.x86.lib"
rem No prebuilt MinHook lib on hand? build-all.cmd compiles one from MinHook
rem source first, then calls this script unchanged.
if not defined MH_INC set "MH_INC=.."
if not defined MH_LIB set "MH_LIB=..\libMinHook.x86.lib"
cl /LD /EHsc /DDA2_PROXY_BUILD da2_proxy.cpp da2_console.cpp ^
   /I"%MH_INC%" "%MH_LIB%" XInput9_1_0.lib user32.lib shell32.lib ^
   /Fe:dinput8.dll /link /LTCG /DEF:da2_proxy.def
