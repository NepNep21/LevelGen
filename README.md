This project uses LGPLv3 components of Qt, its source code can be found at https://code.qt.io, the LGPL can be found at Qt-licenses/LGPLv3.txt and the GPL at Qt-licenses/GPLv3.txt

A mostly useless ark config generator for "OverridePlayerLevelEngramPoints" and "LevelExperienceRampOverrides", this was my first C++ project so its code is likely bad.

Why is it "mostly useless"? Because the way it works is very simple, it just multiplies a starting number by a multiplier x times, which usually generates terribly unbalanced configs, i may fix that when i'm better at this, in the meantime PRs are welcome.

# Usage 

## Windows 

Download the windows archive from the releases page, extract it, and run LevelGen.exe.

## Linux 

1. Install Qt5 if you don't have it (more details below)
2. Download and run the linux binary from the releases page.

## Both 

Enter the values you want, and click "Run", the config will be copied to your clipboard, after that paste it in your Game.ini under "[/Script/ShooterGame.ShooterGameMode]".

## MacOS 

As i do not have a mac for testing, MacOS is not supported, however you could compile it yourself.

# Installing Qt on linux 

## Ubuntu 

`sudo apt-get install qtbase5-dev`

## Arch 

`sudo pacman -Syu qt5-base`

# Compilation 

Requirements: Qt5, cmake, make, and a C++ compiler (tested with GCC 11.1.0 on linux and 8.1.0 (MinGW-W64) on windows).

Download the code and run:

## Linux 

```
cd LevelGen
mkdir build
cd build
cmake ..
make
```

## Windows

```
cd LevelGen
mkdir build
cd build
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH="C:\Path\To\Qt" ..
mingw32-make
```

The executable will be in the build directory.

To redistribute the Qt dlls for windows, run `windeployqt --no-translations --release LevelGen.exe`
