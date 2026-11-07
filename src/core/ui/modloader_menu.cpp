// core/ui/modloader_menu.cpp - the main VR settings in Dishonored Mod Loader's pause-menu MODS list.
// Included by the unity build (mod/dishonoredvr.cpp) after every setter it calls.
//
// The Mod Loader (Crimsab, Nexus mod 383) loads Binaries\Win32\Mods\plugins\*.dll. Our plugin
// (src/tools/modmenu_plugin, DishonoredVR.dll) only forwards: its Plg_QueryMenu returns what
// DvrModMenuQuery returns here. Without the Mod Loader the plugin is never loaded, nothing calls
// these exports, and DvrModMenuTick finds nothing pending.
//
// The menu is the loader's typed menu, version 3 (36-byte header, 36-byte items, groups):
// item type 0 = toggle, 1 = choice list, 3 = slider. Layout read from the loader's validator.
//
// Threading: get and set arrive on the game's UI thread. Get reads the same values the F10 panel
// reads. Set only stores a pending value; DvrModMenuTick applies it on the present thread, the
// thread the F10 panel writes from, through the same setters, and saves the same ini key.
#pragma once

namespace dvr::modmenu {

struct Item { int type; const wchar_t* label; int count; const wchar_t* const* choices;
              int minV, maxV, step; const wchar_t* unit; int group; };
struct Menu { uint32_t size, version, flags, count; const Item* items;
              int (__cdecl* get)(int); void (__cdecl* set)(int, int);
              uint32_t groupCount; const wchar_t* const* groups; };
static_assert(sizeof(Item) == 0x24, "Mod Loader v3 item is 36 bytes");
static_assert(sizeof(Menu) == 0x24, "Mod Loader v3 menu header is 36 bytes");

enum Id { CROUCH, SNAP, FOV, AIM_FIRE, AIM_BLINK, AIM_INTERACT, AIM_THROW, AIM_GADGET, AIM_CARRY,
          AIM_POWERS, PICKUP, DPAD, PAUSE_CHORD, QUICK_SAVE, QUICK_LOAD, PANEL_CHORD, COUNT };

static const wchar_t* const kGroups[] = { L"Comfort", L"Aim from the controller", L"Controls" };
static const wchar_t* const kDpad[]   = { L"Off", L"Right thumbrest", L"R3 (right stick click)", L"Left thumbrest" };
static const int            kDpadMode[] = { dvr::controller::Off, dvr::controller::RightRest,
                                            dvr::controller::R3, dvr::controller::LeftRest };
static const wchar_t* const kPartner[] = { L"Off", L"A", L"B" };

static const Item kItems[COUNT] = {
    { 0, L"Real crouching crouches Corvo",      0, nullptr, 0, 0, 0, nullptr, 0 },
    { 0, L"Snap turn",                          0, nullptr, 0, 0, 0, nullptr, 0 },
    { 3, L"Gameplay field of view",             0, nullptr, 60, 120, 1, L" deg", 0 },
    { 0, L"Crossbow and pistol",                0, nullptr, 0, 0, 0, nullptr, 1 },
    { 0, L"Blink",                              0, nullptr, 0, 0, 0, nullptr, 1 },
    { 0, L"Interactions",                       0, nullptr, 0, 0, 0, nullptr, 1 },
    { 0, L"Grenades",                           0, nullptr, 0, 0, 0, nullptr, 1 },
    { 0, L"Spring razors",                      0, nullptr, 0, 0, 0, nullptr, 1 },
    { 0, L"Carried objects (throw)",            0, nullptr, 0, 0, 0, nullptr, 1 },
    { 0, L"Windblast, Possession, Swarm",       0, nullptr, 0, 0, 0, nullptr, 1 },
    { 0, L"Pick up with your hand",             0, nullptr, 0, 0, 0, nullptr, 1 },
    { 1, L"D-pad modifier",                     4, kDpad, 0, 0, 0, nullptr, 2 },
    { 0, L"X + Y pause chord",                  0, nullptr, 0, 0, 0, nullptr, 2 },
    { 1, L"Quick save: Y +",                    3, kPartner, 0, 0, 0, nullptr, 2 },
    { 1, L"Quick load: Y +",                    3, kPartner, 0, 0, 0, nullptr, 2 },
    { 0, L"Y + right stick click opens F10",    0, nullptr, 0, 0, 0, nullptr, 2 },
};

static volatile LONG g_pending[COUNT];   // 0 = nothing pending, else value + 1
static volatile LONG g_any = 0;

static int dpad_index(int mode) { for (int i = 0; i < 4; ++i) if (kDpadMode[i] == mode) return i; return 0; }

static int get_value(int id)
{
    const LONG p = id >= 0 && id < COUNT ? g_pending[id] : 0;
    if (p) return (int)(p - 1);   // show a choice the player just made before the next frame applies it
    const auto c = dvr::controller::config();
    switch (id) {
    case CROUCH:       return g_crouchOn ? 1 : 0;
    case SNAP:         return dvr::snap::enabled() ? 1 : 0;
    case FOV:        { const float f = ProjectionFovGet(); return f > 0 ? (int)lroundf(f) : 103; }
    case AIM_FIRE:     return FireAimEnabled() ? 1 : 0;
    case AIM_BLINK:    return g_blkDriveUI ? 1 : 0;
    case AIM_INTERACT: return InteractAimEnabled() ? 1 : 0;
    case AIM_THROW:    return ThrowAimEnabled() ? 1 : 0;
    case AIM_GADGET:   return GadgetAimEnabled() ? 1 : 0;
    case AIM_CARRY:    return CarryThrowAimEnabled() ? 1 : 0;
    case AIM_POWERS:   return PowerAimEnabled() ? 1 : 0;
    case PICKUP:       return PickupEnabled() ? 1 : 0;
    case DPAD:         return dpad_index(c.modifier);
    case PAUSE_CHORD:  return c.pauseChord ? 1 : 0;
    case QUICK_SAVE:   return c.quickSave;
    case QUICK_LOAD:   return c.quickLoad;
    case PANEL_CHORD:  return c.panelChord ? 1 : 0;
    }
    return 0;
}

static void apply(int id, int v)
{
    const char* who = "MODS menu";
    const bool on = v != 0;
    switch (id) {
    case CROUCH:       g_crouchOn = on; ConfigWriteKey("Tracking", "PhysicalCrouch", on ? "1" : "0", who); break;
    case SNAP:         dvr::snap::set_enabled(on, who); ConfigWriteKey("Turning", "SnapTurn", on ? "1" : "0", who); break;
    case FOV: {        const float f = (float)(v < 60 ? 60 : v > 120 ? 120 : v); ProjectionFovSet(f);
                       char b[16]; _snprintf(b, sizeof(b), "%.2f", ProjectionFovGet()); b[15] = 0;
                       ConfigWriteKey("Screen", "ProjectionFov", b, who); break; }
    case AIM_FIRE:     FireAimSet(on, who); ConfigWriteKey("Aim", "FireFromHand", on ? "1" : "0", who); break;
    case AIM_BLINK:    g_blkDriveUI = on; g_blkAimOnCfg = on;
                       if (on) g_blkDstReqUI = g_blkDstOnUI ? 0 : 1;
                       ConfigWriteKey("Blink", "ControllerAim", on ? "1" : "0", who);
                       Log("blink: controller aim %s (%s)", on ? "ON" : "off", who); break;
    case AIM_INTERACT: InteractAimSet(on, who); ConfigWriteKey("Aim", "InteractFromHand", on ? "1" : "0", who); break;
    case AIM_THROW:    ThrowAimSet(on, who); ConfigWriteKey("Aim", "ThrowFromHand", on ? "1" : "0", who); break;
    case AIM_GADGET:   GadgetAimSet(on, who); ConfigWriteKey("Aim", "GadgetFromHand", on ? "1" : "0", who); break;
    case AIM_CARRY:    CarryThrowAimSet(on, who); ConfigWriteKey("Aim", "CarryThrowFromHand", on ? "1" : "0", who); break;
    case AIM_POWERS:   PowerAimSet(on, who); ConfigWriteKey("Aim", "PowersFromHand", on ? "1" : "0", who); break;
    case PICKUP:       PickupSet(on, who); ConfigWriteKey("Aim", "PhysicalPickup", on ? "1" : "0", who); break;
    case DPAD: case PAUSE_CHORD: case QUICK_SAVE: case QUICK_LOAD: case PANEL_CHORD: {
        auto c = dvr::controller::config();
        if (id == DPAD)        c.modifier = kDpadMode[v >= 0 && v < 4 ? v : 0];
        if (id == PAUSE_CHORD) c.pauseChord = on;
        if (id == QUICK_SAVE)  c.quickSave = v;
        if (id == QUICK_LOAD)  c.quickLoad = v;
        if (id == PANEL_CHORD) c.panelChord = on;
        c = dvr::controller::normalize(c);
        dvr::controller::configure(c);
        char value[16]; _snprintf(value, sizeof(value), "%d", c.modifier); value[15] = 0;
        ConfigWriteKey("Controllers", "DpadModifier", value, who);
        ConfigWriteKey("Controllers", "PauseChord", c.pauseChord ? "1" : "0", who);
        ConfigWriteKey("Controllers", "QuickSave", dvr::controller::partner_key(c.quickSave), who);
        ConfigWriteKey("Controllers", "QuickLoad", dvr::controller::partner_key(c.quickLoad), who);
        ConfigWriteKey("Controllers", "PanelChord", c.panelChord ? "1" : "0", who);
        break; }
    }
}

static void tick()
{
    if (!InterlockedExchange(&g_any, 0)) return;
    for (int i = 0; i < COUNT; ++i) {
        const LONG p = InterlockedExchange(&g_pending[i], 0);
        if (p) apply(i, (int)(p - 1));
    }
}

static int  __cdecl menu_get(int id) { return get_value(id); }
static void __cdecl menu_set(int id, int v)
{
    if (id < 0 || id >= COUNT) return;
    InterlockedExchange(&g_pending[id], (LONG)v + 1);
    InterlockedExchange(&g_any, 1);
}

static const Menu kMenu = { sizeof(Menu), 3, 1 /* no master toggle: the VR mod is not switched off here */,
                            COUNT, kItems, &menu_get, &menu_set,
                            (uint32_t)(sizeof(kGroups) / sizeof(kGroups[0])), kGroups };

} // namespace dvr::modmenu

static void DvrModMenuTick() { dvr::modmenu::tick(); }

extern "C" const void* __cdecl DvrModMenuQuery()
{
    static volatile LONG told = 0;
    if (!InterlockedExchange(&told, 1)) Log("modmenu: the Mod Loader plugin asked for the menu (%d entries)", (int)dvr::modmenu::COUNT);
    return &dvr::modmenu::kMenu;
}
