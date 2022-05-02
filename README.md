# RE5Fix
This is DLL hook that fixes various issues with Resident Evil 5 relating to ultrawide support and more.<br />

## Features
- FOV adjustment (WIP).
- Uncap 120 FPS limit.
- Remove yellow colour filter.
- Adjust shadow quality.
- Remove 4096x4096 resolution limit.

### Ultrawide Fixes
- Fixed UI scaling using ultrawide.
- Fixed ERROR 09: Unsupported Function crash using ultrawide.
- Fixed stretched movie playback.

## Installation
- Downloaded the [latest release](https://github.com/Lyall/RE5Fix/releases) of RE5Fix.
- Unzip in to the game directory (e.g **steamapps/common/Resident Evil 5**).
- Edit **RE5Fix.ini** to enable/adjust features.

## Troubleshooting
This fix was primarily created for use with [QOL Fixes](https://steamcommunity.com/sharedfiles/filedetails/?id=1533171339) but it **may** work with other versions of RE5.

1. **I get "xlive.dll is missing" error.** <br />
The included version of GFWL on the steam version of RE5 does not install properly. Either find an "xlive.dll" from the internet/other game and place it in the game directory or use one of the unofficial patches. <br />
2. **The file "xinput_1_3.dll" conflicts with another mod.** <br />
You can rename the DLL to "dinput8.dll" or if you are using a mod with a DLL loader then you can just put it in the mod's plugins folder.

## Known Issues
- FOV adjustment is still a work-in-progress.

## Screenshots

| ![20220430010145_1](https://user-images.githubusercontent.com/695941/166082065-67568c51-8e1e-4cd1-af49-5e956860a47a.jpg) |
|:--:|
| 21:9 with colour filter removal and adjusted FOV. |

| ![re5dx9_2022_04_28_08_07_39_715](https://user-images.githubusercontent.com/695941/165991472-15f70372-551e-45b7-a48c-2323eb52e605.jpg) |
|:--:|
| 32:9 with working cutscenes! |


## Credits
[RERevHook](https://www.nexusmods.com/residentevilrevelations/mods/26) for the DLL proxy code.<br />
[inih](https://github.com/jtilly/inih) for ini reading.<br />
[SignatureScanner](https://github.com/Imrglop/SignatureScanner) for signature scanning.<br />
[FlawlessWidescreen](http://www.flawlesswidescreen.org/)'s RE5 script for info on where to start with UI/Res Limit fix.


