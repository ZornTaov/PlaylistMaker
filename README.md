# PlaylistMaker
This project was made for giving Switch users an alternative way to generate playlist files.

## Building for Switch

To build for Switch, a standard development environment must first be set up. In order to do so, [refer to the Getting Started guide](https://devkitpro.org/wiki/Getting_Started).

```bash
# To get the source and all dependencies, do:
git clone --recursive [this repo]
# Doing so will get all of the required submodules.
# For first time setup, install the other needed libraries from pacman
(sudo) (dkp-)pacman -S switch-glfw switch-mesa switch-glm switch-glad
# First build the static library
cd switch-nanogui
make -j$(nproc)
# Now build the project
cd ..
make -j$(nproc)
```

## Settings
- `retroarchPATH`: Path to retroarch.
- `romsPaths`: Possible Roms paths, change if your games are in another folder.
- `playlistsPath`: Path to playlist directory.
- `coresPath`: Path to Core directory.
- `indexRomPathUsed`: Default path index used.
- `useShorthandName`: If the folder validator should use shorthand or full system names for the folders it makes/checks for.
- `useAllExtentions`: If the generator should use all possible extentions for all possible cores for that system.
- `makeJsonPlaylists`: If the generator should create Json playlists.  Note that existing playlists will keep their format.
- `printDebugLogs`: set to true if you want more debug logs to be printed to the log file.  Exceptions will always be written there if they occur.

## Caution
- PyListMaker will modify the folder structure of your Roms folder only by adding folders that don't exist when Validate Folders is ran.

- You might not have your roms for each system in the correct folder for that system (all your games are just in /roms/ for instance) in which case for the generator to work you will need to move all the games into their respective system folders that Validate Folders creates in order to be scanned (.smc in snes, .gba in gba, and so on).

- Playlists will either be created if there are roms available for that system and there's no playlist, or modified with new entries if the playlist exists.

- Duplicates may happen!  When generating or modifying playlists this script will also check for zip files, so if you have game.rom and game.zip then both entries will exist in the playlist.

## ToDo
- Allow users to select individual folders to scan.
- Allow users to remove individual entries in existing playlists.
