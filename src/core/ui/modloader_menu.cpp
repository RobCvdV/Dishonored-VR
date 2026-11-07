// core/ui/modloader_menu.cpp - the VR settings in Dishonored Mod Loader's pause-menu MODS list.
// Included by the unity build (mod/dishonoredvr.cpp) after every setter it calls.
//
// The Mod Loader (Crimsab, Nexus mod 383) loads Binaries\Win32\Mods\plugins\*.dll. Our plugin
// (src/tools/modmenu_plugin, DishonoredVR.dll) only forwards: its Plg_QueryMenu returns what
// DvrModMenuQuery returns here. Without the Mod Loader the plugin is never loaded, nothing calls
// these exports, and DvrModMenuTick finds nothing pending.
//
// The menu is the loader's typed menu, version 3 (36-byte header, 36-byte items, at most 48
// items in at most 8 groups): item type 0 = toggle, 1 = choice list, 3 = slider. Layout read from
// the loader's own validator. Only settings that do not need watching while they change are here;
// hand and HUD placement, sizes and other live tuning stay in the F10 panel.
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

enum Group { G_DISPLAY, G_COMFORT, G_AIM, G_HANDS, G_CONTROLS, G_CINE, G_HUD, G_COUNT };
static const wchar_t* const kGroups[G_COUNT] = {
    L"Display", L"Comfort and movement", L"Aiming", L"Hands and grabbing", L"Controls",
    L"Cutscenes and peeking", L"HUD" };

enum Id {
    RES, UPSCALE_ON, UPSCALER, UPSCALE_MODE, SHARPEN, FOV, MIRROR_OFF, RAIN_HIDE,
    CROUCH, SNAP, SNAP_ANGLE, POSTRACK, HEAD_MOVE,
    DOT, LASER, RETICLE_HAND, AIM_FIRE, AIM_BLINK, AIM_INTERACT, AIM_THROW, AIM_GADGET, AIM_CARRY, AIM_POWERS,
    PICKUP, DOORS, CARRY, USABLES, ANIM_MANTLE, ANIM_TAKEDOWN, ANIM_CINE,
    DPAD, DPAD_FLIP, PAUSE_CHORD, QUICK_SAVE, QUICK_LOAD, PANEL_CHORD,
    CINE_PITCH, CINE_ROLL, CINE_FOV, CINE_FOV_MATCH, CINE_BORDERS, SPECIAL_HEAD,
    HUD_PANELS, COUNT };
static_assert(COUNT <= 48, "the Mod Loader takes at most 48 entries");

static const wchar_t* const kDpad[]     = { L"Off", L"Right thumbrest", L"R3 (right stick click)", L"Left thumbrest" };
static const int            kDpadMode[] = { dvr::controller::Off, dvr::controller::RightRest,
                                            dvr::controller::R3, dvr::controller::LeftRest };
static const wchar_t* const kPartner[]  = { L"Off", L"A", L"B" };
static const wchar_t* const kUpscaler[] = { L"NVIDIA DLSS", L"AMD FSR" };
static const wchar_t* const kHand[]     = { L"Left", L"Right" };
static wchar_t              g_modeNames[dvr::dlss::QCount][48];
static const wchar_t*       g_modeList[dvr::dlss::QCount];

#define T(label, g)                 { 0, label, 0, nullptr, 0, 0, 0, nullptr, g }
#define C(label, n, list, g)        { 1, label, n, list, 0, 0, 0, nullptr, g }
#define S(label, lo, hi, st, u, g)  { 3, label, 0, nullptr, lo, hi, st, u, g }
static Item g_items[COUNT] = {
    S(L"Render resolution (total pixels)", 50, 300, 10, L"%", G_DISPLAY),
    T(L"Upscaling / anti-aliasing", G_DISPLAY),
    C(L"Upscaler", 2, kUpscaler, G_DISPLAY),
    C(L"Upscaler mode", dvr::dlss::QCount, g_modeList, G_DISPLAY),
    S(L"Sharpening", 0, 100, 5, L"%", G_DISPLAY),
    S(L"Gameplay field of view", 60, 120, 1, L" deg", G_DISPLAY),
    T(L"Disable desktop mirror (faster)", G_DISPLAY),
    T(L"Hide close rain overlay", G_DISPLAY),

    T(L"Real crouching crouches Corvo", G_COMFORT),
    T(L"Snap turn", G_COMFORT),
    S(L"Snap turn angle", 15, 90, 15, L" deg", G_COMFORT),
    T(L"Positional head tracking", G_COMFORT),
    T(L"Walk where you look", G_COMFORT),

    T(L"Aim dot", G_AIM),
    T(L"Aim laser", G_AIM),
    C(L"Aim dot hand", 2, kHand, G_AIM),
    T(L"Controller aims: crossbow and pistol", G_AIM),
    T(L"Controller aims: Blink", G_AIM),
    T(L"Controller aims: interactions", G_AIM),
    T(L"Controller aims: grenades", G_AIM),
    T(L"Controller aims: spring razors", G_AIM),
    T(L"Controller aims: carried objects", G_AIM),
    T(L"Controller aims: Windblast, Possession, Swarm", G_AIM),

    T(L"Pick up loot by reaching for it", G_HANDS),
    T(L"Open doors by grabbing them", G_HANDS),
    T(L"Carry and throw objects by grabbing", G_HANDS),
    T(L"Use levers, switches and traps by grabbing", G_HANDS),
    T(L"Game hands while mantling", G_HANDS),
    T(L"Hide arms in takedowns", G_HANDS),
    T(L"Game hands and arms in cinematics", G_HANDS),

    C(L"D-pad modifier", 4, kDpad, G_CONTROLS),
    T(L"D-pad on the right stick", G_CONTROLS),
    T(L"X + Y pause chord", G_CONTROLS),
    C(L"Quick save: Y +", 3, kPartner, G_CONTROLS),
    C(L"Quick load: Y +", 3, kPartner, G_CONTROLS),
    T(L"Y + right stick click opens F10", G_CONTROLS),

    T(L"Suppress animation up/down tilt", G_CINE),
    T(L"Suppress cinematic roll", G_CINE),
    T(L"Suppress cinematic FOV zoom", G_CINE),
    T(L"Cutscenes use the gameplay FOV", G_CINE),
    T(L"Hide cinematic black borders", G_CINE),
    T(L"Natural head look in lean and keyholes", G_CINE),

    T(L"HUD on its own panels", G_HUD),
};
#undef T
#undef C
#undef S

static volatile LONG g_pending[COUNT];   // 0 = nothing pending, else value + 1
static volatile LONG g_any = 0;

static int dpad_index(int mode) { for (int i = 0; i < 4; ++i) if (kDpadMode[i] == mode) return i; return 0; }
static int mode_index(int q) { for (int i = 0; i < dvr::dlss::QCount; ++i) if (dvr::dlss::kQualityOrder[i] == q) return i; return 0; }
static int b(bool v) { return v ? 1 : 0; }
static const char* s01(bool v) { return v ? "1" : "0"; }

static int res_percent()
{
    const float p = g_resWantW && g_resWantH
        ? 100.0f * ((float)g_resWantW / 2750.0f) * ((float)g_resWantH / 2850.0f) : 100.0f;
    return (int)lroundf(p / 10.0f) * 10;
}

static int get_value(int id)
{
    const LONG p = id >= 0 && id < COUNT ? g_pending[id] : 0;
    if (p) return (int)(p - 1);   // show a choice the player just made before the next frame applies it
    const auto c = dvr::controller::config();
    const auto rc = dvr::aim::config();
    switch (id) {
    case RES:            return res_percent();
    case UPSCALE_ON:     return b(dvr::dlss::mode() != dvr::dlss::ModeOff);
    case UPSCALER:       return b(dvr::dlss::backend() == dvr::dlss::BackendFsr);
    case UPSCALE_MODE:   return mode_index(dvr::dlss::quality());
    case SHARPEN:        return (int)lroundf(dvr::clarity::sharpen() * 100.0f);
    case FOV:          { const float f = ProjectionFovGet(); return f > 0 ? (int)lroundf(f) : 103; }
    case MIRROR_OFF:     return b(dvr::desktop_eye::mirror_off());
    case RAIN_HIDE:      return b(RainHideEnabled());
    case CROUCH:         return b(g_crouchOn);
    case SNAP:           return b(dvr::snap::enabled());
    case SNAP_ANGLE:     return (int)lroundf(dvr::snap::angle_deg());
    case POSTRACK:       return b(g_posTrack);
    case HEAD_MOVE:      return b(HeadMovementEnabled());
    case DOT:            return b(rc.dot);
    case LASER:          return b(rc.laser);
    case RETICLE_HAND:   return rc.hand ? 1 : 0;
    case AIM_FIRE:       return b(FireAimEnabled());
    case AIM_BLINK:      return b(g_blkDriveUI);
    case AIM_INTERACT:   return b(InteractAimEnabled());
    case AIM_THROW:      return b(ThrowAimEnabled());
    case AIM_GADGET:     return b(GadgetAimEnabled());
    case AIM_CARRY:      return b(CarryThrowAimEnabled());
    case AIM_POWERS:     return b(PowerAimEnabled());
    case PICKUP:         return b(PickupEnabled());
    case DOORS:          return b(PickupDoorsEnabled());
    case CARRY:          return b(PickupCarryEnabled());
    case USABLES:        return b(PickupUsablesEnabled());
    case ANIM_MANTLE:    return b(dvr::anim::mantle_enabled());
    case ANIM_TAKEDOWN:  return b(dvr::anim::takedown_arms_hidden());
    case ANIM_CINE:      return b(dvr::anim::cinematic_enabled());
    case DPAD:           return dpad_index(c.modifier);
    case DPAD_FLIP:      return b(c.flip);
    case PAUSE_CHORD:    return b(c.pauseChord);
    case QUICK_SAVE:     return c.quickSave;
    case QUICK_LOAD:     return c.quickLoad;
    case PANEL_CHORD:    return b(c.panelChord);
    case CINE_PITCH:     return b(CinePitchEnabled());
    case CINE_ROLL:      return b(CineRollEnabled());
    case CINE_FOV:       return b(CineFovEnabled());
    case CINE_FOV_MATCH: return b(CineFovMatchEnabled());
    case CINE_BORDERS:   return b(CineBordersEnabled());
    case SPECIAL_HEAD:   return b(SpecialHeadEnabled());
    case HUD_PANELS:     return b(dvr::hudcap::enabled());
    }
    return 0;
}

static void apply_controller(int id, int v)
{
    const char* who = "MODS menu";
    auto c = dvr::controller::config();
    if (id == DPAD)        c.modifier = kDpadMode[v >= 0 && v < 4 ? v : 0];
    if (id == DPAD_FLIP) {   // as in F10: a thumbrest modifier moves to the other hand with the D-pad
        c.flip = v != 0;
        if (c.modifier == dvr::controller::RightRest || c.modifier == dvr::controller::LeftRest)
            c.modifier = c.flip ? dvr::controller::LeftRest : dvr::controller::RightRest;
    }
    if (id == PAUSE_CHORD) c.pauseChord = v != 0;
    if (id == QUICK_SAVE)  c.quickSave = v;
    if (id == QUICK_LOAD)  c.quickLoad = v;
    if (id == PANEL_CHORD) c.panelChord = v != 0;
    c = dvr::controller::normalize(c);
    dvr::controller::configure(c);
    char value[16]; _snprintf(value, sizeof(value), "%d", c.modifier); value[15] = 0;
    ConfigWriteKey("Controllers", "DpadModifier", value, who);
    ConfigWriteKey("Controllers", "DpadFlip", s01(c.flip), who);
    ConfigWriteKey("Controllers", "PauseChord", s01(c.pauseChord), who);
    ConfigWriteKey("Controllers", "QuickSave", dvr::controller::partner_key(c.quickSave), who);
    ConfigWriteKey("Controllers", "QuickLoad", dvr::controller::partner_key(c.quickLoad), who);
    ConfigWriteKey("Controllers", "PanelChord", s01(c.panelChord), who);
}

static void apply(int id, int v)
{
    const char* who = "MODS menu";
    const bool on = v != 0;
    char buf[16];
    switch (id) {
    case RES: {   // as F10's "Set resolution": with DLSS Super Resolution this is the output size
        const float pct = (float)(v < 50 ? 50 : v > 300 ? 300 : v);
        const float axis = sqrtf(pct * 0.01f);
        const uint32_t w = (uint32_t)(2750.0f * axis + 0.5f), h = (uint32_t)(2850.0f * axis + 0.5f);
        if (dvr::dlss::mode() != dvr::dlss::ModeOff && dvr::dlss::quality() != dvr::dlss::QDlaa) {
            dvr::dlss::set_output(w, h, who);
            _snprintf(buf, sizeof(buf), "%u", w); buf[15] = 0; ConfigWriteKey("Clarity", "DlssOutputWidth", buf, who);
            _snprintf(buf, sizeof(buf), "%u", h); buf[15] = 0; ConfigWriteKey("Clarity", "DlssOutputHeight", buf, who);
        } else {
            ResLiveQueue(w, h);
        }
        break; }
    case UPSCALE_ON:   dvr::dlss::set_mode(on ? dvr::dlss::ModeDlaa : dvr::dlss::ModeOff, who);
                       ConfigWriteKey("Clarity", "DLAA", s01(on), who); break;
    case UPSCALER:     dvr::dlss::set_backend(on ? 1 : 0, who); ConfigWriteKey("Clarity", "Upscaler", s01(on), who); break;
    case UPSCALE_MODE: { const int m = dvr::dlss::kQualityOrder[v >= 0 && v < dvr::dlss::QCount ? v : 0];
                       dvr::dlss::set_quality(m, who);
                       _snprintf(buf, sizeof(buf), "%d", m); buf[15] = 0; ConfigWriteKey("Clarity", "DlssQuality", buf, who); break; }
    case SHARPEN:      dvr::clarity::set_sharpen((float)(v < 0 ? 0 : v > 100 ? 100 : v) * 0.01f, who);
                       _snprintf(buf, sizeof(buf), "%.2f", dvr::clarity::sharpen()); buf[15] = 0;
                       ConfigWriteKey("Clarity", "Sharpen", buf, who); break;
    case FOV:        { ProjectionFovSet((float)(v < 60 ? 60 : v > 120 ? 120 : v));
                       _snprintf(buf, sizeof(buf), "%.2f", ProjectionFovGet()); buf[15] = 0;
                       ConfigWriteKey("Screen", "ProjectionFov", buf, who); break; }
    case MIRROR_OFF:   dvr::desktop_eye::set_mirror_off(on); ConfigWriteKey("VR", "DesktopMirrorOff", s01(on), who); break;
    case RAIN_HIDE:    RainHideSet(on); ConfigWriteKey("Rain", "Hide", s01(on), who); break;

    case CROUCH:       g_crouchOn = on; ConfigWriteKey("Tracking", "PhysicalCrouch", s01(on), who); break;
    case SNAP:         dvr::snap::set_enabled(on, who); ConfigWriteKey("Turning", "SnapTurn", s01(on), who); break;
    case SNAP_ANGLE:   dvr::snap::set_angle_deg((float)v, who);
                       _snprintf(buf, sizeof(buf), "%.0f", dvr::snap::angle_deg()); buf[15] = 0;
                       ConfigWriteKey("Turning", "SnapAngle", buf, who); break;
    case POSTRACK:     g_posTrack = on; if (!on) { g_leanRightUU = 0; g_leanUpUU = 0; g_leanFwdUU = 0; }
                       ConfigWriteKey("PosTrack", "Enabled", s01(on), who); break;
    case HEAD_MOVE:    HeadMovementSet(on); ConfigWriteKey("Camera", "HeadBasedMovement", s01(on), who); break;

    case DOT: case LASER: case RETICLE_HAND: {
        auto rc = dvr::aim::config();
        if (id == DOT) rc.dot = on;
        if (id == LASER) rc.laser = on;
        if (id == RETICLE_HAND) rc.hand = on ? 1 : 0;
        dvr::aim::configure(rc, who);
        ConfigWriteKey("Crosshair", "Dot", s01(rc.dot), who);
        ConfigWriteKey("Crosshair", "Laser", s01(rc.laser), who);
        ConfigWriteKey("Crosshair", "Hand", rc.hand ? "right" : "left", who);
        break; }
    case AIM_FIRE:     FireAimSet(on, who); ConfigWriteKey("Aim", "FireFromHand", s01(on), who); break;
    case AIM_BLINK:    g_blkDriveUI = on; g_blkAimOnCfg = on;
                       if (on) g_blkDstReqUI = g_blkDstOnUI ? 0 : 1;
                       ConfigWriteKey("Blink", "ControllerAim", s01(on), who);
                       Log("blink: controller aim %s (%s)", on ? "ON" : "off", who); break;
    case AIM_INTERACT: InteractAimSet(on, who); ConfigWriteKey("Aim", "InteractFromHand", s01(on), who); break;
    case AIM_THROW:    ThrowAimSet(on, who); ConfigWriteKey("Aim", "ThrowFromHand", s01(on), who); break;
    case AIM_GADGET:   GadgetAimSet(on, who); ConfigWriteKey("Aim", "GadgetFromHand", s01(on), who); break;
    case AIM_CARRY:    CarryThrowAimSet(on, who); ConfigWriteKey("Aim", "CarryThrowFromHand", s01(on), who); break;
    case AIM_POWERS:   PowerAimSet(on, who); ConfigWriteKey("Aim", "PowersFromHand", s01(on), who); break;

    case PICKUP:       PickupSet(on, who); ConfigWriteKey("Aim", "PhysicalPickup", s01(on), who); break;
    case DOORS:        PickupSetDoors(on, who); ConfigWriteKey("Aim", "PhysicalDoors", s01(on), who); break;
    case CARRY:        PickupSetCarry(on, who); ConfigWriteKey("Aim", "PhysicalCarry", s01(on), who); break;
    case USABLES:      PickupSetUsables(on, who); ConfigWriteKey("Aim", "PhysicalUsables", s01(on), who); break;
    case ANIM_MANTLE:  dvr::anim::set_mantle(on); ConfigWriteKey("Anim", "MantleHandBack", s01(on), who); break;
    case ANIM_TAKEDOWN: dvr::anim::set_takedown_arms_hidden(on); ConfigWriteKey("Anim", "HideTakedownArms", s01(on), who); break;
    case ANIM_CINE:    dvr::anim::set_cinematic(on); ConfigWriteKey("Anim", "CinematicHandBack", s01(on), who); break;

    case DPAD: case DPAD_FLIP: case PAUSE_CHORD: case QUICK_SAVE: case QUICK_LOAD: case PANEL_CHORD:
        apply_controller(id, v); break;

    case CINE_PITCH:     CinePitchSet(on); OvlSaveBool("Cine", "LockPitch", on); break;
    case CINE_ROLL:      CineRollSet(on); OvlSaveBool("Cine", "LockRoll", on); break;
    case CINE_FOV:       CineFovSet(on); OvlSaveBool("Cine", "LockFov", on); break;
    case CINE_FOV_MATCH: CineFovMatchSet(on); OvlSaveBool("Cine", "MatchGameplayFov", on); break;
    case CINE_BORDERS:   CineBordersSet(on); OvlSaveBool("Cine", "HideBorders", on); break;
    case SPECIAL_HEAD:   SpecialHeadSet(on); OvlSaveBool("Cine", "SpecialHeadLook", on); break;

    case HUD_PANELS:     dvr::hudcap::set_enabled(on); ConfigWriteKey("Hud", "Panel", s01(dvr::hudcap::enabled()), who); break;
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
                            COUNT, g_items, &menu_get, &menu_set, G_COUNT, kGroups };

static const Menu* query()
{
    static volatile LONG ready = 0;
    if (!InterlockedExchange(&ready, 1)) {
        for (int i = 0; i < dvr::dlss::QCount; ++i) {   // the upscaler mode names, as F10 shows them
            const char* n = dvr::dlss::quality_name(dvr::dlss::kQualityOrder[i]);
            if (!n || !MultiByteToWideChar(CP_UTF8, 0, n, -1, g_modeNames[i], 48)) wcscpy_s(g_modeNames[i], L"?");
            g_modeList[i] = g_modeNames[i];
        }
        Log("modmenu: the Mod Loader plugin asked for the menu (%d entries in %d groups)", (int)COUNT, (int)G_COUNT);
    }
    return &kMenu;
}

} // namespace dvr::modmenu

static void DvrModMenuTick() { dvr::modmenu::tick(); }

extern "C" const void* __cdecl DvrModMenuQuery() { return dvr::modmenu::query(); }
