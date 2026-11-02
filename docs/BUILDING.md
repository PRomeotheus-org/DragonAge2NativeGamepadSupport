# Building from source

Two scripts, both Windows + Visual Studio Build Tools (Desktop C++ / x86 MSVC):

- **`build.bat`** - the original one-liner. Needs the x86 Native Tools prompt and a
  MinHook static lib. Include dir and lib default to the author's layout and can be
  overridden with `MH_INC` / `MH_LIB` without editing the file.
- **`build-all.cmd`** - builds `libMinHook.x86.lib` from MinHook source first, then calls
  `build.bat` unchanged. Paths via `MH_SRC`, `OUT_DIR`, `VCVARS`; MinHook source is not
  and should not be committed here, fetch it yourself.

## Why the flags are what they are
`build.bat` compiles with `/LD /EHsc /DDA2_PROXY_BUILD ... /link /LTCG /DEF:da2_proxy.def`
and **deliberately no `/O2`** - the author's comment says every hook was verified against
code the development build generated, and a shipping binary is the wrong place to discover
that a calling convention only held at `/Od`. Do not "modernise" the flags: no `/O2`, no
CRT, no exceptions/RTTI additions, no `/GL`.

## What a correct build looks like
- `dinput8.dll`, 32-bit (`PE32`), roughly 235 KB.
- Import table of only `XINPUT9_1_0.dll`, `USER32.dll`, `SHELL32.dll`, `KERNEL32.dll`.
  No CRT import, so no Visual C++ redistributable is needed. Read it yourself rather than
  trusting this sentence - `dumpbin /imports dinput8.dll`.
- The link is **not bit-deterministic** (debug directory, timestamps), so rebuilding from
  identical sources gives a different SHA-256. Size plus import table is the pass
  condition; a hash identifies one specific binary, which is why the tested one is kept in
  `known-good/` with its hash in the name.

## What the DLL was tested against
`DragonAge2.exe`, 12,172,664 bytes, SHA-256
`478357e492859402618e2225df25d6d78a8105ea6f30f548c19602483474ed0a`, Steam appid 1238040
(Dragon Age II: Ultimate Edition), buildid 10351532. The hooks are pinned to absolute
addresses - see `REINTEGRATION.md` before running this against any other executable.
