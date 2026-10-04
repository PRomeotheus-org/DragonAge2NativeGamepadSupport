// Dragon Age II -- console controls and console UI for the PC build.
// Release build: no diagnostic output. See the development source for why any
// of this is shaped the way it is.

#include <windows.h>
#include <shlobj.h>
#include <xinput.h>
#include <cstdio>
#include <cstdint>
#include <cstdarg>
#include <cmath>
#include <cstring>
#include "MinHook.h"

#pragma comment(lib, "XInput9_1_0.lib")

constexpr uintptr_t PREFERRED_BASE = 0x00400000;

constexpr uintptr_t ADDR_MOVEMENT_READER   = 0x005ABEA0;
constexpr uintptr_t ADDR_SCALEFORM_MGR_PTR = 0x00D59220;
constexpr uintptr_t PLATFORM_FIELD_OFFSET  = 0x370;
constexpr uintptr_t ADDR_APP_SINGLETON_PTR = 0x00D54494;
constexpr uintptr_t OFF_GUISYSTEM          = 0x158;

constexpr uintptr_t ADDR_OPEN_SCREEN       = 0x00654DE0;
constexpr uintptr_t ADDR_CLOSE_SCREEN      = 0x0064FE90;

constexpr uintptr_t ADDR_CTOR_NEXT_CHAR    = 0x0041C740;
constexpr uintptr_t ADDR_CTOR_PREV_CHAR    = 0x0041C890;
constexpr uintptr_t ADDR_GET_TARGET_MGR    = 0x005A6C90;
constexpr uintptr_t ADDR_DISPATCH          = 0x009058A0;

constexpr uintptr_t ADDR_HOVERED_OBJ_ID    = 0x00D1D578;

constexpr uintptr_t ADDR_HOVER_OVERRIDE    = 0x00D4FBA8;

constexpr uintptr_t ADDR_HOVER_UPDATE      = 0x004A01C0;

constexpr uintptr_t ADDR_CURSOR_POS        = 0x007AC2E0;

constexpr uintptr_t VFT_GAMEMODE_EXPLORE   = 0x00BE13FC;
constexpr uintptr_t VFT_GAMEMODE_COMBAT    = 0x00BEEE44;

constexpr uintptr_t ADDR_DIRSEARCH         = 0x006BDA10;
constexpr uintptr_t ADDR_DIRSEARCH_CTX     = 0x00D4FBC8;
constexpr uintptr_t ADDR_DIRSEARCH_RESULT  = 0x00D4FBDC;

constexpr uintptr_t ADDR_EXPLORE_PRIMARY   = 0x00496310;
constexpr uintptr_t ADDR_EXPLORE_SECONDARY = 0x00496360;
constexpr uintptr_t ADDR_OBJ_MANAGER_PTR   = 0x00D54494;
constexpr uintptr_t ADDR_RESOLVE_OBJ       = 0x005A70C0;
constexpr uintptr_t ADDR_PRIMARY_ACTION_OF = 0x00459D20;
constexpr uintptr_t ADDR_DO_ACTION         = 0x0048D190;

constexpr uintptr_t ADDR_CURRENT_TARGET  = 0x00D1D550;
constexpr uintptr_t ADDR_ARMED_ABILITY   = 0x00D4FCE0;
constexpr uintptr_t ADDR_TARGET_MODE     = 0x00D1D570;

constexpr uintptr_t ADDR_FRIENDLY_OBJ    = 0x00D4FB78;
constexpr uintptr_t ADDR_FRIENDLY_TGT_ID = 0x00D4FB84;
constexpr uintptr_t VF_SET_FRIENDLY_TGT  = 0x58;

constexpr DWORD ALLY_SELECT_MS = 8000;

constexpr uintptr_t OFF_GUI_DIALOG_OPEN  = 0x2A2;
constexpr uintptr_t OFF_GUI_POPUP_COUNT  = 0x2C8;

constexpr uintptr_t ADDR_PICKER_PICK      = 0x00497140;
constexpr uintptr_t ADDR_PICKER_VFTABLE   = 0x00BEAFFC;
constexpr uintptr_t OFF_PICK_BEGIN        = 0xBC;
constexpr uintptr_t OFF_PICK_END          = 0xC0;
constexpr uintptr_t OFF_PICK_ACTIVE       = 0xD8;
constexpr uintptr_t OFF_PICK_LOCKED       = 0xD9;
constexpr uintptr_t OFF_PICK_BLOCKED      = 0x1F0;
constexpr size_t    PICK_ENTRY_STRIDE     = 0x48;
constexpr uintptr_t OFF_ENTRY_STATE       = 0x08;
constexpr uintptr_t OFF_ENTRY_LOCKED      = 0x10;

constexpr uintptr_t ADDR_ALLOC           = 0x007AD9C0;
constexpr uintptr_t ADDR_BUILD_WSTRING   = 0x008E16B0;
constexpr uintptr_t ADDR_CONSTRUCT_PANEL = 0x00778930;
constexpr uintptr_t ADDR_REGISTER_PANEL  = 0x0077FA60;
constexpr uintptr_t ADDR_CTOR_INVOKE_CB  = 0x00426CA0;

constexpr uintptr_t ADDR_MSG_FROM_NAME   = 0x00905940;
constexpr uintptr_t ADDR_RESOLVE_MOVIE   = 0x0077A460;
constexpr uintptr_t ADDR_GUI_SINGLETON_PTR = 0x00D5922C;
constexpr uintptr_t ADDR_STR_REPORT_GP   = 0x00C0E3BC;
constexpr uintptr_t OFF_MOVIE_REGISTRY   = 0x164;
constexpr uintptr_t OFF_ENTRY_MOVIEVIEW  = 0x24;

enum GpButton : int {
    GP_None = 0,
    GP_DPadUp = 1, GP_DPadDown = 2, GP_DPadLeft = 3, GP_DPadRight = 4,
    GP_Start = 5, GP_Back = 6,
    GP_LeftThumb = 7, GP_RightThumb = 8,
    GP_LeftShoulder = 9, GP_RightShoulder = 10,
    GP_BottomAction = 11, GP_RightAction = 12, GP_LeftAction = 13, GP_TopAction = 14,
    GP_LeftThumbUp = 15, GP_LeftThumbDown = 16,
    GP_LeftThumbLeft = 17, GP_LeftThumbRight = 18,

    GP_RightThumbUp = 23, GP_RightThumbDown = 24,
    GP_RightThumbLeft = 25, GP_RightThumbRight = 26,
    GP_LeftTrigger = 31, GP_RightTrigger = 32,
};
constexpr int GP_EVENT_PRESSED  = 0x10000;
constexpr int GP_EVENT_RELEASED = 0x20000;

constexpr int  GUI_QUICKBAR    = 0x29;
constexpr int  GUI_RADIAL      = 0x40;
constexpr int  RADIAL_TYPE     = 1;
constexpr size_t PANEL_SIZE = 0xE8, INVOKE_MSG_SIZE = 0x60, ECSTRING_SIZE = 0x1C;

static const wchar_t* T_RADIAL = L"RadialQuickbar";
static const wchar_t* T_HUD    = L"CombinedHUD";

constexpr uintptr_t ADDR_ACTION_ANALOG  = 0x007AC220;
constexpr uintptr_t ADDR_ACTION_DIGITAL = 0x007ABFD0;

constexpr uint32_t ACT_CAMERA_YAW    = 7;
constexpr uint32_t ACT_CAMERA_PITCH  = 10;
constexpr uint32_t ACT_CAM_ZOOM      = 8;
constexpr uint32_t ACT_CAM_ZOOM_IN   = 25;
constexpr uint32_t ACT_CAM_ZOOM_OUT  = 26;

constexpr float ZOOM_STEP = 1.0f;

constexpr uint32_t ACT_CURSOR_POSITION   = 0x31;
constexpr uint32_t ACT_OBJECT_GLOW       = 0x32;
constexpr uint32_t ACT_HP_FLOATIES       = 0x33;
constexpr uint32_t ACT_HP_FLOATIES_F     = 0x34;
constexpr uint32_t ACT_HP_FLOATIES_F2    = 0x35;

constexpr uint32_t ACT_GUI_LEFT_CLICK = 0x3E;

constexpr uint32_t ACT_PARTYSEL_MODE = 0x42;
constexpr uint32_t ACT_PARTYSEL_X    = 0x43;
constexpr uint32_t ACT_PARTYSEL_Y    = 0x44;
constexpr uint32_t ACT_GUI_LT        = 0x57;
constexpr uint32_t ACT_GUI_RT        = 0x58;
constexpr uint32_t ACT_GUI_THUMB_LX  = 0x59;
constexpr uint32_t ACT_GUI_THUMB_LY  = 0x5A;
constexpr uint32_t ACT_GUI_THUMB_RX  = 0x5B;
constexpr uint32_t ACT_GUI_THUMB_RY  = 0x5C;

constexpr uint32_t ACT_AUTOTARGET_X     = 0x49;
constexpr uint32_t ACT_AUTOTARGET_Y     = 0x4A;
constexpr uint32_t ACT_ENABLE_AUTOTARGET = 0x50;
constexpr uint32_t ACT_LINEAR_TGT_MODE  = 0x51;
constexpr uint32_t ACT_LINEAR_TGT_X     = 0x52;
constexpr uint32_t ACT_LINEAR_TGT_Y     = 0x53;

constexpr uint32_t ACT_FILTER_NEUTRAL   = 0x4B;
constexpr uint32_t ACT_FILTER_PLAYER    = 0x4C;
constexpr uint32_t ACT_FILTER_PARTY     = 0x4D;
constexpr uint32_t ACT_FILTER_HOSTILE   = 0x4E;
constexpr uint32_t ACT_FILTER_OBJECTS   = 0x4F;
constexpr uint32_t ACT_GUI_RY        = ACT_GUI_THUMB_RY;

constexpr float CAMERA_SENS_YAW   = 1.0f;
constexpr float CAMERA_SENS_PITCH = 1.0f;
constexpr bool  CAMERA_INVERT_YAW   = false;
constexpr bool  CAMERA_INVERT_PITCH = false;

constexpr BYTE  TRIGGER_THRESHOLD = 64;
constexpr float STICK_DEADZONE = 0.20f;
constexpr float CAM_DEADZONE   = 0.18f;

static uintptr_t g_delta = 0;
static inline uintptr_t R(uintptr_t a) { return a + g_delta; }

static void Log(const char* fmt, ...) {
    char buf[512];
    va_list a; va_start(a, fmt);
    vsprintf_s(buf, fmt, a); va_end(a);
    OutputDebugStringA(buf);
}

#define Log(...)               ((void)0)
#define OutputDebugStringA(s)  ((void)(s))

static volatile LONG g_lastGameplayMs     = 0;
static volatile LONG g_lastGuiMs          = 0;

static volatile LONG g_lastMenuDeliveryMs = 0;

static void* AppSingleton() {
    __try { return *reinterpret_cast<void**>(R(ADDR_APP_SINGLETON_PTR)); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
}

static void* GuiSystem() {
    void* app = AppSingleton();
    if (!app) return nullptr;
    __try { return *reinterpret_cast<void**>((uintptr_t)app + OFF_GUISYSTEM); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
}

static bool ReadPad(XINPUT_STATE& st) {
    for (DWORD i = 0; i < 4; ++i)
        if (XInputGetState(i, &st) == ERROR_SUCCESS) return true;
    return false;
}

constexpr bool kSpoofPlatform = true;

static DWORD WINAPI SpoofThread(LPVOID) {
    if (!kSpoofPlatform) {
        Log("[DA2] platform spoof DISABLED (bisect) -- UI will look like PC\n");
        return 0;
    }
    uintptr_t slot = R(ADDR_SCALEFORM_MGR_PTR);
    void* inst = nullptr;
    while (!inst) { inst = *reinterpret_cast<void**>(slot); if (!inst) Sleep(1); }

    uintptr_t f = reinterpret_cast<uintptr_t>(inst) + PLATFORM_FIELD_OFFSET;
    wchar_t* s = reinterpret_cast<wchar_t*>(f);
    s[0] = L'P'; s[1] = L'S'; s[2] = L'3'; s[3] = L'\0';
    *reinterpret_cast<uint32_t*>(f + 0x10) = 3;
    Log("[DA2] platform -> PS3 (ScaleformManager=0x%08X)\n", (unsigned)(uintptr_t)inst);
    return 0;
}

typedef char (__fastcall* OpenScreen_t)(void*, void*, int);
static OpenScreen_t oOpenScreen = nullptr;
static volatile LONG g_blockQuickbar = 1;

static volatile LONG g_lastScreenOpen = -1;

constexpr uintptr_t OFF_SCREEN_MOVIENAME = 0xA8;
typedef void* (__thiscall* GetScreenByType_t)(void*, int);

constexpr int GUI_LOGIN      = 39;
constexpr int GUI_LOGIN_HOST = 59;

constexpr bool g_blockLogin = true;
static volatile LONG g_loginCloseAt = 0;
static volatile LONG g_loginRecoverAt = 0;
static volatile LONG g_probeScreen1At = 0;
static volatile LONG g_screen1Live    = 0;

static char __fastcall hkOpenScreen(void* thiz, void* edx, int typeId) {
    if (typeId == GUI_QUICKBAR && InterlockedCompareExchange(&g_blockQuickbar, 1, 1))
        return 0;
    char r = oOpenScreen(thiz, edx, typeId);
    if (typeId != GUI_QUICKBAR) {
        InterlockedExchange(&g_lastScreenOpen, typeId);
        Log("[DA2] screen OPEN  type=%d (0x%X)\n", typeId, typeId);
    }
    if (typeId == 1)
        InterlockedExchange(&g_probeScreen1At, (LONG)(GetTickCount() + 300));
    if (typeId == GUI_LOGIN && g_blockLogin) {
        InterlockedExchange(&g_loginCloseAt, (LONG)(GetTickCount() + 120));
        Log("[DA2] login screen (type 39) opened -- closing it\n");
    }
    return r;
}

typedef char (__thiscall* CloseScreen_t)(void*, int, int);

static void* ResolveMovieView(const wchar_t* name);

static void PollLoginBlock() {
    if (!g_blockLogin) return;

    LONG at = InterlockedCompareExchange(&g_loginCloseAt, 0, 0);
    if (at && (LONG)(GetTickCount() - (DWORD)at) >= 0) {
        InterlockedExchange(&g_loginCloseAt, 0);
        void* gui = GuiSystem();
        if (gui) {
            __try {
                char r = reinterpret_cast<CloseScreen_t>(R(ADDR_CLOSE_SCREEN))(gui, GUI_LOGIN, 0);
                Log("[DA2] login close -> %d\n", (int)r);

                void** vt = *reinterpret_cast<void***>(gui);
                auto get = reinterpret_cast<GetScreenByType_t>(vt[0x6C / 4]);
                void* leftover = get(gui, GUI_LOGIN_HOST);
                if (leftover) {
                    char r2 = reinterpret_cast<CloseScreen_t>(R(ADDR_CLOSE_SCREEN))(
                                  gui, GUI_LOGIN_HOST, 0);
                    Log("[DA2] login host (type %d) still open -> close %d\n",
                        GUI_LOGIN_HOST, (int)r2);
                } else {
                    Log("[DA2] login host (type %d) already gone\n", GUI_LOGIN_HOST);
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) {
                Log("[DA2] login close faulted\n");
            }
        }
        InterlockedExchange(&g_loginRecoverAt, (LONG)(GetTickCount() + 400));
        return;
    }

    LONG rec = InterlockedCompareExchange(&g_loginRecoverAt, 0, 0);
    if (rec && (LONG)(GetTickCount() - (DWORD)rec) >= 0) {
        InterlockedExchange(&g_loginRecoverAt, 0);
        const bool back = ResolveMovieView(L"startmenu") || ResolveMovieView(L"StartMenu");
        Log("[DA2] login closed -- start menu %s\n",
            back ? "is back" : "NOT resolvable yet (observation only)");
    }
}

static void CloseQuickbarOnce() {
    void* gui = GuiSystem();
    if (!gui) return;
    __try {
        char r = reinterpret_cast<CloseScreen_t>(R(ADDR_CLOSE_SCREEN))(gui, GUI_QUICKBAR, 0);
        Log("[DA2] quickbar close -> %d\n", (int)r);
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
}

typedef void* (__fastcall* MsgCtor_t)(void* thisPtr);
typedef uintptr_t (__fastcall* GetTargetMgr_t)(void* thisPtr, void* edx);
typedef uint8_t (__cdecl* Dispatch_t)(void* msg, uintptr_t target);

static void FirePartyCycle(bool next) {
    void* app = AppSingleton();
    if (!app) return;
    uintptr_t msg = 0;
    reinterpret_cast<MsgCtor_t>(R(next ? ADDR_CTOR_NEXT_CHAR : ADDR_CTOR_PREV_CHAR))(&msg);
    uintptr_t mgr = reinterpret_cast<GetTargetMgr_t>(R(ADDR_GET_TARGET_MGR))(app, nullptr);
    if (!mgr) return;
    reinterpret_cast<Dispatch_t>(R(ADDR_DISPATCH))(&msg, mgr + 0x18);
}

typedef void*   (__cdecl*    Alloc_t)(size_t, uint32_t);
typedef void*   (__fastcall* WBuild_t)(void*, void*, const wchar_t*);
typedef void*   (__fastcall* Construct_t)(void*, void*, void*, void*, int, int, int);
typedef void    (__fastcall* RegisterP_t)(void*, void*, int, void*);
typedef void*   (__fastcall* InvokeCtor_t)(void*, void*, void*, void*, void*);

static void* g_radialPanel   = nullptr;
static bool  g_radialVisible = false;
static bool  g_showPending   = false;

static void OpenRadialScreen();

static volatile LONG g_eatFaceGameplay = 0;

static int InvokeAS2(const wchar_t* target, const wchar_t* method,
                     const wchar_t* args = L"") {
    void* gui = GuiSystem();
    if (!gui) return -1;
    __try {
        uint8_t sT[ECSTRING_SIZE] = {}, sM[ECSTRING_SIZE] = {}, sE[ECSTRING_SIZE] = {};
        auto b = reinterpret_cast<WBuild_t>(R(ADDR_BUILD_WSTRING));
        b(sT, nullptr, target);
        b(sM, nullptr, method);
        b(sE, nullptr, args);

        uint8_t msg[INVOKE_MSG_SIZE] = {};
        reinterpret_cast<InvokeCtor_t>(R(ADDR_CTOR_INVOKE_CB))(msg, nullptr, sT, sM, sE);
        return reinterpret_cast<Dispatch_t>(R(ADDR_DISPATCH))(msg, (uintptr_t)gui);
    }
    __except (EXCEPTION_EXECUTE_HANDLER) { return -1; }
}

static bool EnsureRadialPanel() {
    if (g_radialPanel) return true;
    void* gui = GuiSystem();
    if (!gui) return false;
    __try {
        void* mem = reinterpret_cast<Alloc_t>(R(ADDR_ALLOC))(PANEL_SIZE, 0);
        if (!mem) return false;
        uint8_t n1[ECSTRING_SIZE] = {}, n2[ECSTRING_SIZE] = {};
        auto b = reinterpret_cast<WBuild_t>(R(ADDR_BUILD_WSTRING));
        b(n1, nullptr, L"RadialQuickbar");
        b(n2, nullptr, T_RADIAL);
        void* p = reinterpret_cast<Construct_t>(R(ADDR_CONSTRUCT_PANEL))(
            mem, nullptr, n1, n2, RADIAL_TYPE, 0, 0);
        if (!p) return false;
        reinterpret_cast<RegisterP_t>(R(ADDR_REGISTER_PANEL))(gui, nullptr, GUI_RADIAL, p);
        g_radialPanel = p;
        Log("[DA2] radial panel registered @0x%08X\n", (unsigned)(uintptr_t)p);
        OpenRadialScreen();
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static void OpenRadialScreen() {
    void* gui = GuiSystem();
    if (!gui) return;
    __try {
        if (oOpenScreen) oOpenScreen(gui, nullptr, GUI_RADIAL);
        else reinterpret_cast<OpenScreen_t>(R(ADDR_OPEN_SCREEN))(gui, nullptr, GUI_RADIAL);
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static void RadialShowRequest() {
    if (!EnsureRadialPanel()) return;
    g_showPending = true;
}

static void RadialTick() {
    if (!g_showPending || g_radialVisible) return;
    if (InvokeAS2(T_RADIAL, L"RadialQuickbarScene.ShowRadialQuickbar") == 1) {
        InvokeAS2(T_HUD, L"BattleMenu.Fade");
        g_radialVisible = true;
        g_showPending   = false;
    } else {
        OpenRadialScreen();
    }
}

static void RadialHide() {
    g_showPending = false;
    if (!g_radialPanel || !g_radialVisible) return;
    InvokeAS2(T_RADIAL, L"RadialQuickbarScene.HideRadialQuickbar");
    InvokeAS2(T_HUD,    L"BattleMenu.Unfade");
    InvokeAS2(T_HUD,    L"BattleMenu.MoveToBottomCorner");
    g_radialVisible = false;
    OpenRadialScreen();
}

static void RadialDoAction() {
    if (!g_radialVisible) return;
    InvokeAS2(T_RADIAL, L"RadialQuickbarScene.DoAction");
}

static void BattleMenuPage(bool second) {
    InvokeAS2(T_HUD, L"BattleMenu.TogglePage", second ? L"2" : L"1");
}

struct GFxValue {
    void*    pObjectInterface;
    uint32_t type;
    double   num;
};

typedef void* (__fastcall* ResolveMovie_t)(void*, void*, void*);
typedef void  (__fastcall* GuiLock_t)(void*, void*);
typedef int   (__fastcall* Invoke5C_t)(void*, void*, const char*, void*, void*, int);

static const wchar_t* kGuiMovies[] = {

    L"abilities", L"Abilities", L"achievementbrowser", L"AchievementBrowser",
    L"armycontrol", L"ArmyControl", L"audiogui", L"battlemenu",
    L"BattleMenu", L"bookback", L"bookfront", L"chanters",
    L"characterrecord", L"CharacterRecord", L"chargen", L"CharGen",
    L"chargen_stage2", L"CharGenStage2", L"ChargenStage2",
    L"CharGen_Stage2", L"Chargen_Stage2", L"chargenstage2",
    L"CharGenStage_2", L"CharacterGeneration", L"charactergeneration",
    L"combinedhud", L"CombinedHUD", L"container",
    L"Container", L"controllerlayout", L"conversation", L"Conversation",
    L"crafting", L"Crafting", L"deathscreen", L"DeathScreen",
    L"dialoguepop", L"DialoguePop", L"floatylayer", L"FloatyLayer",
    L"guiachievementunlocked", L"inventory", L"Inventory", L"itemupgrade",
    L"ItemUpgrade", L"journal", L"Journal", L"mainmenu",
    L"MainMenu", L"map", L"Map", L"minimap",
    L"MiniMap", L"newcontentavailable", L"notifications", L"Notifications",
    L"optionsmenu", L"OptionsMenu", L"partypicker", L"PartyPicker",
    L"pausemenu", L"PauseMenu", L"popuplayer", L"PopupLayer",
    L"portraits", L"Portraits", L"prccontentmanager", L"PRCContentManager",
    L"prestartmenu", L"PreStartMenu", L"purchaseconfirm", L"quickbar", L"Quickbar",
    L"radialquickbar", L"RadialQuickbar", L"saveload", L"SaveLoad",
    L"savescreen", L"SaveScreen", L"startmenu", L"StartMenu",
    L"startmenu2", L"store", L"Store", L"tactics",
    L"Tactics", L"tooltips", L"Tooltips", L"worldmap",
    L"WorldMap",
};

typedef void*   (__cdecl* MsgFromName_t)(void*);

static uintptr_t GameplayTarget() {
    void* app = AppSingleton();
    if (!app) return 0;
    __try {
        uintptr_t mgr = reinterpret_cast<GetTargetMgr_t>(R(ADDR_GET_TARGET_MGR))(app, nullptr);
        return mgr ? mgr + 0x18 : 0;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}

static uint32_t CurrentTarget() {
    __try { return *reinterpret_cast<uint32_t*>(R(ADDR_CURRENT_TARGET)); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 0xFFFFFFFF; }
}
static uint32_t ArmedAbility() {
    __try { return *reinterpret_cast<uint32_t*>(R(ADDR_ARMED_ABILITY)); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}

static uint32_t ArmedTargetMask() {
    __try { return *reinterpret_cast<uint32_t*>(R(0x00D4FCEC)); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}

static void SetFriendlyCastTarget(uint32_t id) {
    __try {
        void*  obj = reinterpret_cast<void*>(R(ADDR_FRIENDLY_OBJ));
        void** vt  = *reinterpret_cast<void***>(obj);
        if (!vt) return;
        reinterpret_cast<void(__fastcall*)(void*, void*, uint32_t)>(
            vt[VF_SET_FRIENDLY_TGT / sizeof(void*)])(obj, nullptr, id);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Log("[DA2] SetFriendlyCastTarget(%08X) faulted\n", id);
    }
}

typedef void* (__fastcall* ResolveObject_t)(void*, void*, uint32_t);
typedef char  (__stdcall*  IsLegalTarget_t)(void*, uint32_t);

static bool TargetLegalForArmedAbility(uint32_t id) {
    if (id == 0xFFFFFFFF) return false;
    __try {
        void* app = AppSingleton();
        if (!app) return false;
        void* obj = reinterpret_cast<ResolveObject_t>(R(0x005A70C0))(app, nullptr, id);
        if (!obj) return false;
        const uint32_t mask = *reinterpret_cast<uint32_t*>(R(0x00D4FCEC));
        return reinterpret_cast<IsLegalTarget_t>(R(0x0045A350))(obj, mask) != 0;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static uint32_t FriendlyCastTarget() {
    __try { return *reinterpret_cast<uint32_t*>(R(ADDR_FRIENDLY_TGT_ID)); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 0xFFFFFFFF; }
}

static uint32_t g_lastTarget = 0xFFFFFFFF;

static void TargetSnapshot(const char* tag) {
    uint32_t t = CurrentTarget();
    if (t == g_lastTarget) return;
    Log("[DA2] target %08X -> %08X  (%s)\n", g_lastTarget, t, tag);
    g_lastTarget = t;
}

static const wchar_t* kProbeNames[] = {
    L"SelectNextHostileTargetMessage",
    L"SelectPreviousHostileTargetMessage",
    L"SelectNextUsableObjectMessage",
    L"SelectPreviousUsableObjectMessage",

    L"SelectForwardTargetableObjectMessage",
    L"SelectBackTargetableObjectMessage",
    L"SelectLeftTargetableObjectMessage",
    L"SelectRightTargetableObjectMessage",
    L"ClearCurrentTargetMessage",
    L"PerformDefaultActionOnTargetMessage",
    L"PerformSecondaryActionOnTargetMessage",
    L"PerformUseObjectMessage",
    L"UsePlotActionMessage",
    L"PerformAbilityOnSelectedTargetMessage",
    L"ToggleTargetLockMessage",
    L"FriendlyCastTargetChangedUpMessage",
    L"FriendlyCastTargetChangedDownMessage",
    L"PerformFriendlyCastMessage",
    L"CancelFriendlyCastMessage",
    L"EnableObjectGlowMessage",
    L"ShowAllFloatiesMessage",
    L"HideAllFloatiesMessage",
    L"SkipMovieMessage",
    L"SkipConversationMessage",
    L"SelectResponseMessage",
    L"DialogBoxClosedMessage",
    L"EasyDialogBoxClosedMessage",
    L"PreStartContinueMessage",
    L"CancelAbilityTargetingMessage",

    L"MoveToPointMessage",
    L"FireAOEAbilityMessage",
    L"ChargenConfirmMessage",
    L"ChargenDeselectMessage",
    L"SelectPartyMemberUnderCursorMessage",

    L"PerformPrimaryActionOnTargetUnderCursorMessage",
    L"PerformSecondaryActionOnTargetUnderCursorMessage",
};

static bool g_canPause = false;
static bool g_useMemoryPause = false;

static void ProbeMessageNames() {
    auto b = reinterpret_cast<WBuild_t>(R(ADDR_BUILD_WSTRING));
    int found = 0;
    for (size_t i = 0; i < _countof(kProbeNames); ++i) {
        __try {
            uint8_t s[ECSTRING_SIZE] = {};
            b(s, nullptr, kProbeNames[i]);
            void* m = reinterpret_cast<MsgFromName_t>(R(ADDR_MSG_FROM_NAME))(s);
            if (m) {
                ++found;
                void* vt = *reinterpret_cast<void**>(m);
                typedef void(__fastcall* Rel_t)(void*, void*, int);
                void* rel = *reinterpret_cast<void**>((uintptr_t)vt + 0x10);
                if (rel) reinterpret_cast<Rel_t>(rel)(m, nullptr, 1);
            } else {

                Log("[DA2] probe: %S  NOT REGISTERED (check spelling)\n", kProbeNames[i]);
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    Log("[DA2] probe: %d/%d bound message names verified\n",
        found, (int)_countof(kProbeNames));

    auto reg = [&](const wchar_t* n) -> bool {
        __try {
            uint8_t s2[ECSTRING_SIZE] = {};
            b(s2, nullptr, n);
            void* m = reinterpret_cast<MsgFromName_t>(R(ADDR_MSG_FROM_NAME))(s2);
            if (!m) return false;
            void* vt = *reinterpret_cast<void**>(m);
            typedef void(__fastcall* Rel_t)(void*, void*, int);
            void* rel = *reinterpret_cast<void**>((uintptr_t)vt + 0x10);
            if (rel) reinterpret_cast<Rel_t>(rel)(m, nullptr, 1);
            return true;
        } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
    };
    const bool p  = reg(L"PauseMessage"),       u  = reg(L"UnpauseMessage");
    const bool mp = reg(L"MemoryPauseMessage"), mu = reg(L"MemoryUnpauseMessage");

    g_useMemoryPause = mp && mu;
    g_canPause = g_useMemoryPause || (p && u);
    Log("[DA2] pause: Pause=%d Unpause=%d MemoryPause=%d MemoryUnpause=%d -> %s\n",
        p?1:0, u?1:0, mp?1:0, mu?1:0,
        !g_canPause ? "disabled (one-way pause would strand the game)"
                    : (g_useMemoryPause ? "ENABLED (save/restore bracket)"
                                        : "ENABLED (plain, no save/restore)"));
}

static bool SendCommand(const wchar_t* command) {
    __try {
        uint8_t s[ECSTRING_SIZE] = {};
        reinterpret_cast<WBuild_t>(R(ADDR_BUILD_WSTRING))(s, nullptr, command);

        void* msg = reinterpret_cast<MsgFromName_t>(R(ADDR_MSG_FROM_NAME))(s);
        if (!msg) {
            Log("[DA2] command \"%S\" -> factory NULL (not registered by name)\n", command);
            return false;
        }
        void* vt = *reinterpret_cast<void**>(msg);
        auto dispatch = reinterpret_cast<Dispatch_t>(R(ADDR_DISPATCH));

        struct Cand { const char* name; uintptr_t t; };
        Cand cands[8]; int nc = 0;
        void* app = AppSingleton();
        __try {
            if (app) {
                uintptr_t gm = *reinterpret_cast<uintptr_t*>((uintptr_t)app + 0x144);
                if (gm) {
                    cands[nc++] = { "gameMode", gm };
                    uintptr_t a = *reinterpret_cast<uintptr_t*>(gm + 0x9C);
                    if (a) cands[nc++] = { "gameMode+0x9C", a };
                    uintptr_t c = *reinterpret_cast<uintptr_t*>(gm + 0x88);
                    if (c) cands[nc++] = { "gameMode+0x88", c };
                }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        uintptr_t play = GameplayTarget();
        if (play) cands[nc++] = { "partyTarget", play };
        void* gui = GuiSystem();
        if (gui) cands[nc++] = { "gui", (uintptr_t)gui };
        if (app) cands[nc++] = { "app", (uintptr_t)app };

        uint8_t r = 0;
        const char* who = "none";
        for (int i = 0; i < nc && !r; ++i) {
            __try { r = dispatch(msg, cands[i].t); }
            __except (EXCEPTION_EXECUTE_HANDLER) { r = 0; }
            if (r) who = cands[i].name;
        }

        static const wchar_t* s_lastCmd = nullptr;
        static const char*    s_lastWho = nullptr;
        static uint8_t        s_lastR   = 0xFF;
        if (command != s_lastCmd || who != s_lastWho || r != s_lastR || !r) {
            Log("[DA2] command \"%S\" -> %d (%s, tried %d)\n", command, (int)r, who, nc);
            s_lastCmd = command; s_lastWho = who; s_lastR = r;
        }

        if (wcsstr(command, L"Target")) {
            char tag[96];
            _snprintf_s(tag, sizeof(tag), _TRUNCATE, "%S", command);
            TargetSnapshot(tag);
        }

        typedef void(__fastcall* Rel_t)(void*, void*, int);
        void* rel = *reinterpret_cast<void**>((uintptr_t)vt + 0x10);
        if (rel) reinterpret_cast<Rel_t>(rel)(msg, nullptr, 1);
        return r != 0;
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Log("[DA2] command \"%S\" EXCEPTION\n", command);
        return false;
    }
}

static volatile LONG g_manualTarget = 0xFFFFFFFF;
static float         g_manualYawX = 0.0f;
static float         g_manualYawY = 0.0f;
static bool          g_manualYawValid = false;

constexpr float MANUAL_RELEASE_DOT = 0.819f;

static bool CameraForward2D(float* outX, float* outY);

static void LatchManualTarget(uint32_t id) {
    InterlockedExchange(&g_manualTarget, (LONG)id);
    g_manualYawValid = CameraForward2D(&g_manualYawX, &g_manualYawY);
    Log("[DA2] target latched by hand -> %08X (auto-aim will not move it)\n", id);
}

static void ClearManualTarget(const char* why) {
    if ((uint32_t)InterlockedExchange(&g_manualTarget, (LONG)0xFFFFFFFF) != 0xFFFFFFFF)
        Log("[DA2] manual target released (%s)\n", why);
    g_manualYawValid = false;
}

static bool CycleTarget(const wchar_t* msg, const char* label) {
    const uint32_t before = CurrentTarget();
    SendCommand(msg);
    if (CurrentTarget() != before) { TargetSnapshot(label); return true; }

    SendCommand(L"ClearCurrentTargetMessage");
    SendCommand(msg);
    const bool moved = CurrentTarget() != before;
    Log("[DA2] %S: clamped at end, wrapped -> %s\n", msg,
        moved ? "new target" : "NO CANDIDATES");
    TargetSnapshot(label);
    return moved;
}

static bool CycleTargetManual(const wchar_t* msg, const char* label) {
    const bool moved = CycleTarget(msg, label);
    if (moved) {
        const uint32_t now = CurrentTarget();
        if (now != 0xFFFFFFFF) LatchManualTarget(now);
    }
    return moved;
}

static volatile LONG g_dialogNeedRelease = 0;

static int InvokeRawAS(const wchar_t* movie, const char* method);

typedef char (__thiscall* ShowDialog_t)(void*, void*, void*);
static ShowDialog_t oShowDialog = nullptr;

static char __fastcall hkShowDialog(void* thiz, void* edx, void* msg) {
    __try {
        Log("[DA2] ShowDialogBox HANDLER: this=%08X guiSystem=%08X %s\n",
            (unsigned)(uintptr_t)thiz, (unsigned)(uintptr_t)GuiSystem(),
            (thiz == GuiSystem()) ? "(match)" : "(MISMATCH -- offsets read the wrong object)");
    } __except (EXCEPTION_EXECUTE_HANDLER) {}

    InterlockedExchange(&g_dialogNeedRelease, 1);

    {
        static bool refreshed = false;
        if (!refreshed) {
            refreshed = true;
            const int r0 = InvokeRawAS(L"PopupLayer", "GUISystem.StartSceneFromGame");
            Log("[DA2] popup: re-read platform via StartSceneFromGame -> %d\n", r0);
        }
    }

    char r = oShowDialog(thiz, edx, msg);
    __try {
        Log("[DA2] ShowDialogBox AFTER: 2A2=%d 2C8=%d\n",
            *reinterpret_cast<uint8_t*>((uintptr_t)thiz + OFF_GUI_DIALOG_OPEN),
            *reinterpret_cast<uint32_t*>((uintptr_t)thiz + OFF_GUI_POPUP_COUNT));
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return r;
}

typedef char (__fastcall* InvokeAS_t)(void*, void*, const char*, void*, void*, int);
static InvokeAS_t oInvokeAS = nullptr;

constexpr uintptr_t GFXVALUE_SIZE = 0x10;
constexpr uintptr_t OFF_GFXVALUE_TYPE  = 0x04;
constexpr uintptr_t OFF_GFXVALUE_VALUE = 0x08;

static bool LooksLikeWideText(const wchar_t* p) {
    if (!p) return false;
    __try {
        for (int i = 0; i < 6; ++i) {
            const wchar_t c = p[i];
            if (c == 0) return i > 0;
            if (c < 0x09 || c > 0x2122) return false;
        }
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static const wchar_t* GfxArgWide(const void* args, int i) {
    __try {
        const uintptr_t v = (uintptr_t)args + (uintptr_t)i * GFXVALUE_SIZE;
        const wchar_t* direct = *reinterpret_cast<const wchar_t* const*>(v + OFF_GFXVALUE_VALUE);
        if (LooksLikeWideText(direct)) return direct;
        const wchar_t* const* indirect =
            *reinterpret_cast<const wchar_t* const* const*>(v + OFF_GFXVALUE_VALUE);
        if (indirect && LooksLikeWideText(*indirect)) return *indirect;
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return nullptr;
}

static bool GfxArgNumber(const void* args, int i, double* out) {
    __try {
        const uintptr_t v = (uintptr_t)args + (uintptr_t)i * GFXVALUE_SIZE;
        if ((*reinterpret_cast<const uint32_t*>(v + OFF_GFXVALUE_TYPE) & 0x0F) != 3) return false;
        *out = *reinterpret_cast<const double*>(v + OFF_GFXVALUE_VALUE);
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static uint32_t GfxArgType(const void* args, int i) {
    __try {
        return *reinterpret_cast<const uint32_t*>(
            (uintptr_t)args + (uintptr_t)i * GFXVALUE_SIZE + OFF_GFXVALUE_TYPE);
    } __except (EXCEPTION_EXECUTE_HANDLER) { return 0xFFFFFFFF; }
}

static void LogWideChunked(const char* label, const wchar_t* s) {
    if (!s) { Log("[DA2]   %s <null>\n", label); return; }
    __try {
        static char utf8[4096];
        const int n = WideCharToMultiByte(CP_UTF8, 0, s, -1, utf8, (int)sizeof(utf8), nullptr, nullptr);
        if (n <= 0) { Log("[DA2]   %s <unconvertible>\n", label); return; }
        const int len = n - 1;
        if (len <= 300) { Log("[DA2]   %s \"%s\"\n", label, utf8); return; }
        Log("[DA2]   %s (%d chars, split)\n", label, len);
        for (int off = 0; off < len; off += 300) {
            char part[320];
            int take = len - off; if (take > 300) take = 300;
            memcpy(part, utf8 + off, (size_t)take);
            part[take] = '\0';
            Log("[DA2]     | %s\n", part);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Log("[DA2]   %s <faulted while reading>\n", label);
    }
}

static void LogTutorialInvoke(const void* args, int argc) {
    if (!args || argc <= 0) { Log("[DA2] TUTORIAL: argc=%d, no args\n", argc); return; }
    Log("[DA2] TUTORIAL: argc=%d\n", argc);
    for (int i = 0; i < argc && i < 8; ++i) {
        double num = 0.0;
        const uint32_t ty = GfxArgType(args, i);
        if (GfxArgNumber(args, i, &num)) {
            Log("[DA2]   arg%d type=%u NUMBER  %.0f\n", i, ty, num);
        } else {
            const wchar_t* s = GfxArgWide(args, i);
            if (s) {
                char label[32];
                sprintf_s(label, "arg%d type=%u TEXT", i, ty);
                LogWideChunked(label, s);
            } else {
                Log("[DA2]   arg%d type=%u <unreadable>\n", i, ty);
            }
        }
    }
}

struct TutorialSwap { const wchar_t* pc; const wchar_t* console; };

static const TutorialSwap kTutorialSwaps[] = {

    { L"<RightClickDefaultCapped/> an enemy to attack.",
      L"Press <A/> to attack your target." },
    { L"<RightClickDefaultCapped/> on a target to attack.",
      L"Press <A/> to attack your target." },

    { L"<LeftClickDefaultCapped/> on a tree to zoom.",
      L"Use <theleftstick/> to select a tree, then <A/> to zoom." },

    { L"<LeftClickDefaultCapped/> a party member's portrait to take control of the character or use <SelectChar1/>, <SelectChar2/>, <SelectChar3/>, and <SelectChar4/>.",
      L"Use <LB/> or <RB/> to control another party member." },
    { L"To take control of different party members, simply <LeftClickDefault/> on their portraits or <LeftClickDefault/> on the characters themselves. To select multiple party members, hold Shift while selecting them.",
      L"Use <LB/> or <RB/> to control another party member." },

    { L"Drag abilities to your quickbar to make them easily accessible. During combat, you can only reorganize icons if you pause the game by pressing <PauseKey/>.",
      L"Hold <RT/> to access an additional three slots on your battle menu." },

    { L"You have received a potion. To use a potion to replenish a party member's health, stamina, or mana, click on the icons on the right side of the quickbar.",
      L"You have received a potion. To use a potion to replenish a party member's health, stamina, or mana, open the radial menu with <LT/>." },

    { L"To change maps, select Kirkwall - Night or The Free Marches. Click on a destination to go to that area.",
      L"Use <theleftstick/> to select your destination and press <A/>. Selecting Kirkwall - Night or The Free Marches will switch to those maps." },

    { L"Abilities are accessed from the quickbar at the bottom of the screen.",
      L"Abilities are accessed from the battle menu at the bottom of the screen. Hold <RT/> for three more slots." },

    { L"<RightClickDefaultCapped/> on a foe to begin basic attacks. To use a talent like Mighty Blow or a spell like Winter's Grasp, first target the enemy, then select the ability from your quickbar.",
      L"Press <A/> to begin basic attacks. To use a talent like Mighty Blow or a spell like Winter's Grasp, target the enemy first, then choose the ability from your battle menu." },

    { L"You have entered targeting mode. Use the mouse to aim the spell or talent and <LeftClickDefault/>.",
      L"You have entered targeting mode. Use <theleftstick/> to aim the spell or talent, then press <A/>." },

    { L"<LeftClickDefaultCapped/> to target this ability on a party member.",
      L"Press <A/> to target this ability on a party member." },

    { L"Press <PauseKey/> to pause the game.",
      L"Pull <LT/> to open the radial menu, which pauses the game." },
    { L"Press the spacebar to pause the game and issue orders to your party members.",
      L"Pull <LT/> to open the radial menu. The game pauses while it is open, so you can issue orders to your party." },

    { L"Tough battles require that you control each party member in turn. Pause the game by pressing <PauseKey/>, give the current character an order, then switch to the next character. Repeat until the whole party has an order queued up, then unpause.",
      L"Tough battles require that you control each party member in turn. Pull <LT/> to pause, give the current character an order, then use <LB/> or <RB/> to switch. Repeat until the whole party has an order queued up." },

    { L"Ordering the entire party to attack the same opponent is easy. First, press <PartySelectKey/> to select everyone at once. Then <RightClickDefault/> on the desired enemy.",
      L"To order the whole party to attack one opponent, target the enemy and press <A/>. Your party will follow the order you give." },

    { L"Only rogues can detect and disarm traps. While controlling a rogue, <RightClickDefault/> a detected trap to disarm it. Lock and trap difficulties progress from simple to standard to complex to master. Defeating each of these requires 10, 20, 30, and 40 points of cunning, respectively.",
      L"Only rogues can detect and disarm traps. While controlling a rogue, press <A/> on a detected trap to disarm it. Lock and trap difficulties progress from simple to standard to complex to master. Defeating each of these requires 10, 20, 30, and 40 points of cunning, respectively." },

    { L"To use potions, select Quick Heal, Quick Mana, or Quick Stamina on the right side of the quickbar.",
      L"To use potions, pull <LT/> to open the radial menu and choose Quick Heal, Quick Mana, or Quick Stamina." },
    { L"For easy access to potions, click on Quick Heal or Quick Mana/Stamina on the right side of the quickbar. You can also drag usable items into the quickbar as if they were spells or talents.",
      L"For easy access to potions, pull <LT/> to open the radial menu and choose Quick Heal or Quick Mana/Stamina." },

    { L"The quickbar has a limited number of slots. Assign the abilities you use the most.",
      L"The battle menu has a limited number of slots. Assign the abilities you use the most, and hold <RT/> to reach the second set." },

    { L"You now have more abilities than slots on your quickbar. Drag any abilities you don't use often out of the quickbar and replace them with others from the abilities screen.",
      L"You now have more abilities than slots on your battle menu. Press <Start/> and open the abilities screen to choose which ones to assign." },

    { L"You can reorganize the icons in the quickbar at any time except during active combat. During a battle, you must first pause the game by pressing <PauseKey/>.",
      L"You can reorganize your battle menu at any time except during active combat. In a battle, pull <LT/> to pause first." },

    { L"To map a talent or spell to your quickbar, drag the icon from the abilities screen to the desired slot in the quickbar.",
      L"To assign a talent or spell to your battle menu, press <Start/>, open the abilities screen and choose the slot you want." },

    { L"The first ten slots in the quickbar at the bottom of the screen are mapped to the keys <QuickSlot1Key/>, <QuickSlot2Key/>, <QuickSlot3Key/>, <QuickSlot4Key/>, <QuickSlot5Key/>, <QuickSlot6Key/>, <QuickSlot7Key/>, <QuickSlot8Key/>, <QuickSlot9Key/>, and <QuickSlot10Key/>.",
      L"The battle menu at the bottom of the screen holds three slots. Hold <RT/> to reach three more." },

    { L"To equip new weapons, armor, and accessories, double-click or drag-and-drop the item. Right-clicking an item moves it to the Junk category to be quickly sold at stores. Drag an item to the edge of the screen to destroy it.",
      L"To equip new weapons, armor and accessories, select the item and press <A/>. Items marked as Junk can be sold quickly at stores." },

    { L"Did playing with your options menu settings get a little out of hand? <LeftClickDefaultCapped/> on \"Restore Defaults\" in the options menu.",
      L"Did playing with your options menu settings get a little out of hand? Choose \"Restore Defaults\" in the options menu." },

    { L"Mighty Blow inflicts devastating damage against a target and other nearby enemies. <LeftClickDefaultCapped/> Mighty Blow in the quickbar to use it now. You must <LeftClickDefault/> an enemy if you don't have a target selected.",
      L"Mighty Blow inflicts devastating damage against a target and other nearby enemies. Choose Mighty Blow from your battle menu to use it now. Press <A/> on an enemy first if you do not have a target selected." },
    { L"Backstab instantly moves you behind your target to deal massive damage. <LeftClickDefaultCapped/> Backstab in the quickbar to use it now. You must <LeftClickDefault/> an enemy if you don't have a target selected.",
      L"Backstab instantly moves you behind your target to deal massive damage. Choose Backstab from your battle menu to use it now. Press <A/> on an enemy first if you do not have a target selected." },
    { L"Winter's Grasp damages and slows your target with a bitter blast. <LeftClickDefaultCapped/> Winter's Grasp in the quickbar to use it now. You must <LeftClickDefault/> an enemy if you don't have a target selected.",
      L"Winter's Grasp damages and slows your target with a bitter blast. Choose Winter's Grasp from your battle menu to use it now. Press <A/> on an enemy first if you do not have a target selected." },
    { L"Fireball is an explosive attack that inflicts widespread fire damage. <LeftClickDefaultCapped/> Fireball in the quickbar to use it now. ",
      L"Fireball is an explosive attack that inflicts widespread fire damage. Choose Fireball from your battle menu to use it now. " },
    { L"Miasmic Flask stuns targets in a small area. <LeftClickDefaultCapped/> Miasmic Flask in the quickbar to use it now.",
      L"Miasmic Flask stuns targets in a small area. Choose Miasmic Flask from your battle menu to use it now." },

    { L"You can order potions if you discover the right combination of resources, along with a recipe. Resources you've discovered are permanently available to craftsmen, but placing an order costs money. To use a potion you've ordered, press <InventoryKey/> to open the inventory, then drag it from the Usable Items tab into your quickbar.",
      L"You can order potions if you discover the right combination of resources, along with a recipe. Resources you've discovered are permanently available to craftsmen, but placing an order costs money. To use a potion you've ordered, press <Start/> and select \"Inventory\", then assign it from the Usable Items tab." },
    { L"You can order poisons and bombs if you discover the right combination of resources, along with a recipe. Resources you've discovered are permanently available to craftsmen, but placing an order costs money. To apply poison to melee weapons or throw a bomb at enemies, press <InventoryKey/> to open the inventory, then drag the item from the Usable Items tab into your quickbar.",
      L"You can order poisons and bombs if you discover the right combination of resources, along with a recipe. Resources you've discovered are permanently available to craftsmen, but placing an order costs money. To apply poison to melee weapons or throw a bomb at enemies, press <Start/> and select \"Inventory\", then assign the item from the Usable Items tab." },
};

struct GlyphTag { const wchar_t* tag; const wchar_t* tex; };

static const GlyphTag kGlyphTags[] = {
    { L"<A/>",                   L"PS3Gamepad_BottomAction.dds" },
    { L"<B/>",                   L"PS3Gamepad_RightAction.dds"  },
    { L"<X/>",                   L"PS3Gamepad_LeftAction.dds"   },
    { L"<Y/>",                   L"PS3Gamepad_TopAction.dds"    },
    { L"<LB/>",                  L"PS3Gamepad_LeftBumper.dds"   },
    { L"<RB/>",                  L"PS3Gamepad_RightBumper.dds"  },
    { L"<LT/>",                  L"PS3Gamepad_LeftTrigger.dds"  },
    { L"<RT/>",                  L"PS3Gamepad_RightTrigger.dds" },
    { L"<Start/>",               L"PS3Gamepad_Start.dds"        },
    { L"<Back/>",                L"PS3Gamepad_Select.dds"       },
    { L"<theDpad/>",             L"PS3Gamepad_DPad.dds"         },
    { L"<theleftstick/>",        L"PS3Gamepad_LeftStick.dds"    },
    { L"<therightstick/>",       L"PS3Gamepad_RightStick.dds"   },
    { L"<theleftstickbutton/>",  L"PS3Gamepad_LeftStickPush.dds"  },
    { L"<therightstickbutton/>", L"PS3Gamepad_RightStickPush.dds" },
};

static wchar_t g_glyphBuf[2][1024];
static int     g_glyphBufNext = 0;

static const wchar_t* ExpandGlyphTags(const wchar_t* s) {
    bool any = false;
    for (const GlyphTag& g : kGlyphTags) if (wcsstr(s, g.tag)) { any = true; break; }
    if (!any) return s;

    wchar_t* out = g_glyphBuf[g_glyphBufNext];
    g_glyphBufNext = (g_glyphBufNext + 1) % 2;
    size_t o = 0;
    const size_t cap = 1024 - 1;

    for (const wchar_t* p = s; *p && o < cap; ) {
        const GlyphTag* hit = nullptr;
        for (const GlyphTag& g : kGlyphTags) {
            const size_t n = wcslen(g.tag);
            if (wcsncmp(p, g.tag, n) == 0) { hit = &g; break; }
        }
        if (hit) {

            const int w = _snwprintf_s(out + o, cap - o, _TRUNCATE,
                                       L"<img src='img://%s' align='baseline' vspace='-8'>",
                                       hit->tex);
            if (w < 0) break;
            o += (size_t)w;
            p += wcslen(hit->tag);
        } else {
            out[o++] = *p++;
        }
    }
    out[o] = L'\0';
    return out;
}

static bool TutorialTextEqual(const wchar_t* a, const wchar_t* b) {
    while (*a == L' ' || *a == L'\t' || *a == L'\n' || *a == L'\r') ++a;
    while (*b == L' ' || *b == L'\t' || *b == L'\n' || *b == L'\r') ++b;
    size_t la = wcslen(a), lb = wcslen(b);
    while (la && (a[la-1] == L' ' || a[la-1] == L'\t' || a[la-1] == L'\n' || a[la-1] == L'\r')) --la;
    while (lb && (b[lb-1] == L' ' || b[lb-1] == L'\t' || b[lb-1] == L'\n' || b[lb-1] == L'\r')) --lb;
    return la == lb && wcsncmp(a, b, la) == 0;
}

static bool TrySwapTutorialArg(void* args, int i) {
    __try {
        const uintptr_t v = (uintptr_t)args + (uintptr_t)i * GFXVALUE_SIZE;
        const uint32_t ty = *reinterpret_cast<const uint32_t*>(v + OFF_GFXVALUE_TYPE);
        if ((ty & 0x0F) != 5) return false;
        if (ty & 0x40) {
            Log("[DA2] tutorial: arg%d is a MANAGED string (type=%u), left alone\n", i, ty);
            return false;
        }
        const wchar_t** slot = reinterpret_cast<const wchar_t**>(v + OFF_GFXVALUE_VALUE);
        const wchar_t* cur = *slot;
        if (!LooksLikeWideText(cur)) return false;
        for (const TutorialSwap& s : kTutorialSwaps) {
            if (TutorialTextEqual(cur, s.pc)) {
                const wchar_t* text = ExpandGlyphTags(s.console);
                *slot = text;
                Log("[DA2] tutorial SWAPPED arg%d -> console wording%s\n", i,
                    text == s.console ? "" : " (+glyph <img> tags)");
                return true;
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return false;
}

static bool TipLooksLike(const wchar_t* pc, const wchar_t* tip) {
    const wchar_t* bestP = nullptr;
    size_t bestN = 0;
    for (const wchar_t* p = pc; *p; ) {
        if (*p == L'<') {
            while (*p && *p != L'>') ++p;
            if (*p) ++p;
            continue;
        }
        const wchar_t* start = p;
        while (*p && *p != L'<') ++p;
        const wchar_t* end = p;
        while (start < end && (*start == L' ' || *start == L'\n' || *start == L'\r')) ++start;
        while (end > start && (end[-1] == L' ' || end[-1] == L'\n' || end[-1] == L'\r')) --end;
        const size_t n = (size_t)(end - start);
        if (n > bestN) { bestN = n; bestP = start; }
    }
    if (bestN < 18) return false;
    for (const wchar_t* q = tip; *q; ++q)
        if (wcsncmp(q, bestP, bestN) == 0) return true;
    return false;
}

static bool TrySwapLoadingTip(void* args, int i) {
    __try {
        const uintptr_t v = (uintptr_t)args + (uintptr_t)i * GFXVALUE_SIZE;
        const uint32_t ty = *reinterpret_cast<const uint32_t*>(v + OFF_GFXVALUE_TYPE);
        if ((ty & 0x0F) != 5) return false;
        if (ty & 0x40) return false;
        const wchar_t** slot = reinterpret_cast<const wchar_t**>(v + OFF_GFXVALUE_VALUE);
        const wchar_t* cur = *slot;
        if (!LooksLikeWideText(cur)) return false;
        for (const TutorialSwap& s : kTutorialSwaps) {
            if (TutorialTextEqual(cur, s.pc) || TipLooksLike(s.pc, cur)) {
                *slot = ExpandGlyphTags(s.console);
                Log("[DA2] loading tip SWAPPED -> console wording\n");
                return true;
            }
        }
        LogWideChunked("tip NOT MAPPED:", cur);
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return false;
}

static char __fastcall hkInvokeAS(void* thiz, void* edx, const char* method,
                                  void* ret, void* args, int argc) {
    __try {

        if (method && InterlockedCompareExchange(&g_screen1Live, 0, 0)) {
            static char seen[64][64];
            static int sn = 0;
            bool isNew = true;
            for (int k = 0; k < sn; ++k)
                if (strcmp(seen[k], method) == 0) { isNew = false; break; }
            if (isNew && sn < 64) {
                strncpy_s(seen[sn], method, _TRUNCATE);
                ++sn;
                Log("[DA2] CHARGEN AS2: %s\n", method);
            }
        }
        if (method && strstr(method, "DisplayNewLoadingText") && args) {
            for (int i = 0; i < argc && i < 4; ++i)
                if (TrySwapLoadingTip(args, i)) break;
        }
        if (method && strstr(method, "DisplayTutorial")) {
            Log("[DA2] TUTORIAL invoke: \"%s\"\n", method);

            LogTutorialInvoke(args, argc);

            bool swapped = false;
            if (args) {
                for (int i = 0; i < argc && i < 8; ++i)
                    if (TrySwapTutorialArg(args, i)) swapped = true;
                if (!swapped && argc >= 2) {
                    const wchar_t* body = GfxArgWide(args, 1);
                    if (body && wcslen(body) > 8)
                        LogWideChunked("tutorial NOT MAPPED:", body);
                }
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return oInvokeAS(thiz, edx, method, ret, args, argc);
}

static Dispatch_t oDispatch = nullptr;

static const char* MessageTypeName(void* msg) {
    __try {
        void** vt = *reinterpret_cast<void***>(msg);
        if (!vt) return nullptr;
        void*  col = vt[-1];
        if (!col) return nullptr;
        void*  td  = reinterpret_cast<void**>(col)[3];
        if (!td) return nullptr;
        return reinterpret_cast<const char*>(td) + 8;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
}

static volatile LONG g_aoeTargeting = 0;

constexpr uintptr_t ADDR_AOE_SHAPE    = 0x00D4FD04;
constexpr uintptr_t ADDR_AOE_INVALID  = 0x00D4F9E2;
constexpr uintptr_t ADDR_AOE_MOVEFLAG = 0x00D4F9DB;

static uint32_t AoeShape() {
    __try { return *reinterpret_cast<uint32_t*>(R(ADDR_AOE_SHAPE)); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}
static bool AoeReticuleInvalid() {
    __try { return *reinterpret_cast<uint8_t*>(R(ADDR_AOE_INVALID)) != 0; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}
static bool AoeIsMoveToPoint() {
    __try { return *reinterpret_cast<uint8_t*>(R(ADDR_AOE_MOVEFLAG)) != 0; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

constexpr uintptr_t ADDR_AOE_RETICULE = 0x00D4FD10;
constexpr uintptr_t ADDR_GET_PLAYER   = 0x005A7220;
constexpr uintptr_t OFF_OBJ_POS       = 0x160;
constexpr uintptr_t OFF_OBJ_FACING    = 0x170;

typedef void* (__fastcall* GetPlayerObj_t)(void*, void*);
static bool ReadLeftStick(const XINPUT_STATE& st, float& ox, float& oy);

static void LogReticule() {
    static float px = 0, py = 0, pz = 0;
    __try {
        const float* r = reinterpret_cast<const float*>(R(ADDR_AOE_RETICULE));
        const float dx = r[0] - px, dy = r[1] - py, dz = r[2] - pz;
        if (dx*dx + dy*dy + dz*dz < 0.01f) return;
        px = r[0]; py = r[1]; pz = r[2];
        Log("[DA2] reticule -> %.2f %.2f %.2f   invalid=%d\n",
            r[0], r[1], r[2], AoeReticuleInvalid() ? 1 : 0);
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static volatile LONG g_centreSuppressUntil = 0;

static volatile LONG g_eatMoveUntilCentred = 0;

static bool IsRadialCommitMessage(const char* n) {
    if (!n) return false;
    static const char* kCommit[] = {
        "UseBestPotionMessage",
        "PerformAbilityMessage",
        "PerformAbilityOnSelectedTargetMessage",
        "PerformAbilityOnPlayerMessage",
        "MoveToPointMessage",
        "ToggleHoldPartyMessage",
        "RadialDefaultMessage",
        "CancelActionsMessage",
    };
    for (const char* c : kCommit)
        if (strstr(n, c)) return true;
    return false;
}

static void RumbleForMessage(const char* name);

static uint8_t __cdecl hkDispatch(void* msg, uintptr_t target) {
    const char* name = nullptr;
    __try { name = MessageTypeName(msg); }
    __except (EXCEPTION_EXECUTE_HANDLER) { name = nullptr; }

    RumbleForMessage(name);

    if (name && InterlockedCompareExchange(&g_screen1Live, 0, 0)) {
        static const char* seen[64] = {};
        static int sn = 0;
        bool isNew = true;
        for (int k = 0; k < sn; ++k)
            if (seen[k] == name || strcmp(seen[k], name) == 0) { isNew = false; break; }
        if (isNew && sn < (int)_countof(seen)) {
            seen[sn++] = name;
            Log("[DA2] CHARGEN MSG: %s\n", name);
        }
    }

    static volatile LONG s_inHide = 0;
    if (g_radialVisible && IsRadialCommitMessage(name)
        && InterlockedCompareExchange(&s_inHide, 1, 0) == 0) {
        Log("[DA2] radial: \"%s\" committed -- closing wheel\n", name);
        __try { RadialHide(); } __except (EXCEPTION_EXECUTE_HANDLER) {}
        InterlockedExchange(&g_eatFaceGameplay, 1);
        InterlockedExchange(&s_inHide, 0);
    }

    return oDispatch(msg, target);
}

typedef uint8_t (__fastcall* ExploreAct_t)(void* self, void* edx, void* msg);
static ExploreAct_t oExplorePrimary   = nullptr;
static ExploreAct_t oExploreSecondary = nullptr;

static volatile LONG g_primaryRan = 0;

static uint8_t __fastcall hkExplorePrimary(void* self, void* edx, void* msg) {
    const uint8_t r = oExplorePrimary(self, edx, msg);
    if (r) InterlockedExchange(&g_primaryRan, (LONG)GetTickCount());
    return r;
}

static uint8_t __fastcall hkExploreSecondary(void* self, void* edx, void* msg) {
    return oExploreSecondary(self, edx, msg);
}

static bool g_autoAim = true;

static bool g_centreRay = false;

static HWND GameWindow() {
    static HWND cached = nullptr;
    if (!cached || !IsWindow(cached)) {
        cached = nullptr;
        struct Ctx { DWORD pid; HWND best; LONG area; } ctx = { GetCurrentProcessId(), nullptr, 0 };
        EnumWindows([](HWND h, LPARAM p) -> BOOL {
            Ctx* c = reinterpret_cast<Ctx*>(p);
            DWORD pid = 0; GetWindowThreadProcessId(h, &pid);
            if (pid != c->pid || !IsWindowVisible(h)) return TRUE;
            RECT r{}; if (!GetClientRect(h, &r)) return TRUE;
            const LONG a = (r.right - r.left) * (r.bottom - r.top);
            if (a > c->area) { c->area = a; c->best = h; }
            return TRUE;
        }, reinterpret_cast<LPARAM>(&ctx));
        cached = ctx.best;
    }
    return cached;
}

static bool GameClientCentre(int* cx, int* cy) {
    HWND cached = GameWindow();
    if (!cached) return false;
    RECT r{};
    if (!GetClientRect(cached, &r)) return false;
    const LONG w = r.right - r.left, h = r.bottom - r.top;
    if (w <= 0 || h <= 0) return false;
    *cx = (int)(w / 2); *cy = (int)(h / 2);
    return true;
}

typedef void (__fastcall* CursorPos_t)(void* thiz, void* edx, int32_t* out, uint32_t* handle);
static CursorPos_t oCursorPos = nullptr;

constexpr bool kCursorHide       = true;
constexpr bool kCursorPark       = true;
constexpr bool kCursorCentreFeed = false;

static volatile LONG g_clientCx = 0;
static volatile LONG g_clientCy = 0;

static void __fastcall hkCursorPos(void* thiz, void* edx, int32_t* out, uint32_t* handle) {
    oCursorPos(thiz, edx, out, handle);
    if (!kCursorCentreFeed) return;
    if (!g_centreRay || !out || !handle) return;

    if (g_radialVisible) return;

    if (InterlockedCompareExchange(&g_aoeTargeting, 0, 0)) return;
    {
        const LONG until = InterlockedCompareExchange(&g_centreSuppressUntil, 0, 0);
        if (until && (LONG)GetTickCount() - until < 0) return;
    }
    __try {
        if (handle[0] != ACT_CURSOR_POSITION) return;

        const int cx = (int)InterlockedCompareExchange(&g_clientCx, 0, 0);
        const int cy = (int)InterlockedCompareExchange(&g_clientCy, 0, 0);
        if (cx <= 0 || cy <= 0) return;

        static bool said = false;
        if (!said) {
            said = true;
            Log("[DA2] auto-aim: cursor was %d,%d -> feeding centre %d,%d\n",
                out[0], out[1], cx, cy);
        }
        out[0] = cx; out[1] = cy;
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static void* ResolveMovieView(const wchar_t* name);

static bool OptionsMouseActive() {

    return ResolveMovieView(L"OptionsMenu") != nullptr;
}

typedef HCURSOR (WINAPI* SetCursorFn)(HCURSOR);
static SetCursorFn oSetCursorApi = nullptr;
static bool g_hideCursor = true;

static bool CursorShouldHide() {
    if (!kCursorHide) return false;
    if (!g_hideCursor) return false;

    if (InterlockedCompareExchange(&g_screen1Live, 0, 0)) return false;
    HWND fg = GetForegroundWindow();
    if (fg && fg != GameWindow()) return false;
    return true;
}

static HCURSOR WINAPI hkSetCursor(HCURSOR h) {
    if (CursorShouldHide()) return oSetCursorApi(nullptr);
    return oSetCursorApi(h);
}

static void PollCursorHide() {
    if (!oSetCursorApi) return;
    const bool hide = CursorShouldHide();
    static int last = -1;
    if ((int)hide == last) return;
    last = (int)hide;

    if (hide) oSetCursorApi(nullptr);
    else      oSetCursorApi(LoadCursorA(nullptr, IDC_ARROW));
    Log("[DA2] cursor %s\n", hide ? "hidden" : "shown (options menu or alt-tab)");
}

struct PadCache {
    volatile LONG  valid;
    volatile float lx, ly, rx, ry;
    volatile float lt, rt;
    volatile LONG  aDown;
    volatile LONG  dpadUp, dpadDown, dpadLeft, dpadRight;
    volatile LONG  zoomIn, zoomOut;
};
static PadCache g_pad = {};

enum { PARK_OFF = 0, PARK_BR, PARK_BL, PARK_TR, PARK_TL, PARK_CENTER };

constexpr int g_cursorPark = PARK_BR;

static void PollCursorPark() {
    if (!kCursorPark) return;
    if (g_cursorPark == PARK_OFF) return;
    if (!CursorShouldHide()) return;
    HWND hw = GameWindow();
    if (!hw) return;
    RECT rc{};
    if (!GetClientRect(hw, &rc)) return;
    const int w = rc.right - rc.left, h = rc.bottom - rc.top;
    if (w <= 2 || h <= 2) return;

    POINT p{};

    const bool convLive = ResolveMovieView(L"conversation") != nullptr;
    {
        static bool prev = false;
        if (convLive != prev) {
            prev = convLive;
            Log("[DA2] wheel: conversation %s\n", convLive ? "open" : "closed");
        }
    }
    if (convLive) {
        const float x = g_pad.lx, y = g_pad.ly;
        static int lastOct = -2;
        static bool wasLive = false;
        if (!wasLive) lastOct = -2;
        wasLive = true;
        int oct = -1;
        if (x != 0.0f || y != 0.0f) {
            double a2 = atan2((double)y, (double)x) * 180.0 / 3.14159265358979;
            if (a2 < 0) a2 += 360.0;
            oct = (int)((a2 + 22.5) / 45.0) % 8;
        }

        const int prevOct = lastOct;
        lastOct = oct;
        if (oct >= 0 && oct != prevOct) {

            static const struct { double deg; int idx; } kOpt[] = {
                {  30.0, 0 },
                {   0.0, 4 },
                { -30.0, 1 },
                { 150.0, 2 },
                { 180.0, 5 },
                {-150.0, 3 },
            };
            const double deg = atan2((double)y, (double)x) * 180.0 / 3.14159265358979;
            int order[6] = { 0, 1, 2, 3, 4, 5 };
            double dist[6];
            for (int k = 0; k < 6; ++k) {
                double d = deg - kOpt[k].deg;
                while (d > 180.0)  d -= 360.0;
                while (d < -180.0) d += 360.0;
                dist[k] = d < 0 ? -d : d;
            }
            for (int a2 = 0; a2 < 6; ++a2)
                for (int b2 = a2 + 1; b2 < 6; ++b2)
                    if (dist[order[b2]] < dist[order[a2]]) {
                        const int tmp = order[a2]; order[a2] = order[b2]; order[b2] = tmp;
                    }

            wchar_t arg[16];
            for (int k = 0; k < 6; ++k) arg[k] = (wchar_t)(L'0' + kOpt[order[k]].idx);
            arg[6] = L'\0';
            const int r1 = InvokeAS2(L"conversation", L"Conversation.SetResponseIndex", arg);

            if (r1 <= 0) {
                static bool warned = false;
                if (!warned) { warned = true;
                    Log("[DA2] wheel: SetResponseIndex missing -- override not updated\n"); }
            }
        }
    }

    switch (g_cursorPark) {
        case PARK_BL:     p.x = 0;     p.y = h - 1; break;
        case PARK_TR:     p.x = w - 1; p.y = 0;     break;
        case PARK_TL:     p.x = 0;     p.y = 0;     break;
        case PARK_CENTER: p.x = w / 2; p.y = h / 2; break;
        default:          p.x = w - 1; p.y = h - 1; break;
    }
    if (!ClientToScreen(hw, &p)) return;

    POINT cur{};
    if (GetCursorPos(&cur) && cur.x == p.x && cur.y == p.y) return;
    SetCursorPos(p.x, p.y);
}

typedef DWORD (WINAPI* XInputSetStateFn)(DWORD, XINPUT_VIBRATION*);
static XInputSetStateFn oXInputSetState = nullptr;

static DWORD WINAPI hkXInputSetState(DWORD idx, XINPUT_VIBRATION* v) {
    static int logged = 0;
    if (logged < 20) {
        ++logged;
        Log("[DA2] ENGINE XInputSetState(pad=%lu, L=%u R=%u)\n",
            idx, v ? v->wLeftMotorSpeed : 0, v ? v->wRightMotorSpeed : 0);
    }
    return oXInputSetState(idx, v);
}

static DWORD g_rumbleUntil = 0;

static void RumblePulse(WORD left, WORD right, DWORD ms) {
    XINPUT_VIBRATION v{};
    v.wLeftMotorSpeed  = left;
    v.wRightMotorSpeed = right;
    if (XInputSetState(0, &v) != ERROR_SUCCESS) return;
    g_rumbleUntil = GetTickCount() + ms;
}

static void PollRumbleStop() {
    if (!g_rumbleUntil) return;
    if ((LONG)(GetTickCount() - g_rumbleUntil) < 0) return;
    g_rumbleUntil = 0;
    XINPUT_VIBRATION v{};
    XInputSetState(0, &v);
}

constexpr bool kRumbleEnabled = true;

static void RumbleForMessage(const char* name) {
    if (!kRumbleEnabled || !name) return;
    struct Trig { const char* needle; WORD l, r; DWORD ms; };
    static const Trig kTrig[] = {

        { "PerformPrimaryActionOnTargetUnderCursorMessage", 8000,  30000, 110 },
        { "PerformDefaultActionOnTargetMessage",            8000,  30000, 110 },

        { "PerformAbilityMessage",                          45000, 22000, 190 },
        { "PerformAbilityOnSelectedTargetMessage",          45000, 22000, 190 },
        { "PerformAbilityOnPlayerMessage",                  36000, 18000, 170 },

        { "FireAOEAbilityMessage",                          62000, 32000, 280 },
    };
    for (const auto& t : kTrig)
        if (strstr(name, t.needle)) { RumblePulse(t.l, t.r, t.ms); return; }
}

static uint32_t PlayerObjectId();
static bool     InExploreMode();
static void*    ResolveCamera();

constexpr bool kDamageRumble = true;

constexpr unsigned OFF_CRE_STATS_OWNER = 0x810;
constexpr unsigned OFF_STAT_CURRENT    = 0x2C;
constexpr unsigned OFF_STATS_BEGIN     = 0x200;
constexpr unsigned OFF_STATS_END       = 0x204;
constexpr int      STAT_INDEX_HEALTH   = 7;
constexpr int      STAT_INDEX_STAMINA  = 10;
constexpr int      STAT_MIN_ENTRIES    = 36;
constexpr int      STAT_MAX_ENTRIES    = 128;
constexpr unsigned STAT_DUMP_BYTES     = 0x40;
constexpr unsigned STAT_SEARCH_BYTES   = 0x1000;

static bool LooksHeap(uintptr_t p) {
    return p >= 0x10000000 && p < 0x80000000 && (p & 3) == 0;
}

static bool Readable(uintptr_t p, size_t len) {
    if (!p || !len) return false;
    MEMORY_BASIC_INFORMATION mbi{};
    if (!VirtualQuery(reinterpret_cast<void*>(p), &mbi, sizeof(mbi))) return false;
    if (mbi.State != MEM_COMMIT) return false;
    if (mbi.Protect & (PAGE_GUARD | PAGE_NOACCESS)) return false;
    const DWORD readable = PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY |
                           PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE |
                           PAGE_EXECUTE_WRITECOPY;
    if (!(mbi.Protect & readable)) return false;
    const uintptr_t regionEnd =
        reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
    return p + len <= regionEnd;
}

static void** TryStatsAt(uintptr_t obj, int* outCount) {
    if (!LooksHeap(obj)) return nullptr;
    if (!Readable(obj + OFF_STATS_BEGIN, 8)) return nullptr;
    const uintptr_t b = *reinterpret_cast<const uintptr_t*>(obj + OFF_STATS_BEGIN);
    const uintptr_t e = *reinterpret_cast<const uintptr_t*>(obj + OFF_STATS_END);
    if (!LooksHeap(b) || !LooksHeap(e) || e <= b) return nullptr;
    const uintptr_t bytes = e - b;
    if (bytes & 3) return nullptr;
    const int n = (int)(bytes / 4);
    if (n < STAT_MIN_ENTRIES || n > STAT_MAX_ENTRIES) return nullptr;
    if (!Readable(b, bytes)) return nullptr;
    void** v = reinterpret_cast<void**>(b);
    for (int i = 0; i < n; ++i)
        if (!LooksHeap(reinterpret_cast<uintptr_t>(v[i]))) return nullptr;
    *outCount = n;
    return v;
}

static void** FindStatsVector(uintptr_t cre, int* outCount, unsigned* outVia,
                              int* outTried) {
    *outTried = 0;
    void** v = TryStatsAt(cre, outCount);
    if (v) { *outVia = 0xFFFFFFFF; return v; }
    for (unsigned off = 0; off < STAT_SEARCH_BYTES; off += 4) {
        if (!Readable(cre + off, 4)) continue;
        const uintptr_t p = *reinterpret_cast<const uintptr_t*>(cre + off);
        if (!LooksHeap(p)) continue;
        ++(*outTried);
        v = TryStatsAt(p, outCount);
        if (v) { *outVia = off; return v; }
    }
    return nullptr;
}

static void PollHealthStat() {
    if (!kDamageRumble) return;

    static DWORD steadySince = 0, leftAt = 0;
    static bool  primed = false;
    static uint32_t prevWho = 0;
    static float prevHp = 0.0f, peakHp = 0.0f;

    if (!InExploreMode()) {
        if (!leftAt) leftAt = GetTickCount();
        if (GetTickCount() - leftAt > 1500) {
            steadySince = 0; primed = false; prevWho = 0;
        }
        return;
    }
    leftAt = 0;
    if (!steadySince) steadySince = GetTickCount();
    if (GetTickCount() - steadySince < 2000) return;

    static DWORD last = 0;
    const DWORD now = GetTickCount();
    if (now - last < 100) return;
    last = now;

    static int said = 0;
    #define HPSAY(...) do { if (said < 12) { ++said; Log(__VA_ARGS__); } } while (0)

    void* app = AppSingleton();
    if (!app) { HPSAY("[DA2] health stat: no app singleton\n"); return; }
    const uint32_t me = PlayerObjectId();
    if (me == 0 || me == 0xFFFFFFFF) {
        HPSAY("[DA2] health stat: player id is %08X\n", me);
        return;
    }

    void* obj = nullptr;
    __try {
        obj = reinterpret_cast<ResolveObject_t>(R(0x005A70C0))(app, nullptr, me);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        HPSAY("[DA2] health stat: resolve faulted for %08X\n", me);
        return;
    }
    if (!obj) { HPSAY("[DA2] health stat: %08X did not resolve\n", me); return; }

    const uintptr_t c = reinterpret_cast<uintptr_t>(obj);
    if (!Readable(c, STAT_SEARCH_BYTES)) {
        HPSAY("[DA2] health stat: creature @%08X not readable\n", (unsigned)c);
        return;
    }

    static unsigned via = OFF_CRE_STATS_OWNER;
    static bool viaKnown = true;
    static DWORD lastSearch = 0;

    int n = 0;
    void** stats = nullptr;

    if (viaKnown) {
        uintptr_t owner = 0;
        if (via == 0xFFFFFFFF) owner = c;
        else if (Readable(c + via, 4))
            owner = *reinterpret_cast<const uintptr_t*>(c + via);
        stats = TryStatsAt(owner, &n);
        if (!stats) viaKnown = false;
    }

    if (!stats) {
        if (now - lastSearch < 3000) return;
        lastSearch = now;
        int tried = 0;
        unsigned found = 0;
        stats = FindStatsVector(c, &n, &found, &tried);
        if (!stats) {
            HPSAY("[DA2] health stat: no stats vector in creature %08X @%08X"
                  " (%d candidates)\n", me, (unsigned)c, tried);
            return;
        }
        via = found;
        viaKnown = true;
        Log("[DA2] health stat: stats relocated to cre+%03X, %d entries\n", via, n);
    }

    void* hp = stats[STAT_INDEX_HEALTH];
    if (!Readable(reinterpret_cast<uintptr_t>(hp), STAT_DUMP_BYTES)) {
        HPSAY("[DA2] health stat: stats[%d] @%08X not readable\n",
              STAT_INDEX_HEALTH, (unsigned)(uintptr_t)hp);
        viaKnown = false;
        return;
    }

    float cur = 0.0f;
    memcpy(&cur, reinterpret_cast<const void*>(
               reinterpret_cast<uintptr_t>(hp) + OFF_STAT_CURRENT), sizeof(cur));
    if (!(cur == cur) || cur < -1.0e6f || cur > 1.0e6f) return;

    if (!primed || prevWho != me) {
        primed = true;
        prevWho = me;
        prevHp = peakHp = cur;
        Log("[DA2] damage rumble armed: Hawke %08X health %.1f\n", me, cur);
        return;
    }

    if (cur > peakHp) peakHp = cur;

    if (cur < prevHp - 0.05f) {
        const float drop = prevHp - cur;
        const float frac = (peakHp > 1.0f) ? drop / peakHp : 0.0f;

        float f = frac * 4.0f;
        if (f < 0.18f) f = 0.18f;
        if (f > 1.00f) f = 1.00f;
        const WORD lo = (WORD)(22000.0f + 43000.0f * f);
        const WORD hi = (WORD)(10000.0f + 26000.0f * f);
        const DWORD ms = (DWORD)(110.0f + 180.0f * f);
        RumblePulse(lo, hi, ms);
        Log("[DA2] hit: %.1f -> %.1f of %.0f (-%.0f%%)\n",
            prevHp, cur, peakHp, frac * 100.0f);
    }
    prevHp = cur;
    #undef HPSAY
}

static void PollCursorToggleKey() {
    static bool prev = false;
    const bool now = (GetAsyncKeyState(VK_F4) & 0x8000) != 0;
    if (now && !prev) {
        g_hideCursor = !g_hideCursor;
        Log("[DA2] cursor hiding %s (F4)\n", g_hideCursor ? "ON" : "OFF");
    }
    prev = now;
}

static volatile LONG g_optMouseHotMs = 0;

static bool OptionsCursorHot() {
    const LONG t = InterlockedCompareExchange(&g_optMouseHotMs, 0, 0);
    return t != 0 && (GetTickCount() - (DWORD)t) < 2000;
}

constexpr bool kOptionsMouseEnabled = false;

static void PollOptionsMouse() {
    if (!kOptionsMouseEnabled) return;

    static bool  active   = false;
    static bool  btnDown  = false;
    static bool  prevA    = false;
    static float fx = 0.0f, fy = 0.0f;
    static DWORD lastMs   = 0;

    const bool now = OptionsMouseActive();
    if (!now) {
        if (active) {

            if (btnDown) { mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); btnDown = false; }
            active = false;
            Log("[DA2] options mouse mode OFF\n");
        }
        prevA = false;
        return;
    }

    HWND hw = GameWindow();
    RECT rc{};
    if (!hw || !GetClientRect(hw, &rc) || rc.right <= 0 || rc.bottom <= 0) return;

    if (!active) {
        active = true;
        lastMs = GetTickCount();

        POINT p{};
        if (GetCursorPos(&p) && ScreenToClient(hw, &p)) {
            fx = (float)p.x; fy = (float)p.y;
        } else {
            fx = rc.right * 0.5f; fy = rc.bottom * 0.5f;
        }
        Log("[DA2] options mouse mode ON (right stick = cursor, A = click)\n");
    }

    XINPUT_STATE st{};
    if (XInputGetState(0, &st) != ERROR_SUCCESS) return;

    const DWORD nowMs = GetTickCount();
    float dt = (nowMs - lastMs) / 1000.0f;
    lastMs = nowMs;
    if (dt <= 0.0f || dt > 0.25f) dt = 0.016f;

    auto axis = [](SHORT v) -> float {
        const float dz = 8000.0f;
        float f = (float)v;
        if (f > -dz && f < dz) return 0.0f;
        f = (f > 0 ? f - dz : f + dz) / (32767.0f - dz);
        return f * f * (f < 0 ? -1.0f : 1.0f);
    };
    const float SPEED = 1100.0f;
    const float ax = axis(st.Gamepad.sThumbRX);
    const float ay = axis(st.Gamepad.sThumbRY);
    if (ax != 0.0f || ay != 0.0f) {
        InterlockedExchange(&g_optMouseHotMs, (LONG)GetTickCount());
        static bool saidHot = false;
        if (!saidHot) { saidHot = true; Log("[DA2] options cursor engaged (A = click)\n"); }
    }
    fx += ax * SPEED * dt;
    fy -= ay * SPEED * dt;

    if (fx < 0) fx = 0; if (fx > rc.right  - 1) fx = (float)(rc.right  - 1);
    if (fy < 0) fy = 0; if (fy > rc.bottom - 1) fy = (float)(rc.bottom - 1);

    POINT sp{ (LONG)fx, (LONG)fy };
    if (ClientToScreen(hw, &sp)) SetCursorPos(sp.x, sp.y);

    const bool a = (st.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
    const bool hot = OptionsCursorHot();
    if (a && !prevA && hot) { mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0); btnDown = true; }
    if (!a && prevA && btnDown) { mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0); btnDown = false; }
    prevA = a;
}

static bool CameraForward2D(float* outX, float* outY);

static void PollAoeTargeting() {
    static bool  active   = false;
    static bool  pa = false, pb = false;
    static DWORD lastPick = 0;
    static uint32_t lastShape = 0;
    static bool  seeded   = false;
    static DWORD lastMs   = 0;

    const uint32_t shape = AoeShape();
    const bool now = (shape != 0);
    InterlockedExchange(&g_aoeTargeting, now ? 1 : 0);

    if (!now) {
        if (active) { active = false; Log("[DA2] targeting mode OFF\n"); }
        pa = pb = false; lastShape = 0; seeded = false;
        return;
    }
    if (!active || shape != lastShape) {
        active = true; lastShape = shape;
        pa = pb = true;
        lastPick = GetTickCount(); lastMs = lastPick; seeded = false;
        Log("[DA2] targeting mode ON  shape=%u (%s) -- aim with the CAMERA, A commits, B cancels\n",
            shape, AoeIsMoveToPoint() ? "move to point" : "ability");
    }

    LogReticule();

    XINPUT_STATE st{};
    if (XInputGetState(0, &st) != ERROR_SUCCESS) return;

    const DWORD nowMs = GetTickCount();
    float dt = (nowMs - lastMs) / 1000.0f;
    lastMs = nowMs;
    if (dt <= 0.0f || dt > 0.25f) dt = 0.016f;

    if (AoeIsMoveToPoint()) {
        __try {
            void* app = AppSingleton();
            void* pl  = app ? reinterpret_cast<GetPlayerObj_t>(R(ADDR_GET_PLAYER))(app, nullptr)
                            : nullptr;
            if (pl) {
                const float* ppos = reinterpret_cast<const float*>((uintptr_t)pl + OFF_OBJ_POS);
                const float* pfwd = reinterpret_cast<const float*>((uintptr_t)pl + OFF_OBJ_FACING);
                float* ret = reinterpret_cast<float*>(R(ADDR_AOE_RETICULE));

                if (!seeded) {
                    seeded = true;
                    ret[0] = ppos[0]; ret[1] = ppos[1]; ret[2] = ppos[2];
                }

                float sx, sy;
                if (ReadLeftStick(st, sx, sy)) {
                    const float SPEED = 9.0f;

                    float fx = pfwd[0], fy = pfwd[1];
                    const bool cam = CameraForward2D(&fx, &fy);

                    static int lastMtpBasis = -1;
                    const int mtpNow = cam ? 1 : 0;
                    if (mtpNow != lastMtpBasis) {
                        lastMtpBasis = mtpNow;
                        Log("[DA2] move-to-point basis -> %s\n",
                            cam ? "CAMERA" : "player facing (camera unavailable)");
                    }

                    const float rx =  fy,     ry = -fx;
                    ret[0] += (fx * sy + rx * sx) * SPEED * dt;
                    ret[1] += (fy * sy + ry * sx) * SPEED * dt;

                    const float dx = ret[0] - ppos[0], dy = ret[1] - ppos[1];
                    const float d2 = dx * dx + dy * dy;
                    const float MAXD = 25.0f;
                    if (d2 > MAXD * MAXD) {
                        const float s = MAXD / sqrtf(d2);
                        ret[0] = ppos[0] + dx * s;
                        ret[1] = ppos[1] + dy * s;
                    }
                }
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    const bool a = (st.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;
    const bool b = (st.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
    if (a && !pa) {

        if (AoeReticuleInvalid()) {
            Log("[DA2] targeting: cannot commit -- reticule invalid (no ground there)\n");
        } else {

            const bool wasMove = AoeIsMoveToPoint();
            Log("[DA2] targeting: commit (%s)\n", wasMove ? "move to point" : "ability");
            SendCommand(L"FireAOEAbilityMessage");
            InterlockedExchange(&g_eatFaceGameplay, 1);

            if (wasMove) InterlockedExchange(&g_eatMoveUntilCentred, 1);
        }
    } else if (b && !pb) {
        Log("[DA2] targeting: cancelled\n");
        SendCommand(L"CancelAbilityTargetingMessage");
        InterlockedExchange(&g_eatFaceGameplay, 1);
    }
    pa = a; pb = b;
}

static volatile LONG g_pauseHeld = 0;

static void UpdateConsolePause() {
    if (!g_canPause) return;

    constexpr bool kPauseOnRadial = true;

    const LONG want = ((kPauseOnRadial && g_radialVisible)
                    || InterlockedCompareExchange(&g_aoeTargeting, 0, 0) != 0) ? 1 : 0;

    if (InterlockedExchange(&g_pauseHeld, want) == want) return;

    if (want) {
        SendCommand(g_useMemoryPause ? L"MemoryPauseMessage" : L"PauseMessage");
    } else {

        if (g_useMemoryPause) SendCommand(L"MemoryUnpauseMessage");
        SendCommand(L"UnpauseMessage");
    }
    Log("[DA2] console pause %s (radial=%d targeting=%d)\n",
        want ? "ON" : "OFF", g_radialVisible ? 1 : 0,
        InterlockedCompareExchange(&g_aoeTargeting, 0, 0) ? 1 : 0);
}

constexpr uintptr_t ADDR_CAMERA_MGR   = 0x00D5869C;
constexpr uintptr_t OFF_CAMMGR_VF_GET = 0x2C;
constexpr uintptr_t OFF_HOLDER_CAMERA = 0xB8;
constexpr uintptr_t OFF_CAMERA_VF_RAY = 0x44;

typedef void* (__fastcall* CamMgrGet_t)(void* self, void* edxDummy);
typedef void  (__fastcall* CamRay_t)(void* self, void* edxDummy,
                                     int sx, int sy, float* nearPt, float* farPt);

static void* ResolveCamera() {
    __try {
        void* mgr = *reinterpret_cast<void**>(R(ADDR_CAMERA_MGR));
        if (!mgr) return nullptr;
        void** vft = *reinterpret_cast<void***>(mgr);
        if (!vft) return nullptr;
        CamMgrGet_t get = reinterpret_cast<CamMgrGet_t>(vft[OFF_CAMMGR_VF_GET / 4]);
        if (!get) return nullptr;
        void* holder = get(mgr, nullptr);
        if (!holder) return nullptr;
        return *reinterpret_cast<void**>((uintptr_t)holder + OFF_HOLDER_CAMERA);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return nullptr;
    }
}

constexpr bool kUseCameraBasis = true;

static bool CameraForward2D(float* outX, float* outY) {
    if (!kUseCameraBasis) {
        static bool said = false;
        if (!said) { said = true;
            Log("[DA2] camera basis DISABLED (bisect) -- auto-aim uses player facing\n"); }
        return false;
    }
    __try {
        void* cam = ResolveCamera();
        if (!cam) return false;
        int cx = 0, cy = 0;
        if (!GameClientCentre(&cx, &cy)) return false;

        void** vft = *reinterpret_cast<void***>(cam);
        if (!vft) return false;
        CamRay_t ray = reinterpret_cast<CamRay_t>(vft[OFF_CAMERA_VF_RAY / 4]);
        if (!ray) return false;

        __declspec(align(16)) float np[4] = { 0, 0, 0, 0 };
        __declspec(align(16)) float fp[4] = { 0, 0, 0, 0 };
        ray(cam, nullptr, cx, cy, np, fp);

        const float dx = fp[0] - np[0], dy = fp[1] - np[1];
        const float d2 = dx * dx + dy * dy;

        if (!(d2 > 1e-6f) || !(d2 < 1e12f)) return false;
        const float s = 1.0f / sqrtf(d2);
        *outX = dx * s; *outY = dy * s;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

static bool CameraPitchDeg(float* outDeg) {
    if (!kUseCameraBasis) return false;
    __try {
        void* cam = ResolveCamera();
        if (!cam) return false;
        int cx = 0, cy = 0;
        if (!GameClientCentre(&cx, &cy)) return false;

        void** vft = *reinterpret_cast<void***>(cam);
        if (!vft) return false;
        CamRay_t ray = reinterpret_cast<CamRay_t>(vft[OFF_CAMERA_VF_RAY / 4]);
        if (!ray) return false;

        __declspec(align(16)) float np[4] = { 0, 0, 0, 0 };
        __declspec(align(16)) float fp[4] = { 0, 0, 0, 0 };
        ray(cam, nullptr, cx, cy, np, fp);

        const float dx = fp[0] - np[0], dy = fp[1] - np[1], dz = fp[2] - np[2];
        const float h2 = dx * dx + dy * dy;
        if (!(h2 > 1e-6f) || !(h2 < 1e12f)) return false;
        if (!(dz > -1e6f) || !(dz < 1e6f)) return false;
        *outDeg = atan2f(dz, sqrtf(h2)) * 57.2957795f;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        return false;
    }
}

constexpr bool  kCameraAutoLevel  = true;
constexpr float kCamLevelTarget   = -10.0f;
constexpr float kCamLevelTol      =   2.5f;

constexpr DWORD kCamLevelSettleMs =  150;
constexpr DWORD kCamLevelMaxMs    = 5000;

constexpr DWORD kCamLevelHoldMs   =  400;
constexpr float kCamLevelFar      = 20.0f;
constexpr float kCamLevelMaxDrive =  2.50f;
constexpr float kCamLevelMinDrive =  0.15f;

constexpr int   kCamLevelPolarity = -1;

static volatile LONG g_camFixDrive = 0;

static volatile LONG g_newGameCamPending = 0;

static void PollCameraAutoLevel() {
    if (!kCameraAutoLevel) return;

    enum { S_IDLE, S_WAIT, S_DRIVE, S_DONE };
    static int   state = S_IDLE;
    static DWORD enteredAt = 0, driveUntil = 0, lastCheck = 0, inTolSince = 0;
    static float startPitch = 0.0f, lastPitch = 0.0f;
    static int   dir = kCamLevelPolarity;
    static bool  flipped = false;

    if (!InExploreMode()) {
        if (state != S_IDLE) {
            InterlockedExchange(&g_camFixDrive, 0);
            state = S_IDLE;
        }
        return;
    }

    const DWORD now = GetTickCount();

    if (state == S_IDLE) {
        enteredAt = now;
        dir = kCamLevelPolarity;
        flipped = false;
        inTolSince = 0;
        state = S_WAIT;
        return;
    }
    if (state == S_DONE) return;

    if (state == S_WAIT) {
        if (now - enteredAt < kCamLevelSettleMs) return;
        float p = 0.0f;
        if (!CameraPitchDeg(&p)) {
            if (now - enteredAt > kCamLevelSettleMs + 5000) {
                Log("[DA2] camera level: no pitch reading, leaving it alone\n");
                state = S_DONE;
            }
            return;
        }
        startPitch = lastPitch = p;
        const bool newGame = InterlockedExchange(&g_newGameCamPending, 0) != 0;
        Log("[DA2] camera level: entry pitch %+.1f deg (target %+.1f, %s)\n",
            p, kCamLevelTarget, newGame ? "new game" : "loaded save");
        if (!newGame) {

            Log("[DA2] camera level: not a new game, leaving it alone\n");
            state = S_DONE;
            return;
        }
        const float err0 = p - kCamLevelTarget;
        if (err0 > -kCamLevelTol && err0 < kCamLevelTol) {
            Log("[DA2] camera level: already where it should be\n");
            state = S_DONE;
            return;
        }
        driveUntil = now + kCamLevelMaxMs;
        lastCheck  = now;
        inTolSince = 0;
        state = S_DRIVE;
        return;
    }

    const float ry = g_pad.ry;
    if (ry > 0.15f || ry < -0.15f) {
        Log("[DA2] camera level: player took the stick at %+.1f, stopping\n", lastPitch);
        InterlockedExchange(&g_camFixDrive, 0);
        state = S_DONE;
        return;
    }

    float p = lastPitch;
    if (CameraPitchDeg(&p)) {
        const float err = p - kCamLevelTarget;
        const float ae  = (err < 0.0f) ? -err : err;

        if (ae <= kCamLevelTol) {

            InterlockedExchange(&g_camFixDrive, 0);
            if (!inTolSince) inTolSince = now;
            if (now - inTolSince >= kCamLevelHoldMs) {
                Log("[DA2] camera level: %+.1f -> %+.1f deg, settled\n", startPitch, p);
                state = S_DONE;
                return;
            }
        } else {
            inTolSince = 0;

            float mag = (ae / kCamLevelFar) * kCamLevelMaxDrive;
            if (mag > kCamLevelMaxDrive) mag = kCamLevelMaxDrive;
            if (mag < kCamLevelMinDrive) mag = kCamLevelMinDrive;
            const int sign = (err > 0.0f) ? 1 : -1;
            InterlockedExchange(&g_camFixDrive,
                                (LONG)(dir * sign * (int)(mag * 1000.0f)));
        }

        if (now - lastCheck > 400) {
            const float dt = (float)(now - lastCheck) / 1000.0f;
            Log("[DA2] camera level: %+.1f deg, err %+.1f (%.0f deg/s)\n",
                p, err, dt > 0.0f ? (lastPitch - p) / dt : 0.0f);

            const float aeWas = (lastPitch - kCamLevelTarget < 0.0f)
                              ? -(lastPitch - kCamLevelTarget)
                              :  (lastPitch - kCamLevelTarget);
            if (!flipped && ae > aeWas + 0.5f) {
                flipped = true;
                dir = -dir;
                Log("[DA2] camera level: wrong way (err %+.1f -> %+.1f), flipping\n",
                    aeWas, ae);
            }
            lastCheck = now;
            lastPitch = p;
        }
    }

    if ((LONG)(now - driveUntil) >= 0) {
        Log("[DA2] camera level: gave up at %+.1f deg (started %+.1f)\n", p, startPitch);
        InterlockedExchange(&g_camFixDrive, 0);
        state = S_DONE;
    }
}

typedef void (__fastcall* HoverUpd_t)(void* self);
static HoverUpd_t oHoverUpdate = nullptr;
static void* g_hoverOwner = nullptr;

static volatile LONG g_hoverTick = 0;

static void __fastcall hkHoverUpdate(void* self) {
    InterlockedExchange(&g_hoverTick, (LONG)GetTickCount());
    if (!g_hoverOwner) {
        g_hoverOwner = self;
        uintptr_t vft = 0;
        __try { vft = *reinterpret_cast<uintptr_t*>(self); }
        __except (EXCEPTION_EXECUTE_HANDLER) {}
        Log("[DA2] hover updater owner = %p vft=%08X (GameModeExplore is %08X)\n",
            self, (unsigned)vft, (unsigned)R(VFT_GAMEMODE_EXPLORE));
    }
    oHoverUpdate(self);
}

typedef void (__fastcall* DirSearch_t)(void* thiz, void* edx, uint32_t dir);

static bool ObjectFacing(uint32_t id, float out[2]) {
    if (id == 0xFFFFFFFF) return false;
    __try {
        void* app = AppSingleton();
        if (!app) return false;
        void* o = reinterpret_cast<ResolveObject_t>(R(0x005A70C0))(app, nullptr, id);
        if (!o) return false;
        const float* p = reinterpret_cast<const float*>((uintptr_t)o + 0x170);
        const float a = p[0], b = p[1];
        const float len2 = a * a + b * b;
        if (!(len2 > 0.8f && len2 < 1.2f)) return false;
        out[0] = a; out[1] = b;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static bool ObjectPos(uint32_t id, float out[3]) {
    if (id == 0xFFFFFFFF) return false;
    __try {
        void* app = AppSingleton();
        if (!app) return false;
        void* o = reinterpret_cast<ResolveObject_t>(R(0x005A70C0))(app, nullptr, id);
        if (!o) return false;
        const float* p = reinterpret_cast<const float*>((uintptr_t)o + 0x160);
        for (int i = 0; i < 3; ++i) {
            const float v = p[i];
            if (!(v > -100000.0f && v < 100000.0f)) return false;
            out[i] = v;
        }
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static void*    ResolveMovieView(const wchar_t* name);
static bool     DialogIsOpen();
static uint32_t PlayerObjectId();
static int      ReadPartyMembers(uint32_t* out, int maxN);
static void     SetCurrentTargetDirect(uint32_t id);

static bool InExploreMode() {
    const LONG t = InterlockedCompareExchange(&g_hoverTick, 0, 0);
    if (!t) return false;
    const bool live = (LONG)GetTickCount() - t < 250;

    static bool wasLive = false;
    if (live != wasLive) {
        wasLive = live;
        Log("[DA2] explore mode %s\n", live ? "ACTIVE" : "inactive");
    }
    return live;
}

static void PollAutoAim() {
    if (!g_autoAim) return;

    if (InterlockedCompareExchange(&g_aoeTargeting, 0, 0)) {
        __try {
            uint32_t* o = reinterpret_cast<uint32_t*>(R(ADDR_HOVER_OVERRIDE));
            if (*o != 0xFFFFFFFF) *o = 0xFFFFFFFF;
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        return;
    }

    if (!InExploreMode()) {
        __try {
            uint32_t* o = reinterpret_cast<uint32_t*>(R(ADDR_HOVER_OVERRIDE));
            if (*o != 0xFFFFFFFF) {
                *o = 0xFFFFFFFF;
                Log("[DA2] auto-aim off (not explore mode)\n");
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
        return;
    }

    if (ResolveMovieView(L"conversation") || DialogIsOpen() || g_radialVisible) return;

    static DWORD nextAt = 0;
    const DWORD now = GetTickCount();
    if ((LONG)(now - nextAt) < 0) return;
    nextAt = now + 120;

    __try {
        uint32_t* result = reinterpret_cast<uint32_t*>(R(ADDR_DIRSEARCH_RESULT));
        uint32_t* ovr    = reinterpret_cast<uint32_t*>(R(ADDR_HOVER_OVERRIDE));
        auto search = reinterpret_cast<DirSearch_t>(R(ADDR_DIRSEARCH));
        void* ctx = reinterpret_cast<void*>(R(ADDR_DIRSEARCH_CTX));

        uint32_t cand[12]; int n = 0;
        *result = 0xFFFFFFFF;
        for (int i = 0; i < 12; ++i) {
            search(ctx, nullptr, 0 );
            const uint32_t id = *result;
            if (id == 0xFFFFFFFF) break;
            bool seen = false;
            for (int j = 0; j < n; ++j) if (cand[j] == id) { seen = true; break; }
            if (seen) break;
            cand[n++] = id;
        }

        const uint32_t me = PlayerObjectId();
        uint32_t party[16];
        const int pn = ReadPartyMembers(party, 16);

        float mePos[3], meFwd[2];
        const bool haveMe  = ObjectPos(me, mePos);
        bool haveFwd = ObjectFacing(me, meFwd);

        float camFwd[2];
        const bool haveCam = CameraForward2D(&camFwd[0], &camFwd[1]);
        if (haveCam) { meFwd[0] = camFwd[0]; meFwd[1] = camFwd[1]; haveFwd = true; }

        static int lastAimBasis = -1;
        const int basisNow = haveCam ? 1 : 0;
        if (basisNow != lastAimBasis) {
            lastAimBasis = basisNow;
            Log("[DA2] auto-aim axis -> %s\n",
                haveCam ? "CAMERA" : "player facing (camera unavailable)");
        }

        uint32_t pick = 0xFFFFFFFF, fallback = 0xFFFFFFFF;
        float bestD = 3.4e38f, bestFallbackD = 3.4e38f;
        float pickDotA = 0.0f;

        uint32_t manual = (uint32_t)InterlockedCompareExchange(&g_manualTarget, 0, 0);
        bool manualHolds = false;
        if (manual != 0xFFFFFFFF) {
            if (CurrentTarget() != manual) {
                ClearManualTarget("target changed elsewhere");
                manual = 0xFFFFFFFF;
            } else if (haveCam && g_manualYawValid) {

                const float d = camFwd[0] * g_manualYawX + camFwd[1] * g_manualYawY;
                if (d < MANUAL_RELEASE_DOT) {
                    ClearManualTarget("camera turned away");
                    manual = 0xFFFFFFFF;
                } else {
                    manualHolds = true;
                }
            } else {
                manualHolds = true;
            }
        }

        for (int i = 0; i < n; ++i) {
            if (cand[i] == me) continue;

            float p[3];
            float d = 3.3e38f, dotA = 1.0f;

            if (haveMe && ObjectPos(cand[i], p)) {
                const float dx = p[0] - mePos[0], dy = p[1] - mePos[1], dz = p[2] - mePos[2];
                d = dx * dx + dy * dy + dz * dz;

                if (haveFwd) {
                    const float flat = sqrtf(dx * dx + dy * dy);
                    if (flat > 0.001f) {
                        const float ux = dx / flat, uy = dy / flat;

                        dotA = ux * meFwd[0] + uy * meFwd[1];
                        if (dotA < 0.25f) continue;
                    }
                }
            }

            bool inParty = false;
            for (int j = 0; j < pn; ++j) if (party[j] == cand[i]) { inParty = true; break; }

            if (inParty) {
                if (d < bestFallbackD) { bestFallbackD = d; fallback = cand[i]; }
            } else {
                if (d < bestD) { bestD = d; pick = cand[i]; pickDotA = dotA; }
            }
        }
        if (pick == 0xFFFFFFFF) pick = fallback;

        if (manualHolds) {
            bool stillThere = false;
            for (int i = 0; i < n; ++i) if (cand[i] == manual) { stillThere = true; break; }
            if (!stillThere) { ClearManualTarget("target gone"); manualHolds = false; }
        }

        const uint32_t want = manualHolds ? manual : pick;

        if (*ovr != want) {
            *ovr = want;

            if (pick != 0xFFFFFFFF && ArmedAbility() == 0 && !manualHolds)
                SetCurrentTargetDirect(pick);

            Log("[DA2] auto-aim -> %08X  dist=%.1f dot=%.2f  (%d cand, %d party)\n",
                pick, (bestD < 3.0e38f) ? sqrtf(bestD) : -1.0f, pickDotA, n, pn);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static void PollHoverOverride() {
    __try {

        if (g_autoAim) return;
        const uint32_t sel = CurrentTarget();
        uint32_t*      ov  = reinterpret_cast<uint32_t*>(R(ADDR_HOVER_OVERRIDE));

        const uint32_t want = sel;
        if (*ov != want) {
            static uint32_t last = 0;
            if (want != last) {
                last = want;
                Log("[DA2] hover override -> %08X\n", want);
            }
            *ov = want;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static uint32_t HoveredObjId() {
    __try { return *reinterpret_cast<uint32_t*>(R(ADDR_HOVERED_OBJ_ID)); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 0xFFFFFFFF; }
}

static bool SetHoveredObjId(uint32_t id) {
    __try {
        void* p = reinterpret_cast<void*>(R(ADDR_HOVERED_OBJ_ID));
        MEMORY_BASIC_INFORMATION mbi{};
        if (VirtualQuery(p, &mbi, sizeof(mbi))) {
            const DWORD writable = PAGE_READWRITE | PAGE_WRITECOPY |
                                   PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
            if (!(mbi.Protect & writable)) {
                DWORD old = 0;
                if (!VirtualProtect(p, sizeof(uint32_t), PAGE_READWRITE, &old)) {
                    Log("[DA2] hover: global is READ-ONLY and VirtualProtect failed\n");
                    return false;
                }
                Log("[DA2] hover: global was read-only (0x%X), made writable\n", mbi.Protect);
            }
        }
        *reinterpret_cast<uint32_t*>(p) = id;
        return true;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static bool DialogIsOpen() {
    void* gui = GuiSystem();
    if (!gui) return false;
    __try {
        return *reinterpret_cast<uint8_t*>((uintptr_t)gui + OFF_GUI_DIALOG_OPEN) != 0;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static uint32_t EasyDialogId() {
    void* gui = GuiSystem();
    if (!gui) return 0xFFFFFFFF;
    __try {
        uintptr_t l = *reinterpret_cast<uintptr_t*>((uintptr_t)gui + 0x2C4);
        if (!l) return 0xFFFFFFFF;
        return *reinterpret_cast<uint32_t*>(l + 0x168);
    } __except (EXCEPTION_EXECUTE_HANDLER) { return 0xFFFFFFFF; }
}

static void AnswerDialog(int button);
static void* ResolveMovieView(const wchar_t* name);
static void SendGamepadEvent(int buttonId, bool pressed);

static volatile LONG g_eatFace = 0;

static void PumpDialogInput() {
    XINPUT_STATE st{};
    if (XInputGetState(0, &st) != ERROR_SUCCESS) return;
    const WORD wb = st.Gamepad.wButtons;
    static bool pa = false, pb = false;
    const bool da = (wb & XINPUT_GAMEPAD_A) != 0;
    const bool db = (wb & XINPUT_GAMEPAD_B) != 0;

    if (InterlockedCompareExchange(&g_dialogNeedRelease, 0, 0)) {
        if (!da && !db) {
            InterlockedExchange(&g_dialogNeedRelease, 0);
            Log("[DA2] dialog: opening press released, now accepting input\n");
        }
        pa = da; pb = db;
        return;
    }

    static DWORD pendingAt = 0;
    static int   pendingBtn = -1;

    if ((db && !pb) || (da && !pa)) {

        const int btn = (db && !pb) ? 0 : 1;
        const int gp  = (btn == 0) ? GP_RightAction : GP_BottomAction;
        Log("[DA2] dialog: %s -> popup movie\n", btn == 0 ? "OK (circle)" : "Cancel (cross)");
        SendGamepadEvent(gp, true);
        SendGamepadEvent(gp, false);
        pendingBtn = btn;
        pendingAt  = GetTickCount();
        InterlockedExchange(&g_eatFace, 1);
    }

    if (pendingBtn >= 0 && !DialogIsOpen()) pendingBtn = -1;

    if (pendingBtn >= 0 && GetTickCount() - pendingAt > 300) {
        if (DialogIsOpen()) {
            Log("[DA2] dialog: movie did not close it -- answering natively\n");
            AnswerDialog(pendingBtn);
        } else {
            Log("[DA2] dialog: closed by the movie\n");
        }
        pendingBtn = -1;
    }
    pa = da; pb = db;
}

static void AnswerDialog(int button) {

    {
        const char* m = (button == 0) ? "_root.DialogBoxCallback0"
                                      : "_root.DialogBoxCallback1";
        Log("[DA2] dialog: %s -> %d\n", m, InvokeRawAS(L"PopupLayer", m));
    }

    wchar_t cmd[64];
    _snwprintf_s(cmd, _countof(cmd), _TRUNCATE, L"DialogBoxClosedMessage %d", button);
    if (SendCommand(cmd)) return;

    const uint32_t id = EasyDialogId();
    if (id == 0xFFFFFFFF) { Log("[DA2] dialog: no easy-dialog listener either\n"); return; }
    _snwprintf_s(cmd, _countof(cmd), _TRUNCATE,
                 L"EasyDialogBoxClosedMessage %d %u", button, id);
    SendCommand(cmd);
}

static void LogDialogState() {
    void* gui = GuiSystem();
    if (!gui) return;
    static int pOpen = -1, pCount = -1;
    __try {
        int o = *reinterpret_cast<uint8_t*>((uintptr_t)gui + OFF_GUI_DIALOG_OPEN);
        int c = *reinterpret_cast<int*>((uintptr_t)gui + OFF_GUI_POPUP_COUNT);
        if (o != pOpen || c != pCount) {
            Log("[DA2] dialog state: open=%d count=%d easyId=%08X\n", o, c, EasyDialogId());
            pOpen = o; pCount = c;
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
}

static uint32_t TargetMode() {
    __try { return *reinterpret_cast<uint32_t*>(R(ADDR_TARGET_MODE)); }
    __except (EXCEPTION_EXECUTE_HANDLER) { return 0xFFFFFFFF; }
}

static void LogModeChanges() {
    static uint32_t pm = 0xEEEEEEEE, pa = 0xEEEEEEEE;
    uint32_t m = TargetMode(), a = ArmedAbility();
    if (m != pm || a != pa) {
        Log("[DA2] targetMode %08X -> %08X   armedAbility %08X -> %08X\n", pm, m, pa, a);
        pm = m; pa = a;
    }
}

static volatile LONG g_friendlyTarget = 0;

static volatile LONG g_allyUntil = 0;

typedef void (__fastcall* SetCurTarget_t)(void*, void*, uint32_t);

static void SetCurrentTargetDirect(uint32_t id) {
    __try {
        void* app = AppSingleton();
        if (!app) return;
        void* gm = *reinterpret_cast<void**>((uintptr_t)app + 0x144);
        if (!gm) return;
        reinterpret_cast<SetCurTarget_t>(R(0x0042C010))(gm, nullptr, id);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Log("[DA2] SetCurrentTargetDirect(%08X) faulted\n", id);
    }
}

typedef void* (__fastcall* GetPlayerObj_t)(void*, void*);

static uint32_t PlayerObjectId() {
    __try {
        void* app = AppSingleton();
        if (!app) return 0xFFFFFFFF;
        void* p = reinterpret_cast<GetPlayerObj_t>(R(0x005A7220))(app, nullptr);
        if (!p) return 0xFFFFFFFF;
        return *reinterpret_cast<uint32_t*>((uintptr_t)p + 0x10);
    } __except (EXCEPTION_EXECUTE_HANDLER) { return 0xFFFFFFFF; }
}

static uint32_t g_allyIds[8];
static int      g_allyN   = 0;
static int      g_allyIdx = 0;

static void AddAlly(uint32_t id) {
    if (id == 0xFFFFFFFF) return;
    for (int i = 0; i < g_allyN; ++i) if (g_allyIds[i] == id) return;
    if (g_allyN < 8) g_allyIds[g_allyN++] = id;
}

typedef void* (__fastcall* GetParty_t)(void*, void*);

static int ReadPartyMembers(uint32_t* out, int maxN) {
    __try {
        void* app = AppSingleton();
        if (!app) return 0;
        void* party = reinterpret_cast<GetParty_t>(R(0x005A6C90))(app, nullptr);
        if (!party) return 0;

        const uintptr_t vec = reinterpret_cast<uintptr_t>(party) + 0x2E8;
        uint32_t* b = *reinterpret_cast<uint32_t**>(vec + 0x0C);
        uint32_t* e = *reinterpret_cast<uint32_t**>(vec + 0x10);
        if (!b || e < b) return 0;
        int n = static_cast<int>(e - b);
        if (n <= 0 || n > 16) return 0;
        if (n > maxN) n = maxN;
        for (int i = 0; i < n; ++i) out[i] = b[i];
        return n;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
}

static void BuildAllyList() {
    const uint32_t start = CurrentTarget();
    g_allyN = 0;

    uint32_t mask = 0;
    __try { mask = *reinterpret_cast<uint32_t*>(R(0x00D4FCEC)); }
    __except (EXCEPTION_EXECUTE_HANDLER) {}

    uint32_t party[16] = {};
    const int pn = ReadPartyMembers(party, 16);

    if (mask & 0x01) {
        const uint32_t me = PlayerObjectId();
        if (me != 0xFFFFFFFF) AddAlly(me);
    }
    for (int i = 0; i < pn; ++i)
        if (TargetLegalForArmedAbility(party[i])) AddAlly(party[i]);

    if (g_allyN == 0) {
        static const wchar_t* kDirs[4] = {
            L"SelectForwardTargetableObjectMessage",
            L"SelectBackTargetableObjectMessage",
            L"SelectLeftTargetableObjectMessage",
            L"SelectRightTargetableObjectMessage",
        };
        Log("[DA2] ally list: party read failed (n=%d) -- falling back to sweep\n", pn);
        if (TargetLegalForArmedAbility(start)) AddAlly(start);
        for (int d = 0; d < 4; ++d)
            for (int k = 0; k < 8; ++k) {
                SendCommand(kDirs[d]);
                const uint32_t id = CurrentTarget();
                if (TargetLegalForArmedAbility(id)) AddAlly(id);
            }
    }

    g_allyIdx = 0;
    char line[256];
    int p = _snprintf_s(line, _countof(line), _TRUNCATE,
                        "[DA2] ally list (%d):", g_allyN);
    for (int i = 0; i < g_allyN && p > 0 && p < (int)_countof(line) - 12; ++i)
        p += _snprintf_s(line + p, _countof(line) - p, _TRUNCATE, " %08X", g_allyIds[i]);
    Log("%s\n", line);

    if (g_allyN) {
        SetCurrentTargetDirect(g_allyIds[0]);
        SetFriendlyCastTarget(g_allyIds[0]);
    } else {
        SetCurrentTargetDirect(start);
        Log("[DA2] ally list: NONE legal -- leaving the target alone\n");
    }
}

static void StepAlly(int dir) {
    if (g_allyN <= 0) return;
    g_allyIdx = ((g_allyIdx + dir) % g_allyN + g_allyN) % g_allyN;
    const uint32_t id = g_allyIds[g_allyIdx];
    SetCurrentTargetDirect(id);
    SetFriendlyCastTarget(id);
    Log("[DA2] ally %d/%d -> %08X | castTarget=%08X\n",
        g_allyIdx + 1, g_allyN, id, FriendlyCastTarget());
}

static void EnterAllySelect() {
    InterlockedExchange(&g_allyUntil, (LONG)(GetTickCount() + ALLY_SELECT_MS));
}
static void LeaveAllySelect() {
    InterlockedExchange(&g_allyUntil, 0);
}

static void EnterAllySelect();
static void BuildAllyList();
static bool FriendlyCastActive();
static void RadialHide();

static void PollArmedAbilityWatch() {
    static uint32_t prev = 0;
    const uint32_t now = ArmedAbility();
    if (now && !prev) {
        const uint32_t mask = ArmedTargetMask();

        if (g_radialVisible) {
            Log("[DA2] ability %08X armed from the wheel -- closing radial\n", now);
            RadialHide();
        }

        if ((mask & 0x02) && !FriendlyCastActive()) {
            EnterAllySelect();
            BuildAllyList();
            Log("[DA2] ability %08X armed elsewhere (mask %08X) -- ally select ON\n",
                now, mask);
        }
    }
    prev = now;
}

static bool FriendlyCastActive() {
    if (InterlockedCompareExchange(&g_friendlyTarget, 0, 0)) return true;
    const LONG until = InterlockedCompareExchange(&g_allyUntil, 0, 0);
    if (!until) return false;

    if (!ArmedAbility()) {
        LeaveAllySelect();
        SetFriendlyCastTarget(0xFFFFFFFF);
        Log("[DA2] ally select ended -- ability no longer armed\n");
        return false;
    }
    if ((LONG)GetTickCount() - until >= 0) {
        LeaveAllySelect();
        SetFriendlyCastTarget(0xFFFFFFFF);
        Log("[DA2] ally select timed out -- D-Pad back to targeting\n");
        return false;
    }
    return true;
}

static uint8_t g_movieKeys[_countof(kGuiMovies)][ECSTRING_SIZE];
static bool    g_movieKeysBuilt = false;

static void BuildMovieKeys() {
    if (g_movieKeysBuilt) return;
    auto b = reinterpret_cast<WBuild_t>(R(ADDR_BUILD_WSTRING));
    for (size_t i = 0; i < _countof(kGuiMovies); ++i)
        b(g_movieKeys[i], nullptr, kGuiMovies[i]);
    g_movieKeysBuilt = true;
    Log("[DA2] %d movie lookup keys built\n", (int)_countof(kGuiMovies));
}

static const wchar_t* kHudLayers[] = {
    L"CombinedHUD", L"FloatyLayer", L"PopupLayer", L"Tooltips",
    L"RadialQuickbar", L"Quickbar", L"MiniMap", L"Portraits", L"Notifications",
};

static bool IsHudLayer(const wchar_t* n) {
    for (const wchar_t* h : kHudLayers) if (!wcscmp(h, n)) return true;
    return false;
}

static volatile LONG g_menuOpen = 0;

static void RadialForceHide() {
    InvokeAS2(T_RADIAL, L"RadialQuickbarScene.HideRadialQuickbar");
    InvokeAS2(T_HUD,    L"BattleMenu.Unfade");
    InvokeAS2(T_HUD,    L"BattleMenu.MoveToBottomCorner");
    g_showPending   = false;
    g_radialVisible = false;
    OpenRadialScreen();
}

static void PollRadialStaleHide() {
    static DWORD leftAt = 0, enteredAt = 0;
    static bool armed = true;

    if (!InExploreMode()) {
        if (!leftAt) leftAt = GetTickCount();
        if (GetTickCount() - leftAt > 1500) { enteredAt = 0; armed = true; }
        return;
    }
    leftAt = 0;
    if (!armed) return;
    if (!enteredAt) { enteredAt = GetTickCount(); return; }
    if (GetTickCount() - enteredAt < 700) return;

    armed = false;
    if (g_radialVisible || g_showPending) return;
    RadialForceHide();
    Log("[DA2] radial: cleared stale panel after area load\n");
}

static void PollScreen1Probe() {
    LONG at = InterlockedCompareExchange(&g_probeScreen1At, 0, 0);
    if (!at || (LONG)(GetTickCount() - (DWORD)at) < 0) return;
    InterlockedExchange(&g_probeScreen1At, 0);

    void* gui = GuiSystem();
    if (!gui) { Log("[DA2] PROBE1: no GUI system\n"); return; }

    __try {
        void** vt = *reinterpret_cast<void***>(gui);
        auto get = reinterpret_cast<GetScreenByType_t>(vt[0x6C / 4]);
        void* scr = get(gui, 1);
        if (!scr) { Log("[DA2] PROBE1: no screen object for type 1\n"); return; }

        __try {
            void** vft = *reinterpret_cast<void***>(scr);
            void*  col = vft[-1];
            const char* cname =
                reinterpret_cast<const char*>(reinterpret_cast<void**>(col)[3]) + 8;
            Log("[DA2] PROBE1: screen class %s\n", cname);
        } __except (EXCEPTION_EXECUTE_HANDLER) {
            Log("[DA2] PROBE1: class name unreadable\n");
        }

        const uint8_t* s = reinterpret_cast<const uint8_t*>(scr) + OFF_SCREEN_MOVIENAME;
        char hex[3 * 24 + 1] = {};
        char asc[24 + 1] = {};
        for (int i = 0; i < 24; ++i) {
            _snprintf_s(hex + i * 3, 4, _TRUNCATE, "%02X ", s[i]);
            asc[i] = (s[i] >= 0x20 && s[i] < 0x7F) ? (char)s[i] : '.';
        }
        Log("[DA2] PROBE1: +A8 %s |%s|\n", hex, asc);
    } __except (EXCEPTION_EXECUTE_HANDLER) {
        Log("[DA2] PROBE1: screen read faulted\n");
    }

    char line[900];
    line[0] = 0;
    int p = 0, n = 0;
    for (const wchar_t* m : kGuiMovies) {
        if (!ResolveMovieView(m)) continue;
        ++n;
        if (p < (int)sizeof(line) - 40) {
            const int w = _snprintf_s(line + p, sizeof(line) - p, _TRUNCATE, " %ls", m);
            if (w > 0) p += w;
        }
    }
    Log("[DA2] PROBE1: %d movies resolve:%s\n", n, line);
}

static void PollScreen1Watch() {
    LONG live = 0;
    void* gui = GuiSystem();
    if (gui) {
        __try {
            void** vt = *reinterpret_cast<void***>(gui);
            auto get = reinterpret_cast<GetScreenByType_t>(vt[0x6C / 4]);

            const bool onScreen = get(gui, 1) != nullptr;
            const bool busy = g_radialVisible
                           || ResolveMovieView(L"conversation") != nullptr;
            live = (onScreen && !busy) ? 1 : 0;
        } __except (EXCEPTION_EXECUTE_HANDLER) { live = 0; }
    }
    if (InterlockedExchange(&g_screen1Live, live) != live)
        Log("[DA2] PROBE1: screen 1 %s\n",
            live ? "OPEN -- cursor freed, logging its messages" : "closed");
}

static void PollRadialTrigger() {
    static bool prevLt = false, prevCirc = false, prevCross = false;
    XINPUT_STATE st{};
    if (XInputGetState(0, &st) != ERROR_SUCCESS) { prevLt = false; return; }

    if (InterlockedCompareExchange(&g_menuOpen, 0, 0) || DialogIsOpen()) {
        if (g_radialVisible) RadialHide();
        prevLt = false;
        return;
    }

    const bool lt = st.Gamepad.bLeftTrigger > TRIGGER_THRESHOLD;
    if (lt && !prevLt) RadialShowRequest();
    if (!lt && prevLt) RadialHide();
    if (lt)            RadialTick();
    prevLt = lt;

    const bool circ = (st.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;
    const bool cross= (st.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;

    if (circ && !prevCirc && g_radialVisible) RadialDoAction();

    if (cross && !prevCross && g_radialVisible) {
        Log("[DA2] radial: cross -> back\n");
        SendGamepadEvent(GP_BottomAction, true);  SendGamepadEvent(GP_BottomAction, false);
        SendGamepadEvent(GP_RightAction,  true);  SendGamepadEvent(GP_RightAction,  false);
    }
    prevCirc = circ; prevCross = cross;
}

static volatile LONG g_chargenActive = 0;

static void* ResolveMovieViewByKey(void* key) {
    void* gui = GuiSystem();
    if (!gui) return nullptr;
    __try {
        void* reg = *reinterpret_cast<void**>((uintptr_t)gui + OFF_MOVIE_REGISTRY);
        if (!reg) return nullptr;
        void* entry = reinterpret_cast<ResolveMovie_t>(R(ADDR_RESOLVE_MOVIE))(reg, nullptr, key);
        if (!entry) return nullptr;
        return *reinterpret_cast<void**>((uintptr_t)entry + OFF_ENTRY_MOVIEVIEW);
    } __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
}

static int InvokeRawAS(const wchar_t* movie, const char* method) {
    void* gui = GuiSystem();
    if (!gui || !oInvokeAS) return -1;
    __try {
        void* reg = *reinterpret_cast<void**>((uintptr_t)gui + OFF_MOVIE_REGISTRY);
        if (!reg) return -1;
        uint8_t s[ECSTRING_SIZE] = {};
        reinterpret_cast<WBuild_t>(R(ADDR_BUILD_WSTRING))(s, nullptr, movie);
        void* entry = reinterpret_cast<ResolveMovie_t>(R(ADDR_RESOLVE_MOVIE))(reg, nullptr, s);
        if (!entry) return -2;
        return (int)oInvokeAS(entry, nullptr, method, nullptr, nullptr, 0);
    } __except (EXCEPTION_EXECUTE_HANDLER) { return -3; }
}

constexpr uintptr_t ADDR_FSCMD_SENDMESSAGE = 0x00775880;
typedef void (__cdecl* FsSendMessage_t)(void*, const char*);
static FsSendMessage_t oFsSendMessage = nullptr;

static void __cdecl hkFsSendMessage(void* ctx, const char* args) {
    __try {
        if (args && Readable((uintptr_t)args, 16)
            && strncmp(args, "DA2TRACE ", 9) == 0)
            Log("[DA2] %.400s\n", args);
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    oFsSendMessage(ctx, args);
}

static ResolveMovie_t oResolveMovie = nullptr;

static void* __fastcall hkResolveMovie(void* reg, void* edx, void* key) {
    void* r = oResolveMovie(reg, edx, key);
    if (r && key && InterlockedCompareExchange(&g_screen1Live, 0, 0)) {
        __try {
            static unsigned char seen[32][24] = {};
            static int sn = 0;
            const unsigned char* k = reinterpret_cast<const unsigned char*>(key);
            bool isNew = true;
            for (int i = 0; i < sn; ++i)
                if (memcmp(seen[i], k, 24) == 0) { isNew = false; break; }
            if (isNew && sn < 32) {
                memcpy(seen[sn], k, 24);
                ++sn;

                char hex[3 * 24 + 1] = {};
                char asc[24 + 1] = {};
                for (int i = 0; i < 24; ++i) {
                    _snprintf_s(hex + i * 3, 4, _TRUNCATE, "%02X ", k[i]);
                    asc[i] = (k[i] >= 0x20 && k[i] < 0x7F) ? (char)k[i] : '.';
                }
                Log("[DA2] RESOLVE: %s |%s|\n", hex, asc);
                Log("[DA2] RESOLVE   inline=\"%.23ls\"\n",
                    reinterpret_cast<const wchar_t*>(k));
                const wchar_t* p = *reinterpret_cast<const wchar_t* const*>(k);
                if (reinterpret_cast<uintptr_t>(p) >= 0x10000
                    && reinterpret_cast<uintptr_t>(p) < 0x80000000)
                    Log("[DA2] RESOLVE   deref =\"%.31ls\"\n", p);
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }
    return r;
}

static void* ResolveMovieView(const wchar_t* name) {
    void* gui = GuiSystem();
    if (!gui) return nullptr;
    __try {
        void* reg = *reinterpret_cast<void**>((uintptr_t)gui + OFF_MOVIE_REGISTRY);
        if (!reg) return nullptr;
        uint8_t s[ECSTRING_SIZE] = {};
        reinterpret_cast<WBuild_t>(R(ADDR_BUILD_WSTRING))(s, nullptr, name);
        void* entry = reinterpret_cast<ResolveMovie_t>(R(ADDR_RESOLVE_MOVIE))(reg, nullptr, s);
        if (!entry) return nullptr;
        return *reinterpret_cast<void**>((uintptr_t)entry + OFF_ENTRY_MOVIEVIEW);
    } __except (EXCEPTION_EXECUTE_HANDLER) { return nullptr; }
}

static bool MenuIsOpen() {
    for (size_t i = 0; i < _countof(kGuiMovies); ++i) {
        if (IsHudLayer(kGuiMovies[i])) continue;
        if (ResolveMovieViewByKey(g_movieKeys[i])) return true;
    }
    return false;
}

typedef void* (__fastcall* PickerPick_t)(void* thiz, void* edx);
static PickerPick_t oPickerPick = nullptr;

static volatile LONG g_pickerSlot = -1;

static volatile LONG g_pickerDir = 1;

static volatile LONG g_pickerAutoAt    = 0;
static volatile LONG g_pickerAutoStage = 0;

constexpr DWORD kPickerAutoDelayMs = 180;

static volatile LONG g_pickerLiveMs  = 0;
static volatile LONG g_pickerClosedAt = 0;
constexpr DWORD kPickerLiveWindowMs = 1500;
static bool PickerLive() {
    LONG t = InterlockedCompareExchange(&g_pickerLiveMs, 0, 0);
    if (!t) return false;
    return (GetTickCount() - (DWORD)t) < kPickerLiveWindowMs;
}

static int PickerEntryForSlot(int slot, int n);

static bool EntryPickable(uintptr_t e) {
    __try {
        return *reinterpret_cast<uint32_t*>(e + OFF_ENTRY_STATE) != 3
            && *reinterpret_cast<uint8_t*>(e + OFF_ENTRY_LOCKED) == 0;
    } __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

static volatile LONG g_pickerThis = 0;

static uintptr_t PickerThis() {
    return (uintptr_t)(LONG_PTR)InterlockedCompareExchange(&g_pickerThis, 0, 0);
}

static void* __fastcall hkPickerPick(void* thiz, void* edx) {
    if (thiz) {
        InterlockedExchange(&g_pickerThis, (LONG)(uintptr_t)thiz);

        __try {
            const uintptr_t gm = reinterpret_cast<uintptr_t>(thiz);
            const uintptr_t b  = *reinterpret_cast<uintptr_t*>(gm + OFF_PICK_BEGIN);
            const uintptr_t e  = *reinterpret_cast<uintptr_t*>(gm + OFF_PICK_END);
            if (b && e > b) {
                const size_t n = (e - b) / PICK_ENTRY_STRIDE;
                if (n >= 1 && n <= 64)
                    InterlockedExchange(&g_pickerLiveMs, (LONG)GetTickCount());
            }
        } __except (EXCEPTION_EXECUTE_HANDLER) {}
    }

    LONG want = InterlockedCompareExchange(&g_pickerSlot, -1, -1);
    if (want < 0 || !thiz) return oPickerPick(thiz, edx);

    __try {
        const uintptr_t gm = reinterpret_cast<uintptr_t>(thiz);

        if (*reinterpret_cast<uint8_t*>(gm + OFF_PICK_ACTIVE)
         || *reinterpret_cast<uint8_t*>(gm + OFF_PICK_LOCKED)
         || *reinterpret_cast<uint8_t*>(gm + OFF_PICK_BLOCKED))
            return oPickerPick(thiz, edx);

        const uintptr_t b = *reinterpret_cast<uintptr_t*>(gm + OFF_PICK_BEGIN);
        const uintptr_t e = *reinterpret_cast<uintptr_t*>(gm + OFF_PICK_END);
        if (!b || e <= b) return oPickerPick(thiz, edx);
        const int n = static_cast<int>((e - b) / PICK_ENTRY_STRIDE);
        if (n <= 0 || n > 64) return oPickerPick(thiz, edx);

        const int dir = (int)InterlockedCompareExchange(&g_pickerDir, 0, 0) >= 0 ? 1 : -1;
        for (int k = 0; k < n; ++k) {

            const int slot = ((want + k * dir) % n + n) % n;
            const int i    = PickerEntryForSlot(slot, n);
            const uintptr_t entry = b + static_cast<size_t>(i) * PICK_ENTRY_STRIDE;
            if (EntryPickable(entry)) {

                if (slot != want) InterlockedExchange(&g_pickerSlot, slot);
                return reinterpret_cast<void*>(entry);
            }
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) {}
    return oPickerPick(thiz, edx);
}

static bool SafeFloat(uintptr_t p, float* out) {
    __try { *out = *reinterpret_cast<float*>(p); return true; }
    __except (EXCEPTION_EXECUTE_HANDLER) { return false; }
}

constexpr uintptr_t OFF_CRE_POS_X = 0x160;

static int  g_pickerOrder[64];
static int  g_pickerOrderN = 0;

static volatile LONG g_pickerOrderFor = 0;

static void BuildPickerOrder() {
    g_pickerOrderN = 0;
    const uintptr_t gm = PickerThis();
    if (!gm) return;
    __try {
        const uintptr_t b = *reinterpret_cast<uintptr_t*>(gm + OFF_PICK_BEGIN);
        const uintptr_t e = *reinterpret_cast<uintptr_t*>(gm + OFF_PICK_END);
        if (!b || e <= b) return;
        int n = static_cast<int>((e - b) / PICK_ENTRY_STRIDE);
        if (n <= 0 || n > 64) return;

        float key[64];
        for (int i = 0; i < n; ++i) {
            g_pickerOrder[i] = i;
            key[i] = 0.0f;
            const uintptr_t cre = *reinterpret_cast<uintptr_t*>(
                b + (size_t)i * PICK_ENTRY_STRIDE + 0x04);
            if (cre) SafeFloat(cre + OFF_CRE_POS_X, &key[i]);
        }

        for (int i = 1; i < n; ++i) {
            const int  idx = g_pickerOrder[i];
            const float k  = key[idx];
            int j = i - 1;
            while (j >= 0 && key[g_pickerOrder[j]] < k) {
                g_pickerOrder[j + 1] = g_pickerOrder[j];
                --j;
            }
            g_pickerOrder[j + 1] = idx;
        }
        g_pickerOrderN = n;

        char line[256];
        int p = _snprintf_s(line, _countof(line), _TRUNCATE,
                            "[DA2] picker order (left->right, by cre+160 desc):");
        for (int i = 0; i < n && p > 0 && p < (int)_countof(line) - 8; ++i)
            p += _snprintf_s(line + p, _countof(line) - p, _TRUNCATE, " %d", g_pickerOrder[i]);
        Log("%s\n", line);
    } __except (EXCEPTION_EXECUTE_HANDLER) { g_pickerOrderN = 0; }
}

static int PickerEntryForSlot(int slot, int n) {
    if (g_pickerOrderN == n && slot >= 0 && slot < n) return g_pickerOrder[slot];
    return slot;
}

static void EnsurePickerOrder() {
    const uintptr_t gm = PickerThis();
    if (!gm) return;
    if (InterlockedCompareExchange(&g_pickerOrderFor, 0, 0) == (LONG)gm) return;
    InterlockedExchange(&g_pickerOrderFor, (LONG)gm);
    BuildPickerOrder();
}

static void StepPickerSlot(int dir) {
    int n = 0;
    __try {
        uintptr_t gm = PickerThis();
        if (gm) {
            uintptr_t b = *reinterpret_cast<uintptr_t*>(gm + OFF_PICK_BEGIN);
            uintptr_t e = *reinterpret_cast<uintptr_t*>(gm + OFF_PICK_END);
            if (b && e > b) n = static_cast<int>((e - b) / PICK_ENTRY_STRIDE);
        }
    } __except (EXCEPTION_EXECUTE_HANDLER) { n = 0; }
    if (n <= 0 || n > 64) return;

    InterlockedExchange(&g_pickerDir, dir);
    LONG cur = InterlockedCompareExchange(&g_pickerSlot, -1, -1);
    LONG next = (cur < 0) ? (dir > 0 ? 0 : n - 1) : ((cur + dir) % n + n) % n;
    InterlockedExchange(&g_pickerSlot, next);
    Log("[DA2] picker: slot %d of %d (dir %+d)\n", (int)next, n, dir);
}

static void UpdateChargenState() {
    const bool byMovie = ResolveMovieView(L"PartyPicker") != nullptr
                      || ResolveMovieView(L"CharGen")     != nullptr;
    const bool byPick  = PickerLive();
    const bool up = byMovie || byPick;
    if (up == (InterlockedCompareExchange(&g_chargenActive, 0, 0) != 0)) return;

    const LONG slotBefore = InterlockedCompareExchange(&g_pickerSlot, -1, -1);
    InterlockedExchange(&g_chargenActive, up ? 1 : 0);

    if (up) InterlockedExchange(&g_newGameCamPending, 1);
    if (up) Log("[DA2] picker detected by %s\n",
                byMovie ? (byPick ? "movie+pick" : "movie name") : "pick function");

    InterlockedExchange(&g_pickerSlot, -1);
    InterlockedExchange(&g_pickerDir, 1);
    Log("[DA2] chargen/partypicker %s\n", up ? "ACTIVE" : "closed");

    InterlockedExchange(&g_pickerSlot, -1);
    if (up) {

        InterlockedExchange(&g_pickerOrderFor, 0);

        const DWORD closedAt = (DWORD)InterlockedCompareExchange(&g_pickerClosedAt, 0, 0);
        const bool fresh = (closedAt == 0) || (GetTickCount() - closedAt > 3000);
        if (!fresh) {
            InterlockedExchange(&g_pickerSlot, slotBefore);
            Log("[DA2] picker: re-detected within 3s -- same screen, slot %d kept\n",
                (int)slotBefore);
            return;
        }

        InterlockedExchange(&g_pickerSlot, -1);
        InterlockedExchange(&g_pickerAutoAt, (LONG)(GetTickCount() + kPickerAutoDelayMs));
        InterlockedExchange(&g_pickerAutoStage, 1);
    } else {
        InterlockedExchange(&g_pickerClosedAt, (LONG)GetTickCount());
        InterlockedExchange(&g_pickerAutoStage, 0);
        InterlockedExchange(&g_pickerAutoAt, 0);
    }
}

static bool FaceButtonEaten(int buttonId) {
    if (!InterlockedCompareExchange(&g_eatFace, 0, 0)) return false;
    return buttonId == GP_BottomAction || buttonId == GP_RightAction;
}

static void ClearEatFaceWhenReleased(WORD wButtons) {
    if (!InterlockedCompareExchange(&g_eatFace, 0, 0)) return;
    if (!(wButtons & (XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_B)))
        InterlockedExchange(&g_eatFace, 0);
}

static void SendGamepadEvent(int buttonId, bool pressed) {
    if (pressed && FaceButtonEaten(buttonId)) return;

    const bool dialogModal = DialogIsOpen();
    __try {
        const char* method = reinterpret_cast<const char*>(R(ADDR_STR_REPORT_GP));
        void*  g   = *reinterpret_cast<void**>(R(ADDR_GUI_SINGLETON_PTR));
        void** gvt = g ? *reinterpret_cast<void***>(g) : nullptr;

        GFxValue args[2] = {};
        args[0].type = 3; args[0].num = (double)buttonId;
        args[1].type = 3; args[1].num = (double)(pressed ? GP_EVENT_PRESSED
                                                         : GP_EVENT_RELEASED);

        BuildMovieKeys();
        if (gvt) reinterpret_cast<GuiLock_t>(gvt[0x158 / 4])(g, nullptr);
        int delivered = 0;
        for (size_t i = 0; i < _countof(kGuiMovies); ++i) {

            if (dialogModal && pressed &&
                _wcsicmp(kGuiMovies[i], L"popuplayer") != 0) continue;
            void* mv = ResolveMovieViewByKey(g_movieKeys[i]);
            if (!mv) continue;
            void** vt = *reinterpret_cast<void***>(mv);
            reinterpret_cast<Invoke5C_t>(vt[0x5C / 4])(mv, nullptr, method, nullptr, args, 2);
            ++delivered;
        }
        if (gvt) reinterpret_cast<GuiLock_t>(gvt[0x15C / 4])(g, nullptr);

        if (delivered > 0)
            InterlockedExchange(&g_lastMenuDeliveryMs, (LONG)GetTickCount());

        InterlockedExchange(&g_menuOpen, MenuIsOpen() ? 1 : 0);
        UpdateChargenState();

        {
            static int lastCount = -1;
            if (delivered != lastCount) {
                lastCount = delivered;
                char line[512];
                int n = sprintf_s(line, sizeof(line), "[DA2] movies live (%d):", delivered);
                for (size_t i = 0; i < _countof(kGuiMovies) && n < 440; ++i)
                    if (ResolveMovieViewByKey(g_movieKeys[i]))
                        n += sprintf_s(line + n, sizeof(line) - n, " %S", kGuiMovies[i]);
                sprintf_s(line + n, sizeof(line) - n, "\n");
                OutputDebugStringA(line);
            }
        }

        static int logged = 0;
        if (logged < 12) {
            ++logged;
            Log("[DA2] button %d %s -> %d movie(s)\n",
                buttonId, pressed ? "down" : "up", delivered);
        }
    }
    __except (EXCEPTION_EXECUTE_HANDLER) {
        Log("[DA2] gamepad event EXCEPTION (button %d)\n", buttonId);
    }
}

static volatile LONG g_autoTarget = 0;

static bool g_inMenuLatched = false;

static inline float DeadZone(float v, float dz) {
    float a = v < 0 ? -v : v;
    if (a < dz) return 0.0f;
    float n = (a - dz) / (1.0f - dz);
    return v < 0 ? -n : n;
}

typedef float (__fastcall* ActionAnalog_t)(void*, void*, uint32_t*);
typedef char  (__fastcall* ActionDigital_t)(void*, void*, uint32_t*);
static ActionAnalog_t  oActionAnalog  = nullptr;
static ActionDigital_t oActionDigital = nullptr;

static void PollPadAndDispatch(bool dispatchEvents);

static float __fastcall hkActionAnalog(void* thiz, void* edx, uint32_t* handle) {

    if (handle && handle[0] == ACT_GUI_LT) {

        static bool tipsRefreshed = false;
        static DWORD firstTick = 0;
        if (!firstTick) firstTick = GetTickCount();
        if (!tipsRefreshed && GetTickCount() - firstTick > 5000) {
            tipsRefreshed = true;
            if (ResolveMovieView(L"Tooltips")) {
                const int r = InvokeRawAS(L"Tooltips", "GUISystem.StartSceneFromGame");
                Log("[DA2] tooltips: re-read platform -> %d\n", r);
            } else {
                Log("[DA2] tooltips: movie not resolvable, skipped\n");
            }
        }

        PollAutoAim();
        PollArmedAbilityWatch();
        PollHoverOverride();
        PollOptionsMouse();

        {
            int cx = 0, cy = 0;
            if (GameClientCentre(&cx, &cy)) {
                InterlockedExchange(&g_clientCx, cx);
                InterlockedExchange(&g_clientCy, cy);
            }
        }
        PollCursorHide();
        PollCursorPark();
        PollLoginBlock();
        PollScreen1Probe();
        PollScreen1Watch();
        PollCursorToggleKey();
        PollRumbleStop();
        PollCameraAutoLevel();
        PollHealthStat();
        PollRadialStaleHide();
        PollRadialTrigger();
        PollAoeTargeting();
        UpdateConsolePause();

        XINPUT_STATE es{};
        if (XInputGetState(0, &es) == ERROR_SUCCESS)
            ClearEatFaceWhenReleased(es.Gamepad.wButtons);
    }

    if (DialogIsOpen()) {
        if (handle && handle[0] == ACT_GUI_LT) { LogDialogState(); PumpDialogInput(); }
        return oActionAnalog(thiz, edx, handle);
    }

    if (handle && InterlockedCompareExchange(&g_screen1Live, 0, 0)) {
        static uint32_t seen[128] = {};
        static int sn = 0;
        bool isNew = true;
        for (int k = 0; k < sn; ++k)
            if (seen[k] == handle[0]) { isNew = false; break; }
        if (isNew && sn < 128) {
            seen[sn++] = handle[0];
            Log("[DA2] CHARGEN ACT-A: 0x%02X\n", handle[0]);
        }
    }

    if (handle) {
        if (handle[0] == ACT_GUI_LT) {

            static bool announced = false;
            if (!announced) { announced = true; Log("[DA2] GUI gamepad update is live\n"); }
            InterlockedExchange(&g_lastGuiMs, (LONG)GetTickCount());
            PollPadAndDispatch(false);
        }
        else if (handle[0] == ACT_GUI_RY) PollPadAndDispatch(true);
    }

    if (handle && InterlockedCompareExchange(&g_pad.valid, 1, 1)) {
        const uint32_t id = handle[0];
        const float rx = g_pad.rx, ry = g_pad.ry;

        {
            static bool wasPlaying = false;
            static bool needRecentre = false;
            const bool playing = InExploreMode();
            if (playing && !wasPlaying) needRecentre = true;
            wasPlaying = playing;
            if (needRecentre) {
                const float ax = rx < 0 ? -rx : rx;
                const float ay = ry < 0 ? -ry : ry;
                if (ax < 0.05f && ay < 0.05f) {
                    needRecentre = false;
                    Log("[DA2] camera: stick centred, input accepted\n");
                } else if (id == ACT_CAMERA_YAW || id == ACT_CAMERA_PITCH) {
                    static int said = 0;
                    if (said < 3) { ++said;
                        Log("[DA2] camera: ignoring deflection present at entry"
                            " (rx=%+.3f ry=%+.3f)\n", rx, ry); }
                    return oActionAnalog(thiz, edx, handle);
                }
            }
        }

        if (id == ACT_CAMERA_YAW || id == ACT_CAMERA_PITCH) {
            if (!InExploreMode()) {
                static bool said = false;
                if (!said && (rx != 0.0f || ry != 0.0f)) { said = true;
                    Log("[DA2] camera feed suppressed outside gameplay"
                        " (rx=%+.3f ry=%+.3f)\n", rx, ry); }
                return oActionAnalog(thiz, edx, handle);
            }
        }

        if (id == ACT_CAMERA_YAW && rx != 0.0f)
            return (CAMERA_INVERT_YAW ? -rx : rx) * CAMERA_SENS_YAW;
        if (id == ACT_CAMERA_PITCH && ry != 0.0f)
            return (CAMERA_INVERT_PITCH ? -ry : ry) * CAMERA_SENS_PITCH;

        if (id == ACT_CAMERA_PITCH && ry == 0.0f) {
            const LONG d = InterlockedCompareExchange(&g_camFixDrive, 0, 0);
            if (d) {
                const float synth = (float)d / 1000.0f;
                return (CAMERA_INVERT_PITCH ? -synth : synth) * CAMERA_SENS_PITCH;
            }
        }

        if (id == ACT_CAM_ZOOM) {
            if (InterlockedCompareExchange(&g_pad.zoomIn, 1, 1))  return  ZOOM_STEP;
            if (InterlockedCompareExchange(&g_pad.zoomOut, 1, 1)) return -ZOOM_STEP;
        }

        if (!g_inMenuLatched && InterlockedCompareExchange(&g_autoTarget, 0, 0)) {
            if (id == ACT_AUTOTARGET_X || id == ACT_LINEAR_TGT_X) {
                if (InterlockedCompareExchange(&g_pad.dpadRight, 1, 1)) return  1.0f;
                if (InterlockedCompareExchange(&g_pad.dpadLeft, 1, 1))  return -1.0f;
            }
            if (id == ACT_AUTOTARGET_Y || id == ACT_LINEAR_TGT_Y) {
                if (InterlockedCompareExchange(&g_pad.dpadUp, 1, 1))   return  1.0f;
                if (InterlockedCompareExchange(&g_pad.dpadDown, 1, 1)) return -1.0f;
            }
        }

        switch (id) {

            case ACT_GUI_THUMB_LX: if (g_pad.lx != 0.0f) return g_pad.lx; break;
            case ACT_GUI_THUMB_LY: if (g_pad.ly != 0.0f) return g_pad.ly; break;
            case ACT_GUI_THUMB_RX: if (rx != 0.0f)       return rx;       break;
            case ACT_GUI_THUMB_RY: if (ry != 0.0f)       return ry;       break;
            case ACT_GUI_LT:       if (g_pad.lt > 0.0f)  return g_pad.lt; break;
            case ACT_GUI_RT:       if (g_pad.rt > 0.0f)  return g_pad.rt; break;

            default: break;
        }
    }
    return oActionAnalog(thiz, edx, handle);
}

static char __fastcall hkActionDigital(void* thiz, void* edx, uint32_t* handle) {

    if (DialogIsOpen()) return oActionDigital(thiz, edx, handle);

    if (handle && InterlockedCompareExchange(&g_screen1Live, 0, 0)) {
        static uint32_t seen[128] = {};
        static int sn = 0;
        bool isNew = true;
        for (int k = 0; k < sn; ++k)
            if (seen[k] == handle[0]) { isNew = false; break; }
        if (isNew && sn < 128) {
            seen[sn++] = handle[0];
            Log("[DA2] CHARGEN ACT-D: 0x%02X\n", handle[0]);
        }
    }

    if (handle && InterlockedCompareExchange(&g_pad.valid, 1, 1)) {
        switch (handle[0]) {

        case ACT_GUI_LEFT_CLICK:
            break;

        case ACT_OBJECT_GLOW:

            {
                static bool said = false;
                if (!said) { said = true;
                    Log("[DA2] ObjectGlowEnabled (0x32) forced ON -- proximity labels\n"); }
                return 1;
            }

        case ACT_PARTYSEL_MODE:
            if (FriendlyCastActive()) {
                static bool said = false;
                if (!said) { said = true;
                    Log("[DA2] party radial: mode action 0x42 forced ON\n"); }
                return 1;
            }
            break;

        case ACT_CAM_ZOOM_IN:
            if (InterlockedCompareExchange(&g_pad.zoomIn, 1, 1)) return 1;
            break;
        case ACT_CAM_ZOOM_OUT:
            if (InterlockedCompareExchange(&g_pad.zoomOut, 1, 1)) return 1;
            break;

        case ACT_ENABLE_AUTOTARGET:
        case ACT_LINEAR_TGT_MODE:
            if (!g_inMenuLatched && InterlockedCompareExchange(&g_autoTarget, 1, 1))
                return 1;
            break;

        case ACT_FILTER_HOSTILE:
        case ACT_FILTER_OBJECTS:
            if (g_inMenuLatched) break;
            if (FriendlyCastActive()) break;
            if (InterlockedCompareExchange(&g_autoTarget, 0, 0)) return 1;
            break;
        case ACT_FILTER_PARTY:
        case ACT_FILTER_PLAYER:
            if (g_inMenuLatched) break;
            if (FriendlyCastActive()) return 1;
            if (InterlockedCompareExchange(&g_autoTarget, 0, 0)) return 1;
            break;

        default: break;
        }
    }
    return oActionDigital(thiz, edx, handle);
}

typedef void (__fastcall* MoveReader_t)(void*, void*, float*, char*);
static MoveReader_t oMoveReader = nullptr;

static bool ReadLeftStick(const XINPUT_STATE& st, float& ox, float& oy) {
    float x = st.Gamepad.sThumbLX / 32767.0f;
    float y = st.Gamepad.sThumbLY / 32767.0f;
    float mag = sqrtf(x * x + y * y);
    if (mag < STICK_DEADZONE) return false;
    if (mag > 1.0f) mag = 1.0f;
    float n = (mag - STICK_DEADZONE) / (1.0f - STICK_DEADZONE);
    ox = (x / mag) * n;
    oy = (y / mag) * n;
    return true;
}

static bool g_lb = false, g_rb = false, g_lt = false, g_rt = false, g_a = false;

static bool g_start = false, g_back = false;
static bool g_dl = false, g_dr = false, g_du = false, g_dd = false;
static bool g_bx = false, g_by = false, g_bb = false, g_aPrev = false;
static volatile LONG g_frames = 0;
static bool g_primed = false;

static volatile LONG g_gameplayTick = 0;

static void PollPadAndDispatch(bool dispatchEvents) {
    XINPUT_STATE st{};
    if (!ReadPad(st)) { InterlockedExchange(&g_pad.valid, 0); return; }

    g_pad.lx = DeadZone(st.Gamepad.sThumbLX / 32767.0f, CAM_DEADZONE);
    g_pad.ly = DeadZone(st.Gamepad.sThumbLY / 32767.0f, CAM_DEADZONE);
    g_pad.rx = DeadZone(st.Gamepad.sThumbRX / 32767.0f, CAM_DEADZONE);
    g_pad.ry = DeadZone(st.Gamepad.sThumbRY / 32767.0f, CAM_DEADZONE);

    g_pad.lt = st.Gamepad.bLeftTrigger  / 255.0f;
    g_pad.rt = st.Gamepad.bRightTrigger / 255.0f;
    const WORD wb = st.Gamepad.wButtons;
    InterlockedExchange(&g_pad.aDown,     (wb & XINPUT_GAMEPAD_A) ? 1 : 0);
    InterlockedExchange(&g_pad.dpadUp,    (wb & XINPUT_GAMEPAD_DPAD_UP) ? 1 : 0);
    InterlockedExchange(&g_pad.dpadDown,  (wb & XINPUT_GAMEPAD_DPAD_DOWN) ? 1 : 0);
    InterlockedExchange(&g_pad.dpadLeft,  (wb & XINPUT_GAMEPAD_DPAD_LEFT) ? 1 : 0);
    InterlockedExchange(&g_pad.dpadRight, (wb & XINPUT_GAMEPAD_DPAD_RIGHT) ? 1 : 0);

    InterlockedExchange(&g_pad.zoomIn,  0);
    InterlockedExchange(&g_pad.zoomOut, 0);
    InterlockedExchange(&g_pad.valid, 1);

    {
        static bool pR3 = false, pL3 = false;
        bool r3 = (wb & XINPUT_GAMEPAD_RIGHT_THUMB) != 0;
        bool l3 = (wb & XINPUT_GAMEPAD_LEFT_THUMB) != 0;
        if (r3 && !pR3) {
            LONG v = InterlockedCompareExchange(&g_autoTarget, 0, 0) ? 0 : 1;
            InterlockedExchange(&g_autoTarget, v);
            Log("[DA2] auto-target %s\n", v ? "ON" : "OFF");

            SendCommand(L"ToggleTargetLockMessage");
        }

        if (l3 && !pL3) {
            LONG v = InterlockedCompareExchange(&g_friendlyTarget, 0, 0) ? 0 : 1;
            InterlockedExchange(&g_friendlyTarget, v);
            Log("[DA2] targeting %s\n", v ? "FRIENDLY (party)" : "HOSTILE");
            if (!v) SendCommand(L"CancelFriendlyCastMessage");
        }
        pR3 = r3; pL3 = l3;
    }

    if (!dispatchEvents || g_radialVisible) return;

    InterlockedExchange(&g_gameplayTick, 0);
    g_inMenuLatched = InterlockedCompareExchange(&g_menuOpen, 0, 0) != 0;

    LogDialogState();
    if (DialogIsOpen()) {
        static bool pa = false, pb = false;
        bool da = (wb & XINPUT_GAMEPAD_A) != 0;
        bool db = (wb & XINPUT_GAMEPAD_B) != 0;
        if (db && !pb) { Log("[DA2] dialog: OK (circle)\n");     AnswerDialog(0); }
        if (da && !pa) { Log("[DA2] dialog: Cancel (cross)\n");  AnswerDialog(1); }
        pa = da; pb = db;
        return;
    }

    static bool convWasLive = false;
    if (ResolveMovieView(L"conversation")) {
        static bool pca = false;
        bool ca = (wb & XINPUT_GAMEPAD_A) != 0;
        if (!convWasLive) {
            convWasLive = true;
            pca = ca;
            if (ca) Log("[DA2] conversation opened with A held -- not skipping\n");
        } else {
            if (ca && !pca) SendCommand(L"SkipConversationMessage");
            pca = ca;
        }
    } else {
        convWasLive = false;
    }

    UpdateChargenState();

    if (InterlockedCompareExchange(&g_chargenActive, 0, 0)) {

        EnsurePickerOrder();

        LONG stage = InterlockedCompareExchange(&g_pickerAutoStage, 0, 0);
        if (stage) {
            const WORD kAny = XINPUT_GAMEPAD_DPAD_LEFT | XINPUT_GAMEPAD_DPAD_RIGHT
                            | XINPUT_GAMEPAD_DPAD_UP   | XINPUT_GAMEPAD_DPAD_DOWN
                            | XINPUT_GAMEPAD_A | XINPUT_GAMEPAD_B;
            if (stage == 1 && (wb & kAny)) {

                InterlockedExchange(&g_pickerAutoStage, 0);
                Log("[DA2] picker: auto-select cancelled -- player pressed first\n");
            } else {
                LONG at = InterlockedCompareExchange(&g_pickerAutoAt, 0, 0);

                if ((LONG)(GetTickCount() - (DWORD)at) >= 0
                 && InterlockedCompareExchange(&g_pickerOrderFor, 0, 0) != 0) {

                    StepPickerSlot(+1);
                    InterlockedExchange(&g_pickerAutoStage, 0);
                    Log("[DA2] picker: auto-selected slot 0\n");
                }
            }
        }

        static bool pl = false, pr = false, pa = false, pb = false;
        bool cl = (wb & XINPUT_GAMEPAD_DPAD_LEFT)  != 0;
        bool cr = (wb & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;
        bool ca = (wb & XINPUT_GAMEPAD_A) != 0;
        bool cb = (wb & XINPUT_GAMEPAD_B) != 0;

        if (cl && !pl) StepPickerSlot(-1);
        if (cr && !pr) StepPickerSlot(+1);
        if (cb && !pb) {

            SendCommand(L"SelectPartyMemberUnderCursorMessage");
            SendCommand(L"ChargenConfirmMessage");
        }
        if (ca && !pa) SendCommand(L"ChargenDeselectMessage");
        pl = cl; pr = cr; pa = ca; pb = cb;

    }

    static const struct { WORD mask; int id; } kMap[] = {
        { XINPUT_GAMEPAD_DPAD_UP,        GP_DPadUp        },
        { XINPUT_GAMEPAD_DPAD_DOWN,      GP_DPadDown      },
        { XINPUT_GAMEPAD_DPAD_LEFT,      GP_DPadLeft      },
        { XINPUT_GAMEPAD_DPAD_RIGHT,     GP_DPadRight     },
        { XINPUT_GAMEPAD_A,              GP_BottomAction  },
        { XINPUT_GAMEPAD_B,              GP_RightAction   },
        { XINPUT_GAMEPAD_X,              GP_LeftAction    },
        { XINPUT_GAMEPAD_Y,              GP_TopAction     },
        { XINPUT_GAMEPAD_START,          GP_Start         },
        { XINPUT_GAMEPAD_BACK,           GP_Back          },
        { XINPUT_GAMEPAD_LEFT_THUMB,     GP_LeftThumb     },
        { XINPUT_GAMEPAD_RIGHT_THUMB,    GP_RightThumb    },
        { XINPUT_GAMEPAD_LEFT_SHOULDER,  GP_LeftShoulder  },
        { XINPUT_GAMEPAD_RIGHT_SHOULDER, GP_RightShoulder },
    };
    static WORD prevButtons = 0;
    const WORD b = st.Gamepad.wButtons;
    const WORD changed = b ^ prevButtons;
    if (changed) {

        const bool eatA = OptionsCursorHot();

        for (const auto& m : kMap) {
            if (!(changed & m.mask)) continue;
            const bool down = (b & m.mask) != 0;
            if (eatA && m.mask == XINPUT_GAMEPAD_A && down) continue;
            SendGamepadEvent(m.id, down);
        }
        prevButtons = b;
    }

    static bool pLT = false, pRT = false;
    bool lt = st.Gamepad.bLeftTrigger  > TRIGGER_THRESHOLD;
    bool rt = st.Gamepad.bRightTrigger > TRIGGER_THRESHOLD;
    if (lt != pLT) { SendGamepadEvent(GP_LeftTrigger,  lt); pLT = lt; }
    if (rt != pRT) { SendGamepadEvent(GP_RightTrigger, rt); pRT = rt; }
}

static void __fastcall hkMoveReader(void* thiz, void* edx, float* outVec, char* outFlag) {
    oMoveReader(thiz, edx, outVec, outFlag);

    if (!g_primed && InterlockedIncrement(&g_frames) == 120) {
        g_primed = true;
        CloseQuickbarOnce();
        ProbeMessageNames();
        TargetSnapshot("idle");
    }

    if (g_primed) { TargetSnapshot("poll"); LogModeChanges(); LogDialogState(); }

    {
        constexpr bool kGlowAtStart      = true;
        constexpr bool kHideUntargeted   = true;

        static bool glowOn   = false;
        static bool wasMenu  = false;
        const bool  nowMenu  = InterlockedCompareExchange(&g_menuOpen, 0, 0) != 0;
        if (wasMenu && !nowMenu) glowOn = false;
        wasMenu = nowMenu;

        if (kGlowAtStart && g_primed && !nowMenu && !glowOn) {
            glowOn = true;
            SendCommand(L"EnableObjectGlowMessage");

            if (kHideUntargeted) {

                constexpr float kFloatyDimAlpha = 0.35f;
                static bool dimDone = false;
                if (!dimDone) {
                    dimDone = true;
                    __try {
                        float* dim = reinterpret_cast<float*>(R(0x00D304E4));
                        Log("[DA2] floaty dim alpha was %.3f -> %.3f (stock 0.35 = console)\n",
                            *dim, kFloatyDimAlpha);
                        *dim = kFloatyDimAlpha;
                    } __except (EXCEPTION_EXECUTE_HANDLER) {
                        Log("[DA2] floaty dim alpha write faulted\n");
                    }
                }
            }
        }
    }

    InterlockedExchange(&g_gameplayTick, 1);
    InterlockedExchange(&g_lastGameplayMs, (LONG)GetTickCount());

    XINPUT_STATE st{};
    if (!ReadPad(st)) { InterlockedExchange(&g_pad.valid, 0); return; }

    PollPadAndDispatch(true);

    if (g_radialVisible) {
        outVec[0] = 0.0f;
        outVec[1] = 0.0f;
        *outFlag = 0;
    }
    else if (outVec[0] == 0.0f && outVec[1] == 0.0f) {

        float sx, sy;
        const bool deflected = ReadLeftStick(st, sx, sy);

        static bool eating = false;
        if (InterlockedCompareExchange(&g_eatMoveUntilCentred, 0, 0)) {
            eating = true;
            InterlockedExchange(&g_eatMoveUntilCentred, 0);
        }

        if (InterlockedCompareExchange(&g_aoeTargeting, 0, 0)) {
            outVec[0] = 0.0f;
            outVec[1] = 0.0f;
            *outFlag  = 0;
        }
        else if (eating) {
            if (!deflected) { eating = false; Log("[DA2] stick centred -- movement resumed\n"); }
        }
        else if (deflected) {
            outVec[0] = sx;
            outVec[1] = sy;
            *outFlag = 1;
        }
    }

    const WORD b = st.Gamepad.wButtons;

    const bool a = (b & XINPUT_GAMEPAD_A) != 0;

    if (InterlockedCompareExchange(&g_menuOpen, 0, 0)
     || InterlockedCompareExchange(&g_aoeTargeting, 0, 0)) {
        g_lb = (b & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
        g_rb = (b & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
        g_a  = (b & XINPUT_GAMEPAD_A) != 0;
        g_aPrev = g_a;
        g_bx = (b & XINPUT_GAMEPAD_X) != 0;
        g_by = (b & XINPUT_GAMEPAD_Y) != 0;
        g_bb = (b & XINPUT_GAMEPAD_B) != 0;
        g_dl = (b & XINPUT_GAMEPAD_DPAD_LEFT)  != 0;
        g_dr = (b & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;
        g_du = (b & XINPUT_GAMEPAD_DPAD_UP)    != 0;
        g_dd = (b & XINPUT_GAMEPAD_DPAD_DOWN)  != 0;
        g_lt = st.Gamepad.bLeftTrigger  > TRIGGER_THRESHOLD;
        g_rt = st.Gamepad.bRightTrigger > TRIGGER_THRESHOLD;
        bool sIn = (b & XINPUT_GAMEPAD_START) != 0;
        bool kIn = (b & XINPUT_GAMEPAD_BACK)  != 0;
        g_start = sIn; g_back = kIn;
        if (g_radialVisible) RadialHide();
        return;
    }

    bool lb = (b & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
    bool rb = (b & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
    if (lb && !g_lb) FirePartyCycle(false);
    if (rb && !g_rb) FirePartyCycle(true);
    g_lb = lb; g_rb = rb;

    bool start = (b & XINPUT_GAMEPAD_START) != 0;
    bool back  = (b & XINPUT_GAMEPAD_BACK) != 0;
    if (start && !g_start) SendCommand(L"ToggleFullScreenGUIMessage MainMenu");
    if (back  && !g_back)  SendCommand(L"ToggleFullScreenGUIMessage Map");
    g_start = start; g_back = back;

    bool rt = st.Gamepad.bRightTrigger > TRIGGER_THRESHOLD;
    if (rt != g_rt) BattleMenuPage(rt);
    g_rt = rt;

    bool dl = (b & XINPUT_GAMEPAD_DPAD_LEFT)  != 0;
    bool dr = (b & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;
    bool du = (b & XINPUT_GAMEPAD_DPAD_UP)    != 0;
    bool dd = (b & XINPUT_GAMEPAD_DPAD_DOWN)  != 0;
    const bool friendly = FriendlyCastActive();

    if (friendly) {

        if (du && !g_du) StepAlly(-1);
        if (dd && !g_dd) StepAlly(+1);
        if (dl && !g_dl) StepAlly(-1);
        if (dr && !g_dr) StepAlly(+1);
    } else {

        auto updown = [&](bool fwd) {
            const uint32_t before = CurrentTarget();
            SendCommand(fwd ? L"SelectForwardTargetableObjectMessage"
                            : L"SelectBackTargetableObjectMessage");
            if (CurrentTarget() != before) {
                TargetSnapshot(fwd ? "dpad-up (spatial)" : "dpad-down (spatial)");
                return;
            }
            CycleTargetManual(fwd ? L"SelectNextUsableObjectMessage"
                            : L"SelectPreviousUsableObjectMessage",
                        fwd ? "dpad-up (usable)" : "dpad-down (usable)");
        };

        if (dr && !g_dr) CycleTargetManual(L"SelectNextHostileTargetMessage",     "dpad-right");
        if (dl && !g_dl) CycleTargetManual(L"SelectPreviousHostileTargetMessage", "dpad-left");
        if (du && !g_du) updown(true);
        if (dd && !g_dd) updown(false);
    }

    {
        static uint32_t seen = 0xFFFFFFFF;
        uint32_t t = CurrentTarget();
        if (t != seen) {
            seen = t;
            TargetSnapshot("poll");
        }
    }
    g_dl = dl; g_dr = dr; g_du = du; g_dd = dd;

    bool bx = (b & XINPUT_GAMEPAD_X) != 0;
    bool by = (b & XINPUT_GAMEPAD_Y) != 0;
    bool bb = (b & XINPUT_GAMEPAD_B) != 0;

    {
        static bool eating = false;
        if (InterlockedCompareExchange(&g_eatFaceGameplay, 0, 0)) {
            eating = true;
            InterlockedExchange(&g_eatFaceGameplay, 0);
        }
        if (eating) {
            if (!bx && !by && !bb && !a) eating = false;
            else {
                g_bx = bx; g_by = by; g_bb = bb; g_aPrev = a;
                return;
            }
        }
    }

    const wchar_t* slot1 = L"1";
    const wchar_t* slot2 = L"2";
    const wchar_t* slot3 = L"3";

    static DWORD armedAt = 0;
    if (friendly) {
        static bool saidFriendly = false;
        if (!saidFriendly) { saidFriendly = true;
            Log("[DA2] NOTE: ally-select mode -- D-Pad picks a target, circle casts, "
                "cross cancels. Face buttons are NOT quickslots while it is on.\n"); }

        if (bb && !g_bb) {

            const uint32_t before = ArmedAbility();
            const uint32_t tgt    = CurrentTarget();
            SendCommand(L"PerformAbilityOnSelectedTargetMessage");
            const uint32_t after  = ArmedAbility();
            Log("[DA2] ally cast at %08X: armed %08X -> %08X %s\n", tgt, before, after,
                (before == after)
                    ? "(NOT CONSUMED -- target is not legal for this ability)"
                    : "(fired)");
            if (after == 0) { SetFriendlyCastTarget(0xFFFFFFFF); LeaveAllySelect(); }
        }
        if (a && !g_aPrev) {
            SendCommand(L"CancelFriendlyCastMessage");
            SendCommand(L"CancelAbilityTargetingMessage");
            SetFriendlyCastTarget(0xFFFFFFFF);
            Log("[DA2] ally cancel: armed=%08X\n", ArmedAbility());
            LeaveAllySelect();
        }
        g_bx = bx; g_by = by; g_bb = bb;
        g_aPrev = a;
        return;
    }

    auto ArmedTargetMask = []() -> uint32_t {
        __try { return *reinterpret_cast<uint32_t*>(R(0x00D4FCEC)); }
        __except (EXCEPTION_EXECUTE_HANDLER) { return 0; }
    };

    auto tryFireArmed = [&]() -> bool {
        const uint32_t mask = ArmedTargetMask();

        if (mask & 0x02) {

            EnterAllySelect();

            BuildAllyList();
            Log("[DA2] ability %08X PARTY-targetable (mask %08X) -- ally select ON "
                "for %us: D-Pad (any direction) chooses, circle casts, cross cancels\n",
                ArmedAbility(), mask, ALLY_SELECT_MS / 1000);
            return false;
        }
        Log("[DA2] ability %08X armed, mask %08X -- hostile targeting\n",
            ArmedAbility(), mask);
        for (int k = 0; k < 8; ++k) {
            SendCommand(L"PerformAbilityOnSelectedTargetMessage");
            if (!ArmedAbility()) {
                if (k) Log("[DA2] ability fired after %d retarget(s)\n", k);
                return true;
            }
            if (!CycleTarget(L"SelectNextHostileTargetMessage", "ability retarget"))
                break;
        }
        Log("[DA2] ability %08X still armed -- no legal target found (mask %08X)\n",
            ArmedAbility(), mask);
        return false;
    };

    auto fireSlot = [&](const wchar_t* slot) {
        const uint32_t t0 = CurrentTarget();
        Log("[DA2] quickslot press %S: target=%08X armed=%08X friendly=%d rt=%d\n",
            slot, t0, ArmedAbility(), (int)friendly, (int)rt);

        if (!friendly && ArmedAbility()) { tryFireArmed(); return; }
        if (t0 == 0xFFFFFFFF)
            CycleTarget(L"SelectNextHostileTargetMessage", "auto-target before quickslot");
        InvokeAS2(T_HUD, L"BattleMenu.ActivateQuickslot", slot);
        armedAt = GetTickCount();
    };

    if (!g_radialVisible) {
        if (bx && !g_bx) fireSlot(slot1);
        if (by && !g_by) fireSlot(slot2);
        if (bb && !g_bb) fireSlot(slot3);
    }

    if (armedAt) {
        if (ArmedAbility()) {
            armedAt = 0;

            if (friendly) SendCommand(L"PerformFriendlyCastMessage");
            else          tryFireArmed();
        } else if (GetTickCount() - armedAt > 1500) {

            armedAt = 0;

            static bool said = false;
            if (!said) { said = true;
                Log("[DA2] quickslot fired without arming a pending target (normal for direct abilities)\n"); }
        }
    }
    g_bx = bx; g_by = by; g_bb = bb;

    if (a && !g_aPrev && !g_radialVisible) {

        int ty = -1;
        if (!friendly) {
            static const char* kType[20] = {
                "INVALID","GUI","TILE","MODULE","AREA","STORE","CREATURE","ITEM",
                "TRIGGER","PROJECTILE","PLACEABLE","?","AREAOFEFFECT","WAYPOINT",
                "?","SOUND","PARTY","MAPLOCATION","VFX","MAP" };
            const uint32_t tid = CurrentTarget();
            __try {
                void* app = AppSingleton();
                if (app && tid != 0xFFFFFFFF) {
                    void* o = reinterpret_cast<ResolveObject_t>(R(0x005A70C0))(app, nullptr, tid);
                    if (o) ty = *reinterpret_cast<uint8_t*>((uintptr_t)o + 0x14);
                }
            } __except (EXCEPTION_EXECUTE_HANDLER) { ty = -1; }
            Log("[DA2] action on %08X type=%d (%s)\n", tid, ty,
                (ty >= 0 && ty < 20) ? kType[ty] : "unresolved");
        }
        if (friendly) {
            SendCommand(L"PerformFriendlyCastMessage");
        } else {

            const uint32_t before = CurrentTarget();

            bool primaryDidIt = false;
            if (g_autoAim) {

                const uint32_t aimed = HoveredObjId();
                InterlockedExchange(&g_primaryRan, 0);
                SendCommand(L"PerformPrimaryActionOnTargetUnderCursorMessage");
                primaryDidIt = InterlockedCompareExchange(&g_primaryRan, 0, 0) != 0;
                Log("[DA2] primary (auto-aim) on %08X -> %s\n",
                    aimed, primaryDidIt ? "PERFORMED" : "refused");
            } else if (before != 0xFFFFFFFF) {
                const uint32_t wasHovered = HoveredObjId();
                InterlockedExchange(&g_primaryRan, 0);
                SetHoveredObjId(before);
                SendCommand(L"PerformPrimaryActionOnTargetUnderCursorMessage");
                SetHoveredObjId(wasHovered);
                primaryDidIt = InterlockedCompareExchange(&g_primaryRan, 0, 0) != 0;
                Log("[DA2] primary-under-cursor on %08X -> %s\n",
                    before, primaryDidIt ? "PERFORMED" : "refused");
            }

            if (!primaryDidIt) SendCommand(L"PerformDefaultActionOnTargetMessage");

            if (!primaryDidIt && before != 0xFFFFFFFF && CurrentTarget() == 0xFFFFFFFF) {

                SetCurrentTargetDirect(before);
                const wchar_t* verb = (ty == 6) ? L"PerformSecondaryActionOnTargetMessage"
                                                : L"PerformUseObjectMessage";
                SendCommand(verb);

                if (ty != 6 && CurrentTarget() != 0xFFFFFFFF) {
                    SetCurrentTargetDirect(before);
                    SendCommand(L"UsePlotActionMessage");
                }
                Log("[DA2] %08X (type=%d) refused default -> sent %S%s\n",
                    before, ty, verb,
                    (ty != 6) ? " + UsePlotActionMessage" : "");
            }
        }
    }
    g_aPrev = a;

}

static void SynthKey(WORD vk) {
    keybd_event((BYTE)vk, 0, 0, 0);
    keybd_event((BYTE)vk, 0, KEYEVENTF_KEYUP, 0);
}

static DWORD WINAPI TitleAndMovieSkipThread(LPVOID) {
    bool prevStart = false, prevA = false;
    for (;;) {
        Sleep(40);

        UpdateConsolePause();

        {
            static bool prevF6 = false;
            const bool f6 = (GetAsyncKeyState(VK_F6) & 0x8000) != 0;
            if (f6 && !prevF6) {
                InterlockedExchange(&g_pauseHeld, 0);
                SendCommand(L"MemoryUnpauseMessage");
                SendCommand(L"UnpauseMessage");
                Log("[DA2] F6 -- forced unpause\n");
            }
            prevF6 = f6;
        }

        XINPUT_STATE st{};
        if (!ReadPad(st)) { prevStart = prevA = false; continue; }

        const DWORD now = GetTickCount();
        const DWORD QUIET_MS = 400;

        const bool ourPause = InterlockedCompareExchange(&g_pauseHeld, 0, 0) != 0;
        bool gameplayLive = ourPause
            || (now - InterlockedCompareExchange(&g_lastGameplayMs, 0, 0)) < QUIET_MS;
        if (gameplayLive) {
            prevStart = (st.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
            prevA     = (st.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
            continue;
        }

        bool s = (st.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
        bool a = (st.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;

        if (s && !prevStart) {

            if (ResolveMovieView(L"PreStartMenu")
             || InterlockedCompareExchange(&g_lastScreenOpen, 68, 68) == 68) {

                Log("[DA2] START on PreStartMenu -> PreStartContinue\n");
                if (!SendCommand(L"PreStartContinueMessage"))
                    SendCommand(L"SkipMovieMessage");
            } else if (!SendCommand(L"SkipMovieMessage")
                    && !SendCommand(L"SkipConversationMessage")) {
                SynthKey(VK_ESCAPE);
            }
        }

        if (a && !prevA && !InterlockedCompareExchange(&g_screen1Live, 0, 0)) {
            Log("[DA2] any-key (A)\n");
            SynthKey(VK_SPACE);
            SynthKey(VK_RETURN);
        }
        prevStart = s; prevA = a;
    }
}

static bool Arm(uintptr_t addr, void* detour, LPVOID* orig, const char* what) {
    void* t = reinterpret_cast<void*>(R(addr));
    if (MH_CreateHook(t, detour, orig) != MH_OK || MH_QueueEnableHook(t) != MH_OK) {
        Log("[DA2] HOOK FAILED: %s @0x%08X\n", what, (unsigned)(uintptr_t)t);
        return false;
    }
    Log("[DA2] hook queued: %s @0x%08X\n", what, (unsigned)(uintptr_t)t);
    return true;
}

static int g_consoleVersion = 1;

static bool ModuleDir(wchar_t* out, size_t n) {
    HMODULE self = nullptr;
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                            reinterpret_cast<LPCWSTR>(&ModuleDir), &self) || !self)
        return false;
    if (!GetModuleFileNameW(self, out, (DWORD)n)) return false;
    wchar_t* slash = wcsrchr(out, L'\\');
    if (!slash) return false;
    *(slash + 1) = L'\0';
    return true;
}

static bool DirExists(const wchar_t* p) {
    const DWORD a = GetFileAttributesW(p);
    return a != INVALID_FILE_ATTRIBUTES && (a & FILE_ATTRIBUTE_DIRECTORY);
}

static bool FindOverrideDir(const wchar_t* dllDir, const wchar_t* ini,
                            wchar_t* out, size_t n) {

    wchar_t told[MAX_PATH]{};
    GetPrivateProfileStringW(L"DA2", L"Override path", L"", told, MAX_PATH, ini);
    if (told[0]) {
        const size_t L = wcslen(told);
        swprintf_s(out, n, L"%s%s", told, (L && told[L - 1] == L'\\') ? L"" : L"\\");
        if (DirExists(out)) { Log("[DA2] ini: override from 'Override path'\n"); return true; }
        Log("[DA2] ini: 'Override path' = %ls does not exist\n", told);
    }

    wchar_t exeDir[MAX_PATH]{};
    if (GetModuleFileNameW(nullptr, exeDir, MAX_PATH)) {
        wchar_t* slash = wcsrchr(exeDir, L'\\');
        if (slash) *(slash + 1) = L'\0';
    }

    static const wchar_t* kRel[] = {
        L"packages\\core\\override\\",
        L"..\\packages\\core\\override\\",
        L"..\\..\\packages\\core\\override\\",
        L"override\\",
    };
    const wchar_t* bases[2] = { exeDir[0] ? exeDir : nullptr, dllDir };
    for (const wchar_t* base : bases) {
        if (!base) continue;
        for (const wchar_t* rel : kRel) {
            swprintf_s(out, n, L"%s%s", base, rel);
            Log("[DA2] ini:   try %ls\n", out);
            if (DirExists(out)) { Log("[DA2] ini: override found at %ls\n", out); return true; }
        }
    }

    wchar_t docs[MAX_PATH]{};
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_PERSONAL, nullptr, 0, docs))) {
        wchar_t glob[MAX_PATH];
        swprintf_s(glob, L"%s\\BioWare\\*", docs);
        WIN32_FIND_DATAW fd{};
        HANDLE h = FindFirstFileW(glob, &fd);
        if (h != INVALID_HANDLE_VALUE) {
            do {
                if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) continue;
                if (fd.cFileName[0] == L'.') continue;
                swprintf_s(out, n, L"%s\\BioWare\\%s\\packages\\core\\override\\",
                           docs, fd.cFileName);
                Log("[DA2] ini:   try %ls\n", out);
                if (DirExists(out)) {
                    FindClose(h);
                    Log("[DA2] ini: override found at %ls\n", out);
                    return true;
                }
            } while (FindNextFileW(h, &fd));
            FindClose(h);
        } else {
            Log("[DA2] ini:   no %s\\BioWare\\ folder\n", docs);
        }
    }

    Log("[DA2] ini: no override folder found -- see the 'try' lines above.\n");
    Log("[DA2]     add 'Override path=<full path>' to da2_console.ini to set it\n");
    return false;
}

static void ApplyConsoleVersion() {
    wchar_t dir[MAX_PATH]{};
    if (!ModuleDir(dir, MAX_PATH)) { Log("[DA2] ini: could not locate the DLL directory\n"); return; }

    wchar_t exeDir[MAX_PATH]{};
    if (GetModuleFileNameW(nullptr, exeDir, MAX_PATH)) {
        wchar_t* slash = wcsrchr(exeDir, L'\\');
        if (slash) *(slash + 1) = L'\0';
    }

    wchar_t ini[MAX_PATH]{};
    bool haveIni = false;
    const wchar_t* iniBases[2] = { exeDir[0] ? exeDir : nullptr, dir };
    for (const wchar_t* base : iniBases) {
        if (!base) continue;
        swprintf_s(ini, L"%sda2_console.ini", base);
        if (GetFileAttributesW(ini) != INVALID_FILE_ATTRIBUTES) { haveIni = true; break; }
    }

    if (!haveIni) {
        Log("[DA2] ini: no da2_console.ini beside the game exe or the DLL"
            " -- Console version=1 (PS3)\n");
        if (exeDir[0]) Log("[DA2]     put it in %ls\n", exeDir);
        return;
    }
    Log("[DA2] ini: reading %ls\n", ini);
    g_consoleVersion = (int)GetPrivateProfileIntW(L"DA2", L"Console version", 1, ini);
    if (g_consoleVersion != 1 && g_consoleVersion != 2) {
        Log("[DA2] ini: Console version=%d is not 1 or 2 -- using 1 (PS3)\n", g_consoleVersion);
        g_consoleVersion = 1;
    }
    Log("[DA2] ini: Console version=%d (%s)\n", g_consoleVersion,
        g_consoleVersion == 2 ? "Xbox 360" : "PS3");

    wchar_t ov[MAX_PATH]{};
    if (!FindOverrideDir(dir, ini, ov, MAX_PATH)) {
        Log("[DA2] ini: glyph set unchanged\n");
        return;
    }

    wchar_t src[MAX_PATH]{}, dst[MAX_PATH]{};
    swprintf_s(src, L"%satl_ps3_dxt5_dat.%s.xml", ov, g_consoleVersion == 2 ? L"xbox" : L"ps3");
    swprintf_s(dst, L"%satl_ps3_dxt5_dat.xml", ov);
    if (GetFileAttributesW(src) == INVALID_FILE_ATTRIBUTES) {
        Log("[DA2] ini: %ls is not installed -- glyph set unchanged\n", src);
        return;
    }

    bool same = false;
    HANDLE hs = CreateFileW(src, GENERIC_READ, FILE_SHARE_READ, nullptr,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    HANDLE hd = CreateFileW(dst, GENERIC_READ, FILE_SHARE_READ, nullptr,
                            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hs != INVALID_HANDLE_VALUE && hd != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER ls{}, ld{};
        if (GetFileSizeEx(hs, &ls) && GetFileSizeEx(hd, &ld) && ls.QuadPart == ld.QuadPart) {
            same = true;
            static BYTE ba[8192], bb[8192];
            DWORD ra = 0, rb = 0;
            while (same && ReadFile(hs, ba, sizeof(ba), &ra, nullptr) && ra) {
                if (!ReadFile(hd, bb, sizeof(bb), &rb, nullptr) || rb != ra ||
                    memcmp(ba, bb, ra) != 0) same = false;
            }
        }
    }
    if (hs != INVALID_HANDLE_VALUE) CloseHandle(hs);
    if (hd != INVALID_HANDLE_VALUE) CloseHandle(hd);

    const wchar_t* which = (g_consoleVersion == 2) ? L"Xbox" : L"PS3";
    if (same) {
        Log("[DA2] ini: %ls glyph coordinates already installed -- ACTIVE this session\n", which);
    } else if (CopyFileW(src, dst, FALSE)) {
        Log("[DA2] ini: %ls glyph coordinates written.\n", which);
        Log("[DA2]     NOT active this session -- the game reads the atlas at startup,\n");
        Log("[DA2]     before injection. RESTART the game to see them.\n");
    } else {
        Log("[DA2] ini: copy failed (%lu) -- glyph set unchanged\n", GetLastError());
    }
}

static bool ClaimSoleInstance() {
    HANDLE h = CreateMutexW(nullptr, FALSE, L"Local\\DA2ConsoleMod.SoleInstance");
    if (!h) return true;
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        Log("[DA2] ==========================================================\n");
        Log("[DA2] ALREADY LOADED IN THIS PROCESS -- this copy is doing nothing.\n");
        Log("[DA2] You have dinput8.dll in bin_ship AND ran the injector.\n");
        Log("[DA2] Use ONE: delete dinput8.dll, or stop injecting.\n");
        Log("[DA2] Two copies double every button press.\n");
        Log("[DA2] ==========================================================\n");
        CloseHandle(h);
        return false;
    }
    return true;

}

static DWORD WINAPI InitThread(LPVOID) {

    Sleep(750);

    if (!ClaimSoleInstance()) return 0;

    g_delta = reinterpret_cast<uintptr_t>(GetModuleHandleA(NULL)) - PREFERRED_BASE;
    Log("[DA2] console mode init. delta=0x%08X\n", (unsigned)g_delta);

    Log("[DA2] build " __DATE__ " " __TIME__ "\n");

    ApplyConsoleVersion();

    CreateThread(nullptr, 0, SpoofThread, nullptr, 0, nullptr);
    CreateThread(nullptr, 0, TitleAndMovieSkipThread, nullptr, 0, nullptr);

    if (MH_Initialize() != MH_OK) { Log("[DA2] MH_Initialize FAILED\n"); return 1; }

    if (MH_CreateHookApi(L"user32", "SetCursor", &hkSetCursor,
                         (LPVOID*)&oSetCursorApi) == MH_OK &&
        MH_QueueEnableHook(MH_ALL_HOOKS) == MH_OK) {
        Log("[DA2] hook queued: user32!SetCursor (hide the arrow)\n");
    } else {
        Log("[DA2] HOOK FAILED: user32!SetCursor -- the cursor will stay visible\n");
    }

    if (MH_CreateHookApi(L"xinput1_3", "XInputSetState", &hkXInputSetState,
                         (LPVOID*)&oXInputSetState) == MH_OK &&
        MH_QueueEnableHook(MH_ALL_HOOKS) == MH_OK) {
        Log("[DA2] hook queued: xinput1_3!XInputSetState (rumble watch)\n");
    } else {

        Log("[DA2] rumble watch: xinput1_3!XInputSetState not hookable\n");
    }

    Arm(ADDR_OPEN_SCREEN,     &hkOpenScreen, (LPVOID*)&oOpenScreen, "OpenScreenByType");
    Arm(ADDR_MOVEMENT_READER, &hkMoveReader, (LPVOID*)&oMoveReader, "movement/poll");
    Arm(ADDR_ACTION_ANALOG,   &hkActionAnalog,  (LPVOID*)&oActionAnalog,  "action analog");
    Arm(ADDR_ACTION_DIGITAL,  &hkActionDigital, (LPVOID*)&oActionDigital, "action digital");
    Arm(ADDR_PICKER_PICK,     &hkPickerPick,    (LPVOID*)&oPickerPick,    "partypicker pick");
    Arm(0x00623830,           &hkShowDialog,    (LPVOID*)&oShowDialog,    "ShowDialogBox handler");

    Arm(0x0076B880,           &hkInvokeAS,      (LPVOID*)&oInvokeAS,      "Movie::InvokeAS (tutorial swap)");
    Arm(ADDR_FSCMD_SENDMESSAGE, &hkFsSendMessage, (LPVOID*)&oFsSendMessage, "fscommand SendMessage (AS2 trace pipe)");
    Arm(ADDR_RESOLVE_MOVIE,   &hkResolveMovie,  (LPVOID*)&oResolveMovie,  "ResolveMovie (name probe)");
    Arm(ADDR_DISPATCH,        &hkDispatch,      (LPVOID*)&oDispatch,      "Dispatch (rumble + radial close)");
    Arm(ADDR_CURSOR_POS,        &hkCursorPos,        (LPVOID*)&oCursorPos,        "cursor position (auto-aim feed)");
    Arm(ADDR_HOVER_UPDATE,      &hkHoverUpdate,      (LPVOID*)&oHoverUpdate,      "hover updater (capture owner)");
    Arm(ADDR_EXPLORE_PRIMARY,   &hkExplorePrimary,   (LPVOID*)&oExplorePrimary,   "GameModeExplore primary action");
    Arm(ADDR_EXPLORE_SECONDARY, &hkExploreSecondary, (LPVOID*)&oExploreSecondary, "GameModeExplore secondary action");

    if (MH_ApplyQueued() != MH_OK) {
        Log("[DA2] MH_ApplyQueued FAILED -- no hooks are live\n");
        return 1;
    }
    Log("[DA2] all hooks enabled in one pass\n");

    Log("[DA2] Lstick=move Rstick=camera DPad=change target LB/RB=party\n");
    Log("[DA2] L2=radial (A=use) R2=page swap  R3=target lock  L3=friendly/hostile\n");
    Log("[DA2] START=main menu  SELECT=map  (watch for \"factory NULL\" if unregistered)\n");
    return 0;
}

void StartConsoleMod() {
    CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr);
}

#ifndef DA2_PROXY_BUILD
BOOL APIENTRY DllMain(HMODULE h, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(h);
        StartConsoleMod();
    }
    return TRUE;
}
#endif
