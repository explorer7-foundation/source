#pragma once
#include "common.h"
#include "dbgprint.h"
#include "OSVersion.h"
#include "RegistryManager.h"

// Individual option definitions
// - These are external to ensure we can call them elsewhere
extern bool s_ClassicTheme;
extern bool s_DisableComposition;
extern int s_EnableImmersiveShellStack;
extern bool s_UseTaskbarPinning;
extern bool s_ShowStoreAppsOnTaskbar;
extern bool s_ShowStoreAppsInStart;
extern int s_AcrylicAlt;
extern int s_ColorizationOptions;
extern bool s_UseWin8DefaultAlpha;
extern bool s_OverrideAlpha;
extern DWORD s_AlphaValue;
extern bool s_UseDCompFlyouts;

// Responsible for settings these values and calling them from registry
extern void InitializeConfiguration();
