# Re-integrating the Hermes twist after an upstream or game update

## What is ours (everything else is PRomeotheus's, unmodified)

Two commits on top of upstream `23a003b "Removed leftover traces"`
(remote: https://github.com/PRomeotheus-org/DragonAge2NativeGamepadSupport):

| Commit | Files | Nature |
|---|---|---|
| `9551319` | `build-all.cmd` (new, +32), `build.log` (new, +37) | build only — calls vcvars32 + builds `libMinHook.x86.lib` from `E:\RE\Projects\minhook-src`, then `call build.bat`. Does NOT change his compile flags. |
| `493a3f3` | `da2_console.cpp` (+15), `da2_console.ini` (+1 key) | the "Button swap" feature only |

The whole feature is 15 lines in 3 places (line numbers are for HEAD `9114610`;
locate them by the grep strings, since lines drift after any upstream merge):

```
da2_console.cpp:110   static inline int SwapAB(int id) {          (file scope, right after the GpButton enum + int g_buttonSwap)
da2_console.cpp:3539  SendGamepadEvent(SwapAB(m.id), down);        (in PollPadAndDispatch)
da2_console.cpp:4097  g_buttonSwap = GetPrivateProfileIntW(L"DA2", L"Button swap", 0, ini);   (in ApplyConsoleVersion)
```

Find them again after a merge with:
`grep -n "SwapAB\|Button swap" da2_console.cpp da2_console.ini`

Tag `da2-tested-20261101` marks the state Juan verified in game.

## Case A — the repo gets a new upstream version (easy, minutes)

```sh
cd /e/RE/Projects/DA2NativeGamepadSupport
git fetch origin
git switch -c rebase-$(date +%Y%m%d) origin/main   # or master, whichever upstream uses
git cherry-pick 493a3f3                            # 9551319 is build-only; re-apply if he never ships a working build script
MSYS2_ARG_CONV_EXCL="*" cmd /c "E:\RE\Projects\DA2NativeGamepadSupport\build-all.cmd"
```

Conflict risk is limited to the three anchors above. If upstream edits the button
table or `ApplyConsoleVersion()` around them, resolve by hand: keep his logic, re-add
the `SwapAB()` wrapper on the single `SendGamepadEvent` call and the one ini read.
`build-all.cmd` only breaks if he renames `build.bat` or adds new source deps.

Deploy: copy the new `dinput8.dll` to `bin_ship\`, keep `da2_console.ini`
(`Console version=2`, `Button swap=1`). Rollback = restore
`known-good\dinput8-81faf3ff-tested.dll`.

## Case B — the GAME exe gets a new build (the real work, hours)

Our twist is irrelevant here. This mod is anchored to **absolute addresses in
today's `DragonAge2.exe`**: ~40 `constexpr uintptr_t ADDR_*` / `VFT_*` constants at
`da2_console.cpp` lines 17-128 and 1136+ (`0x005ABEA0`, `0x00D59220`, `0x009058A0`,
`0x00BE13FC`, ...). `R(a) = a + g_delta` only compensates for ASLR load bias —
it does **not** detect a different build. A re-patched exe would make hooks land on
the wrong code: crash or undefined behaviour, **not** a clean refusal to load.
There is no byte-pattern signature scan and no exe version/size guard.

Anchored build (recorded 2026-11-02, this is what must match):

```
DragonAge2.exe      size 12,172,664   sha256 478357e492859402618e2225df25d6d78a8105ea6f30f548c19602483474ed0a
DragonAge2Config.exe size 3,657,488   sha256 d8d47781507b04d51a04d6dab2a87bb94790a6fd32987230258b520477bfb465
```

Re-anchoring procedure if that hash ever changes:
1. Open the new exe in Ghidra, re-locate each anchor by following the same callers
   the original offsets were found from (XInput rumble `XInputSetState` for the input
   layer, Scaleform/GUI singleton for the UI layer, the dormant console
   `ReportGamepadAction` dispatcher `0x009058A0` for event injection).
2. Update the `ADDR_*` constants; keep `PREFERRED_BASE = 0x00400000` only if the new
   exe still has no ASLR-relevant base change (verify `SizeOfImage` + image base).
3. Rebuild, retest menu navigation, then gameplay.
4. Re-record the exe fingerprint here and redeploy.

Cheap insurance not yet implemented: a fail-safe guard in `ApplyConsoleVersion()`
that compares `SizeOfImage` (plus a few byte patterns at the hook targets) against
this fingerprint and refuses to install hooks on mismatch. ~10 lines; would turn a
future crash into "mod silently inactive". Ask before rebuilding — the deployed DLL
is the one Juan tested.

## Known-good artifacts

- `known-good/dinput8-81faf3ff-tested.dll` — sha256 `81faf3ff4be57f8f1b735900823bdd783da30e075eafbede897ffc92289717ac`, 235,008 B, the exact DLL live in `bin_ship` and confirmed working in game 2026-11-01
- `known-good/da2_console-tested.ini`
- Note: the working tree's `dinput8.dll` (`a02b4e00…`) is a later rebuild from identical
  source (MSVC is not byte-deterministic). Do not swap it in over the tested build
  without retesting.
