  <p align=center>
  <img src="https://github.com/user-attachments/assets/c4d4ae3b-f9e0-49d5-90c6-ad115a2551b3">
</p>

explorer7 is a **wrapper library** that allows Windows 7's explorer.exe to run properly on modern Windows versions, aiming to resurrect the original Windows 7 shell experience.

<details>
  <summary>Screenshots</summary>

![image](https://github.com/user-attachments/assets/f0d774c5-6f81-4985-ac11-5c867dd34094)
![image](https://github.com/user-attachments/assets/8dd8b78f-fdc1-47ca-9dc6-cfa60e95b6c1)

![image](https://github.com/user-attachments/assets/2cff2aa4-85c4-4ca3-818c-e6d5c8096b1f)
![image](https://github.com/user-attachments/assets/9f3fdb95-cb9f-4999-9cb5-92dedf4af505)

</details>

## Known issues (Milestone 2, last updated 2025-02-07)
These issues, unless specified to have been resolved in a later Windows version, are persistent across subsequent versions of Windows from their introduction.

**MAKE SURE YOU READ THESE FIRST SO YOU ARE AWARE OF WHAT YOU ARE GETTING INTO!**

**Windows 8.1**
- No proper strings for the "Customize Start Menu" dialog (fixed system-wide in Windows 10).
- The start menu expanding animation is broken (persists in later versions).

**Windows 10**
- Autoplay does not work (1507+).
- When ColorizationOptions is set to 0, system msstyles with the name "aero.msstyles" will screw up the start menu and taskbar colorization (1809+).
- "Notification Area Icon" settings in Control Panel are missing (1507+).
- The start menu cannot be correctly invoked from its relevant hotkey whilst immersive shell is enabled (1507+).
- The taskbar might overlap fullscreen applications whilst immersive shell is enabled (1507+).
- If a user has StartIsBack++ installed, it may attempt to erroneously hook the shell, causing both visual and functional issues.
- "Settings" is duplicated in the start menu program list (1607-22H2, fixed system-wide in Windows 11).
- Minimizing applications within the taskbar may cause the program to remain active when the immersive shell is enabled (1507+).

**Windows 11**
- BlurBehind colorization mode no longer works due to the removal of the relevant accent policy (22H2+).
- Taskbar and start menu pin creation is broken due to an internal shell32.dll code logic change (24H2+).
- Immersive shell support does not function correctly, and cannot launch applications (Insider 25H2+).

**Windows 7 limitations/bugs**

All of the following are bugs or limitations within Windows 7's explorer itself, and will not be accounted for:

- Multi-monitor taskbars are not supported. These were later introduced in Windows 8 build 7779.
- Startup items defined in the modern Task Manager are not correctly accounted for - you must use the old msconfig.exe.
- It takes a few minutes for changes to the size and position of the taskbar to be written to the registry; restarting Explorer too quickly will revert these changes.
- Whilst small taskbar icons are enabled, changing the position to `Top` or `Bottom` from the properties window (NOT from dragging) will result in extra space being allocated between the taskbar and the working area.

## Installation Guide

For casual users, the **regular installation method** is listed below:

<details>
  <summary>Standard installation</summary>
  
**Pre-Requirements**
1. The explorer7 package from releases.
2. A valid Windows 7 x64 installation medium, in the same language as your system.

**How-to**
1. Mount your Windows 7 install media by double-clicking it.
2. Extract the explorer7 package to a suitable location. For example: `X:\Program Files\explorer7`, or `X:\explorer7`.
3. Run Ex7forW8.exe. The installer will ask for Windows 7 files. You can select either option for installation, provided that the installation media is mounted.
4. You should see the following dialog if the installer succeeded:
   
   ![image](https://github.com/user-attachments/assets/4772d2b2-e355-4d85-82a5-efe2a1a18539) 
   
5. After that, when you wish to switch your shell to the Windows 7 explorer, select the applicable option. You can always change back by running Ex7forW8.exe once again and selecting the "Use Windows 8 explorer" option (this is currently a misnomer, it actually reverts to your system's default shell, which in most cases is the modern explorer executable).
6. Enjoy!
</details>
 

If you have an unsupported explorer.exe file from another Windows release that you want to use, or your installation medium is in another language, you can try **manually patching and installing** with your own files:

<details>
  <summary>Manual Installation/Patching</summary>

**Pre-Requirements:**
1. The explorer7 package from releases.
2. [CFF Explorer](https://ntcore.com/files/CFF_Explorer.zip)
3. Valid installation medium of your choice (Windows XP x64 - Windows 7 SP1 x64).
4. [7-Zip](https://www.7-zip.org/) or [WinRAR](https://www.win-rar.com/start.html) unless you want to mount install.wim using DISM to extract a few files.
5. Experience with utilizing a personal computer and advanced file modification.

**Step 1 - Fetching files**

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

By default, explorer.exe will not use the wrapper dll, so you have to change out a few imports in the executable. Make sure you've fetched [CFF Explorer](https://ntcore.com/files/CFF_Explorer.zip) from the requirements.
1. Open CFF Explorer, drag explorer.exe into the window
2. Open the "Import Directory" folder in the left sidebar
3. Change out the imports for `SHLWAPI.DLL`, `OLE32.DLL` and (if applicable) `EXPLORERFRAME.DLL`:
![image](https://github.com/user-attachments/assets/6e8acb56-4400-43be-a0c6-0f43ec20480f)
4. Save the file.

By now, you should be able to start `explorer.exe` from task manager or through other means. 

</details>


## Registry options

These options are located under `HKEY_CURRENT_USER\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\Advanced`.

| Name | Type | Description | Default Value |
| ---- | ---- | ----------- | ------------- |
| Theme | REG_SZ | Name of the theme file to use. This is relative to the installation directory. For example, `"aero"` will use the theme at `"explorer7\theme\aero.msstyles"`, `"Aero\aero"` will use the theme at `"explorer7\theme\Aero\aero.msstyles"`. If this is not specified, `aero` will be used. | **aero** |
| OrbDirectory | REG_SZ | Name of the orb images directory to use. This is relative to the installation directory. For example, `"6801"` will use the orb images located at `"explorer7\orbs\6801\"`, `"Orb1\6801"` will use the orbs located at `"explorer7\orbs\Orb1\6801\"`. If this is not specified, the internal explorer image will be used.| **default** |
| DisableComposition | REG_DWORD | When set to 1, explorer7 will act as if the Desktop Window Manager is not running. | **0** |
| ClassicTheme | REG_DWORD | When set to 1, explorer7 will use the Windows Classic theme. | **0** |
| EnableImmersive | REG_DWORD | Controls the ability to run UWP applications in the system. When set to 0, UWP applications will not be able to run. | **0** |
| StoreAppsInStart | REG_DWORD | When set to 0, UWP applications will be hidden from the All Programs list. | **1** |
| StoreAppsOnTaskbar | REG_DWORD | When set to 0, specializations applied to load UWP application icons will not be applied. | **0 (when EnableImmersive = 0)**, **1 (when EnableImmersive = 1)** |
| ColorizationOptions | REG_DWORD | Controls shell colorization behaviour. Options 1 to 4 may have varying compatibility across Windows versions. | **1** |
| AcrylicColorization | REG_DWORD | Controls acrylic colorization behaviour. Options 0-2 control the use of immersive colours, option 3 will use the regular colorization. | **0** |
| OverrideAlpha | REG_DWORD | When set to 1, colorization alpha specified by DWM is overridden on the taskbar, start menu, and thumbnails. | **0** |
| AlphaValue | REG_DWORD | For use alongside OverrideAlpha, to specify a 2-digit hex code for the colorization system to use. | **0x6B** |
| UseTaskbarPinning | REG_DWORD | Determines whether taskbar pinning functionality is available to the user. When set to 0, pins will not be loaded and cannot be modified from jumplists. | **1** |
| Win8DefaultAlpha | REG_DWORD | When set to 1, the colorization system attempts to approximate default Windows 8-era alpha values, rather than using the alpha value defined by default for Windows 10 and 11. <br> <br> This value is deprecated and will be removed in a future release. | **1** |

## Theme support

explorer7 allows any theme from Windows Vista to Windows 8.0 to be used for the start menu and taskbar. If applicable, you **must** include the "en-US" folder that comes along with your .msstyles file, otherwise the theme won't be applied. Themes from Windows 8.1 and later do work, but will not have the proper classes for the start menu, an issue which cannot currently be resolved.

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

We're working based on a series of development milestones. Here's the planned development stages as things currently stand:

|   Stage   | Goal | Status |
| -------- | --------- | ------ |
| Milestone 1 | Initial release focused on stability for Windows 8.1, and providing a starting point for Windows 10 support. | ✅ Completed |
| Milestone 2 | - Achieving stability for Windows 10 and 11 (up to and including 23H2) <br> - Ensuring that behaviour on Windows 8.1 perfectly matches its predecessor. <br> - Providing more visually accurate interfaces (e.g. program list) <br> - Supporting older .msstyles <br> - Introducing immersive shell support <br> - Custom orb support | ⏳ Work in progress |
| Milestone 3 | Solving persistent bugs remaining on Windows 10 and 11. Likely to focus more on fixes and adjustments than new features. | ⛔ Not in active development |

While this project is aimed at restoring Windows 7 explorer.exe functionality, some older explorer versions have been found to work with the wrapper. In the future, we plan to support some of these directly.  Here's the chart
for support:

| Version | Status |
| ------- | ------ |
| Windows 7 | ⏳ Work in progress |
| Windows Vista | ❌ Not in active development |

## Minhook Linker errors

If you're having linker errors because of the prebuilt minhook, do the following:

- In the root folder, open a cmd and grab the minhook repo: `git clone https://github.com/TsudaKageyu/minhook.git minhook`
- Once it's done, compile the solution in `minhook\build\VC17\MinHookVC17.sln`, specifying x64 Platform. You can do this using Visual Studio.
- Either copy it to the explorerwrapper project folder or just don't do anything and compile. A pre-build task will copy the new version over for you to use.

Contributors: DON'T COMMIT YOUR MODIFIED `libMinHook.x64.lib` UNLESS SPECIFIED!

