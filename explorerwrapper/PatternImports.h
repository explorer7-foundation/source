#pragma once
#include "common.h"
#include "dbgprint.h"
#include "OptionConfig.h"
#include "OSVersion.h"

// Ittr: Pattern imports are defined and rewritten here rather than dllmain. Feel free to further improve this.

// Allow 7 msstyles to load by removing animation map data from uxtheme.dll imports
void RemoveLoadAnimationDataMap();
void RemoveGetClassIdForShellTarget(); // for windows 8.1

// Fix authui.dll import for CLogOffOptions by replacing bytes
void FixAuthUI();

// Remove unwanted immersive shell interfaces, often by preventing them from running
void DisableImmersiveStart();
void DisableImmersiveSearch();
void DisableTaskView();
void RestoreWin32Menus();
void DisableWin11AltTab();
void DisableWin11HardwareConfirmators();
void FixWin11SearchIcon();

// Enable taskbar pins on 24H2 and later
void EnablePinning();

// Fix executable context menus on all Windows 11 versions
void FixWin11ContextMenu();

// Disable DComp immersive flyouts as applicable
void RevertFlyouts();

// Main procedure we call from elsewhere
void ChangePatternImports();
