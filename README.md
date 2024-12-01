  <p align=center>
  <img src="https://github.com/user-attachments/assets/c4d4ae3b-f9e0-49d5-90c6-ad115a2551b3">
</p>

explorer7 is a **wrapper library** that allows Windows 7's explorer.exe to run properly on modern Windows versions, aiming to bring back the original Windows 7 shell experience.

<details>
  <summary>Screenshots</summary>

![image](https://github.com/user-attachments/assets/f0d774c5-6f81-4985-ac11-5c867dd34094)
![image](https://github.com/user-attachments/assets/8dd8b78f-fdc1-47ca-9dc6-cfa60e95b6c1)

![image](https://github.com/user-attachments/assets/2cff2aa4-85c4-4ca3-818c-e6d5c8096b1f)
![image](https://github.com/user-attachments/assets/9f3fdb95-cb9f-4999-9cb5-92dedf4af505)

</details>

## Known issues (Milestone 2, last updated 2024-11-29)

**MAKE SURE YOU READ THESE FIRST SO YOU ARE AWARE OF WHAT YOU ARE GETTING INTO!**

**Windows 8.1**
- No proper strings for the "Customize Start Menu" dialog (fixed system-wide in Windows 10).
- The start menu expanding animation is broken (persists in later versions).

**Windows 10**
- Autoplay does not work (persists in later versions).
- System msstyles with the name "aero.msstyles" will screw up the start menu and taskbar colorization (only when ColorizationOptions is set to 0, persists in later versions).
- "Notification Area Icon" settings in Control Panel are missing (persists in later versions).
- The start menu cannot be correctly invoked from its relevant hotkey whilst immersive shell is enabled (persists in later versions).
- The taskbar might overlap fullscreen applications whilst immersive shell is enabled (persists in later versions).
- If a user has StartIsBack++ installed, it may attempt to erroneously hook into the shell, causing both visual and functional issues.
- "Settings" is duplicated in the start menu program list (1607 onwards, ceases to be an issue in Windows 11).

**Windows 11**
- Taskbar and start menu pin creation is broken due to an internal shell32.dll code logic change (currently fixed in Windows 10 22H2, but not in 11 23H2+).
- XAML-based UI dialogs invoked by the keyboard can cause the shell to crash (each must be disabled respectively, likely to be fixed in M3).
- Attempting to invoke context menus for executable files will cause the explorer.exe shell to hang and require a restart.
- BlurBehind colorization mode no longer works due to the removal of the relevant accent policy (starting from 22H2+).

**Windows 7 limitations/bugs**

All of the following are bugs/limitations within Windows 7's Explorer itself, and won't be accounted for in explorer7:

- Multi-monitor taskbars are not supported. These would be introduced by Windows 8 build 7779.
- Startup items defined in the modern task manager are not accounted for. You must use the old msconfig.exe.
- It takes a few minutes for changes to the size and position of the taskbar to be written to the registry; restarting Explorer before that will revert those changes.
- While the small taskbar is enabled, changing the position to top or bottom from the properties window (NOT from dragging) will allocate extra appbar space, and there will be a gap between the taskbar and the working area.

## Installation Guide

For casual users, the **regular installation method** is listed below:

<details>
  <summary>Standard installation</summary>
  
**Pre-Requirements**
1. explorer7 package from releases
2. Valid Windows 7 x64 installation medium, in the same language as your system

**How-to**
1. Mount Windows 7 install media by double clicking on it
2. Have the explorer7 package extracted somewhere handy. A good folder would be `X:\Program Files\explorer7`
3. Run ex7forw8.exe. The installer will ask for Windows 7 files. You can select any of the 2 options provided the installation media is mounted.
4. You should see the following dialog if the installer succeeded:
   
   ![image](https://github.com/user-attachments/assets/4772d2b2-e355-4d85-82a5-efe2a1a18539) 
   
5. If you wish to switch your shell to the Windows 7 explorer right now, use the option for it. You can always change back by running ex7forw8.exe once again and selecting the "Use Windows 8 explorer" option (this is currently misnamed, all it does is revert to your system's default shell)
6. Enjoy!
</details>
 

In case you have an unsupported explorer.exe version you wish to try your luck on, or your installation medium is in another language, you may try **manually patching and installing** providing your own files:

<details>
  <summary>Manual Installation/Patching</summary>

**Pre-Requirements:**
1. explorer7 package from releases
2. [CFF Explorer](https://ntcore.com/files/CFF_Explorer.zip)
3. Valid installation medium of your choice (Windows XP x64 - Windows 7 SP1 x64)
4. [7-Zip](https://www.7-zip.org/) or [WinRAR](https://www.win-rar.com/start.html) unless you want to mount install.wim using DISM to extract a few files like a maniac
5. Slight experience utilizing a personal computer

**Step 1 - Fetching files**

**NOTE:** Windows XP did not have MUI files You only need the `explorer.exe` from it. You can also skip the en-US folder creation part.

1. Mount your install media
2. Open `\sources\install.wim` using your archiver of choice (listed 2 in the pre-requirements)
3. Fetch the following files from install.wim (copy them somewhere safe): `\1\Windows\explorer.exe`, `\1\Windows\en-US\explorer.exe.mui` and `\1\Windows\System32\en-US\shell32.dll.mui`
4. Make an "en-US" folder in the folder which contains the explorer7 package. The file tree will look something like the following:
```
ex7_example/
├─ theme/
├─ en-US/
├─ ex7forw8.exe
├─ Import Me.reg
├─ README.txt
├─ wrp64.dll

```
5. Copy `shell32.dll.mui` and `explorer.exe.mui` to the `en-US` folder you've just created, and `explorer.exe` alongside `wrp64.dll`:
```
ex7_example/
├─ theme/
├─ en-US/
│  ├─ explorer.exe.mui
│  ├─ shell32.dll.mui
├─ ex7forw8.exe
├─ explorer.exe
├─ Import Me.reg
├─ README.txt
├─ wrp64.dll

```

Now you should have all of the necessary files to go onto the next step.

**Step 2 - Patching explorer.exe**

**NOTE:** For now, do not replace the `SHLWAPI.DLL` import on XP x64's `explorer.exe`.

By default, explorer.exe will not use the wrapper dll, so you have to change out a few imports in the executable. Make sure you've fetched [CFF Explorer](https://ntcore.com/files/CFF_Explorer.zip) from the requirements.
1. Open CFF Explorer, drag explorer.exe into the window
2. Open the "Import Directory" folder in the left sidebar
3. Change out the imports for `SHLWAPI.DLL`, `OLE32.DLL` and (if applicable) `EXPLORERFRAME.DLL`:
![image](https://github.com/user-attachments/assets/6e8acb56-4400-43be-a0c6-0f43ec20480f)
4. Save the file.

By now, you should be able to start `explorer.exe` from task manager or through other means. 

</details>


## Registry keys

These keys are located under `HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Advanced`.

| Name | Type | Description | Default Value |
| ---- | ---- | ----------- | ------------- |
| Theme | REG_SZ | Name of the theme file to use. This is relative to the installation directory. For example, `"aero"` will use the theme at `"explorer7\theme\aero.msstyles"`, `"Aero\aero"` will use the theme at `"explorer7\theme\Aero\aero.msstyles"`. If this is not specified, `aero` will be used. | **aero** |
| OrbDirectory | REG_SZ | Name of the orb images directory to use. This is relative to the installation directory. For example, `"6801"` will use the orb images located at `"explorer7\orbs\6801\"`, `"Orb1\6801"` will use the orbs located at `"explorer7\orbs\Orb1\6801\"`. If this is not specified, the internal explorer image will be used.| **default** |
| DisableComposition | REG_DWORD | When set to 1, Explorer7 will act as if the Desktop Window Manager is not running. | **0** |
| ClassicTheme | REG_DWORD | When set to 1, Explorer7 will use the Windows Classic theme. | **0** |
| EnableImmersive | REG_DWORD | Controls the ability to run UWP apps in the system. When set to 0, UWP apps won't run. | **0** |
| EnableUWPAppsInStart | REG_DWORD | When set to 0, UWP apps will be hidden from the All Programs list. | **1** |
| ColorizationOptions | REG_DWORD | Controls shell colorization behaviour. Options 1 to 4 may have varying compatibility across Windows versions. | **1** |
| RPEnabled | REG_DWORD | When set to 1, relevant theme classes suffixed with the number "8" will be used, allowing Windows 8-based themes to render correctly. | **0** |

## Theme support

explorer7 allows any theme from Windows Vista - Windows 8.0 to be used for the start menu/taskbar. If applicable, you **must** include the "en-US" folder that comes along with your .msstyles file, otherwise the theme won't be applied. Windows 8.1+ themes do work, but do not have the proper classes for the Start Menu, and as of today, they cannot be restored.

<details>
  <summary>Here are valid file structures for the theme folder:</summary>

`Theme` registry key set to `theme1`
```
explorer7/
├─ theme/
│  ├─ en-US/
│  ├─ theme1.msstyles
```

`Theme` registry key set to `Themefolder\theme1`
```
explorer7/
├─ theme/
│  ├─ Themefolder/
│  │  ├─ en-US/
│  │  ├─ theme1.msstyles

```
  
</details>

## Custom orbs

As an additional feature, explorer7 lets you import your own custom orbs without having to patch your explorer.exe using Resource Hacker or using specialized programs. Due to WinGDI limitations, it only supports .bmp images. To do this, simply make a directory inside the "orbs" folder and place your images inside it with the naming scheme from the example layout below. If it finds the appropiate images, the orb system will also account for 125% and 150% DPI (HiDPI) automatically. The layout should be as it follows:

<details>
  <summary>Valid layout for custom orbs:</summary>

`OrbDirectory` registry key set to `blue`
```
explorer7/
├─ orbs/
│  ├─ blue/
│  │  ├─ 6801.bmp (100% DPI - 52x162 - Bottom-aligned taskbar image)
│  │  │  6802.bmp (125% DPI - 66x198 - Bottom-aligned taskbar image)
│  │  │  6803.bmp (150% DPI - 81x243 - Bottom-aligned taskbar image)
│  │  │  6804.bmp (190% DPI - 106x318 - Bottom-aligned taskbar image)
│  │  │  6805.bmp (100% DPI - 52x162 - Left/right-aligned taskbar image)
│  │  │  6806.bmp (125% DPI - 66x198 - Left/right-aligned taskbar image)
│  │  │  6807.bmp (150% DPI - 81x243 - Left/right-aligned taskbar image)
│  │  │  6808.bmp (190% DPI - 106x318 - Left/right-aligned taskbar image)
│  │  │  6809.bmp (100% DPI - 52x162 - Top-aligned taskbar image)
│  │  │  6810.bmp (125% DPI - 66x198 - Top-aligned taskbar image)
│  │  │  6811.bmp (150% DPI - 81x243 - Top-aligned taskbar image)
│  │  │  6812.bmp (190% DPI - 106x318 - Top-aligned taskbar image)

```

`OrbDirectory` registry key set to `colors\green`
```
explorer7/
├─ orbs/
│  ├─ colors/
│  │  ├─ green/
│  │  │  ├─ 6801.bmp (100% DPI - 52x162 - Bottom-aligned taskbar image)
│  │  │  │  6802.bmp (125% DPI - 66x198 - Bottom-aligned taskbar image)
│  │  │  │  6803.bmp (150% DPI - 81x243 - Bottom-aligned taskbar image)
│  │  │  │  6804.bmp (190% DPI - 106x318 - Bottom-aligned taskbar image)
│  │  │  │  6805.bmp (100% DPI - 52x162 - Left/right-aligned taskbar image)
│  │  │  │  6806.bmp (125% DPI - 66x198 - Left/right-aligned taskbar image)
│  │  │  │  6807.bmp (150% DPI - 81x243 - Left/right-aligned taskbar image)
│  │  │  │  6808.bmp (190% DPI - 106x318 - Left/right-aligned taskbar image)
│  │  │  │  6809.bmp (100% DPI - 52x162 - Top-aligned taskbar image)
│  │  │  │  6810.bmp (125% DPI - 66x198 - Top-aligned taskbar image)
│  │  │  │  6811.bmp (150% DPI - 81x243 - Top-aligned taskbar image)
│  │  │  │  6812.bmp (190% DPI - 106x318 - Top-aligned taskbar image)

```
  
</details>

**NOTE 1:** BE CAREFUL! If the image corresponding to your case DOES NOT exist in your orb directory, it will automatically fall back to the original image inside explorer.exe.

**NOTE 2:** If an image is larger than what the system expects, the image might clip out. Use the example layout as a reference! For more information, you can also check out this guide: https://www.sevenforums.com/tutorials/73616-how-create-custom-start-orb-image.html

**NOTE 3:** If you're looking to create high-quality orbs (32-bit bitmaps), you could use a tool to convert your images from other formats. Check out [Pixelformer](https://www.qualibyte.com/pixelformer/).

## Development plan

We're working based on a milestone stage. Here's the planned stages of development:

|   Stage   | Goal | Status |
| -------- | --------- | ------ |
| Milestone 1 | First release, focusing on stability for Windows 8.1 and a starting point for Windows 10 support. | ✅ Completed |
| Milestone 2 | Ironing out any last Windows 8.1-specific bugs, stability on Windows 10, likely UWP support, some QOL work (accurate program list, older .msstyles support, custom orb support) and a starting point for Windows 11 support. | ⏳ Work in progress |
| Milestone 3 | Solving any persistent bugs left for Windows 10, whilst finishing up any remaining bug fixes for Windows 11. | ⛔ Not in active development |

While this project is aimed at restoring Windows 7 explorer.exe functionality, some older explorer versions have been found to work with the wrapper. In the future, we plan to support some of these directly.  Here's the chart
for support:

| Version | Status |
| ------- | ------ |
| Windows 7 | ⏳ Work in progress |
| Windows Vista | ❌ Not in active development |
| Windows XP x64 | ⏳ Early work in progress |

## Minhook Linker errors

If you're having linker errors because of the prebuilt minhook, do the following:

- In the root folder, open a cmd and grab the minhook repo: `git clone https://github.com/TsudaKageyu/minhook.git minhook`
- Once it's done, compile the solution in `minhook\build\VC17\MinHookVC17.sln`, specifying x64 Platform. You can do this using Visual Studio.
- Either copy it to the explorerwrapper project folder or just don't do anything and compile. A pre-build task will copy the new version over for you to use.

Contributors: DON'T COMMIT YOUR MODIFIED `libMinHook.x64.lib` UNLESS SPECIFIED!

